#include "CommandDispatcher.hpp"
#include <format>
#include <thread>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/awaitable.hpp>
#include "../core/ServicePublisher/Commands/GetServicesCommand.hpp"
#include "GetServiceHandler.hpp"

namespace XFS4IoTServer
{
    CommandDispatcher::CommandDispatcher(
        const std::vector<XFS4IoT::XFSConstants::ServiceClass>& services,
        std::shared_ptr<ILogger> logger,
        const std::string& assemblyName)
        : logger_(logger), serviceClasses_(services)
        , queueWorkGuard_(boost::asio::make_work_guard(queueIoContext_))

    {
        if (!logger_) {
            throw std::invalid_argument("Логгер не может быть null");
        }
        if (services.empty()) {
            throw std::invalid_argument("Список сервисов не может быть пустым");
        }

        commandQueue_ = std::make_unique<CommandQueue>(logger_);

        queueThread_ = std::jthread([this](std::stop_token) {
            try {
                queueIoContext_.run();
            }
            catch (const std::exception& ex) {
                logger_->error(std::format("Command queue io_context failed: {}", ex.what()));
            }
            });

        // In C++, handlers are registered manually via RegisterHandler<MessageType, HandlerType>()
        // instead of reflection-based discovery used in C#
        //RegisterHandler<
        //    XFS4IoT::ServicePublisher::Commands::GetServicesCommand,
        //    XFS4IoTServer::GetServiceHandler
        //>(XFS4IoT::XFSConstants::ServiceClass::Publisher);

        std::string handlers;
        for (const auto& [type, details] : messageHandlers_) {
            handlers += std::format("{} => Async:{}\n",
                type.name(),
                details.isAsync);
        }

        logger_->trace(std::format("{}() -Dispatch: найдены классы обработчиков команд:\n{}", __FUNCTION__, handlers));
    }

    boost::asio::awaitable<void> CommandDispatcher::Dispatch(
        std::shared_ptr<IConnection> connection,
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::stop_token token)
    {
        std::exception_ptr timeoutEx = nullptr;
        bool hasTimeout = false;
		std::shared_ptr<ICommandHandler> handler;
        try {
            if (!connection) {
                throw std::invalid_argument("Недопустимый параметр в методе Dispatch: Connection");
            }
            if (!command) {
                throw std::invalid_argument("Недопустимый параметр в методе Dispatch: Command");
            }

            if (logger_) {
                logger_->trace(std::format(
                    "CommandDispatcher::Dispatch this = {} команда = {}",
                    static_cast<const void*>(this),
                    command->Header().Name()));
            }

            // Send acknowledgment
            auto ack = std::make_shared<XFS4IoT::Acknowledge>(
                command->Header().RequestId().value(),
                command->Header().Name(),
                command->Header().Version(),
                std::nullopt);
            co_await connection->SendMessageAsync(ack);

            // Create linked cancellation token source
            std::stop_source stopSource;
            std::stop_token stopToken = stopSource.get_token();

            // Link parent token - monitor if parent requests cancellation
            //std::jthread tokenWatcher;
            //if (token.stop_requested()) {
            //    stopSource.request_stop();
            //}
            //else {
            //    tokenWatcher = std::jthread([token, stopSourcePtr = &stopSource](std::stop_token watcherToken) {
            //        while (!watcherToken.stop_requested() &&
            //            !token.stop_requested() &&
            //            !stopSourcePtr->stop_requested()) {
            //            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            //        }

            //        if (token.stop_requested()) {
            //            stopSourcePtr->request_stop();
            //        }
            //        });
            //}
            if (token.stop_requested()) {
                stopSource.request_stop();
            }

            // Setup timeout timer if specified
            std::unique_ptr<boost::asio::steady_timer> timer;
            auto timeoutMs = command->Header().Timeout();
            if (timeoutMs.has_value() && timeoutMs.value() > 0) {
                timer = std::make_unique<boost::asio::steady_timer>(
                    co_await boost::asio::this_coro::executor,
                    std::chrono::milliseconds(timeoutMs.value()));

                timer->async_wait([stopSourcePtr = &stopSource](const boost::system::error_code& ec) {
                    if (!ec) {
                        stopSourcePtr->request_stop();
                    }
                    });
            }

            auto [handler, isAsync] = CreateHandler(
                std::type_index(typeid(*command)),
                connection);

            // Check if command and version are supported
            const auto& cmdName = command->Header().Name();
            const auto& cmdVersion = command->Header().Version();

            auto itSupported = m_messagesSupported.find(cmdName);
            if (itSupported == m_messagesSupported.end()) {
                auto errorMsg = std::format("{} Команда не поддерживается сервисом.", cmdName);
                co_await handler->HandleError(command,
                    std::make_exception_ptr(std::runtime_error(errorMsg)));
                co_return;
            }

            const auto& supportedInfo = itSupported->second;
            if (std::ranges::find(supportedInfo.Versions, cmdVersion) == supportedInfo.Versions.end()) {
                auto errorMsg = std::format("{} Версия команды ({}) не поддерживается сервисом",
                    cmdName, cmdVersion);
                co_await handler->HandleError(command,
                    std::make_exception_ptr(std::runtime_error(errorMsg)));
                co_return;
            }

            // Process the command
            if (isAsync) {
                std::exception_ptr exPtr = nullptr;
                bool hadException = false;

                try {
                    logger_->trace(std::format("{}() - Выполняется {} id:{}", __FUNCTION__, cmdName, command->Header().RequestId().value()));

                    co_await handler->Handle(command, stopToken);

                    logger_->trace(std::format("{}() - Завершено {} id:{}", __FUNCTION__, cmdName, command->Header().RequestId().value()));
                }
                catch (const std::exception& ex) {
                    logger_->error(std::format("{}() - Перехвачено исключение при выполнении асинхронной команды {} id:{}.\n{}",
                        __FUNCTION__, cmdName, command->Header().RequestId().value(), ex.what()));

                    exPtr = std::current_exception();

                    // Check if it's a cancellation/timeout exception and repackage if needed
                    try {
                        std::rethrow_exception(exPtr);
                    }
                    catch (const boost::system::system_error& se) {
                        if (se.code() == boost::asio::error::operation_aborted) {
                            exPtr = std::make_exception_ptr(
                                TimeoutCanceledException(ex.what(), exPtr, false));
                        }
                    }
                    catch (...) {
                        // leave exPtr as-is
                    }

                    hadException = true;
                }

                // Outside of the catch: suspension point is allowed here.
                if (hadException) {
                    co_await handler->HandleError(command, exPtr);
                }

                if (timer) {
                    timer->cancel();
                }
            }
            else {
                logger_->trace(std::format("{}() - Постановка обработчика команды в очередь для {} id:{}", __FUNCTION__,
                    cmdName, command->Header().RequestId().value()));

                co_await commandQueue_->EnqueueCommandAsync(
                    handler, connection, command, stopSource);
            }
        }
        catch (const std::exception& e) {
            // Check if it's a WebSocket exception
            try {
                std::rethrow_exception(std::current_exception());
            }
            catch (const boost::system::system_error& we) {
                logger_->warn(std::format("{}() - Исключение при ответе клиенту — предполагается, что клиент отключился. {}", __FUNCTION__,
                    we.what()));
            }
            catch (const std::exception& ex) {
                // Handle cancellation exceptions
                if (std::string(ex.what()).find("cancel") != std::string::npos) {
                    if (!connection || !command) {
                        throw std::invalid_argument("Недопустимый параметр в методе Dispatch");
                    }

                    timeoutEx = std::make_exception_ptr(
                        TimeoutCanceledException(ex.what(), std::current_exception(), false));

                    auto [handler, _] = CreateHandler(std::type_index(typeid(*command)), connection);
                    
					hasTimeout = true;
                }
                else {
                    throw;
                }
            }
        }

        if (hasTimeout && timeoutEx) {
            co_await handler->HandleError(command, timeoutEx);
		}
    }

    boost::asio::awaitable<void> CommandDispatcher::DispatchError(
        std::shared_ptr<IConnection> connection,
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::exception_ptr exception)
    {
        if (!connection) {
            throw std::invalid_argument("Недопустимый параметр в методе DispatchError: Connection");
        }
        if (!command) {
            throw std::invalid_argument("Недопустимый параметр в методе DispatchError: Command");
        }
        if (!exception) {
            throw std::invalid_argument("Недопустимый параметр в методе DispatchError: Exception");
        }

        auto [handler, _] = CreateHandler(std::type_index(typeid(*command)), connection);
        co_await handler->HandleError(command, exception);
    }

    boost::asio::awaitable<void> CommandDispatcher::RunAsync(
        std::shared_ptr<CancellationSource> cancellationSource)
    {
        co_await commandQueue_->RunAsync(cancellationSource->GetToken());
    }

    boost::asio::awaitable<bool> CommandDispatcher::AnyValidRequestID(
        std::shared_ptr<IConnection> connection,
        const std::vector<int>& requestIds,
        std::stop_token token)
    {
        co_return co_await commandQueue_->AnyValidRequestID(connection, requestIds, token);
    }

    boost::asio::awaitable<void> CommandDispatcher::CancelCommandsAsync(
        std::shared_ptr<IConnection> connection,
        const std::vector<int>& requestIds,
        std::stop_token token)
    {
        co_await commandQueue_->TryCancelItemsAsync(connection, requestIds, token);
    }

    void CommandDispatcher::StartCommandQueue(std::shared_ptr<CancellationSource> cancellationSource)
    {
        boost::asio::co_spawn(
            queueIoContext_,
            commandQueue_->RunAsync(cancellationSource->GetToken()),
            [logger = logger_](std::exception_ptr e) {
                if (!e) return;

                try {
                    std::rethrow_exception(e);
                }
                catch (const std::exception& ex) {
                    logger->error(std::format("CommandQueue failed: {}", ex.what()));
                }
            }
        );
    }

    std::pair<std::shared_ptr<ICommandHandler>, bool> CommandDispatcher::CreateHandler(
        std::type_index type,
        std::shared_ptr<IConnection> connection)
    {
        logger_->trace(std::format(
            "CommandDispatcher::CreateHandler this = {} type = {} handlers_count = {}",
            static_cast<const void*>(this),
            type.name(),
            messageHandlers_.size()));

        //for (const auto& [registeredType, details] : messageHandlers_)
        //{
        //    logger_->trace(std::format(
        //        "Зарегистрирован обработчик на this={} type={}",
        //        static_cast<const void*>(this),
        //        registeredType.name()));
        //}


        auto it = messageHandlers_.find(type);
        if (it == messageHandlers_.end()) {
            throw UnsupportedCommandException(
                std::format("Для типа не импортирован обработчик сообщений: {}", type.name()));
        }

        auto handler = it->second.factory(connection, GetDispatcherPtr(), logger_);


        if (!handler) {
            throw std::runtime_error(
                std::format("Не удалось создать обработчик для типа: {}", type.name()));
        }

        return { handler, it->second.isAsync };
    }

    void CommandDispatcher::Add(
        const std::vector<std::tuple<std::type_index,
        std::function<std::shared_ptr<ICommandHandler>(
            std::shared_ptr<IConnection>,
            std::shared_ptr<ICommandDispatcher>,
            std::shared_ptr<ILogger>)>,
        bool>>&types)
    {
        for (const auto& [messageType, factory, isAsync] : types) {
            messageHandlers_.emplace(
                messageType,
                HandlerDetails(messageType, factory, isAsync)
            );
        }
    }
}