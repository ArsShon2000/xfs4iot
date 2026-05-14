#include "CommandQueue.hpp"
#include "ICommandHandler.hpp"
#include "IConnection.hpp"
#include "CommandDispatcher.hpp"
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/use_awaitable.hpp>

namespace XFS4IoTServer
{
    CommandQueue::CommandQueue(std::shared_ptr<ILogger> logger)
        : logger_(std::move(logger))
    {
        if (!logger_) {
            throw std::invalid_argument("Логгер не может быть null");
        }
    }

    boost::asio::awaitable<void> CommandQueue::EnqueueCommandAsync(
        std::shared_ptr<ICommandHandler> commandHandler,
        std::shared_ptr<IConnection> connection,
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::stop_source stopSource)
    {
        {
            std::unique_lock<std::mutex> lock(syncMutex_);

            contents_.emplace_back(
                std::move(commandHandler),
                std::move(connection),
                std::move(command),
                std::move(stopSource)
            );

            newItemEvent_.Set();
        }
        co_return;
    }

    boost::asio::awaitable<bool> CommandQueue::AnyValidRequestID(
        std::shared_ptr<IConnection> connection,
        const std::vector<int>& requestIds,
        std::stop_token token)
    {
		// Если список ID пустой, считаем, что проверка пройдена успешно (можно отменять все команды для данного соединения)
        if (requestIds.empty()) {
            co_return true;
        }

        std::unique_lock<std::mutex> lock(syncMutex_);

		// прроверка выполняющейся команды - так как может быть только одна выполняющаяся команда, то проверяем её отдельно
        if (currentCommand_.has_value() &&
            currentCommand_->connection == connection)
        {
            int currentId = currentCommand_->command->Header().RequestId().value();
            if (std::ranges::find(requestIds, currentId) != requestIds.end()) {
                co_return true;
            }
        }

		// проверка очереди - так как может быть несколько команд с одинаковыми ID, то проходим циклом и проверяем все совпадения
        for (const auto& item : contents_) {
            if (item.connection == connection) {
                int queuedId = item.command->Header().RequestId().value();
                if (std::ranges::find(requestIds, queuedId) != requestIds.end()) {
                    co_return true;
                }
            }
        }

        co_return false;
    }

    boost::asio::awaitable<void> CommandQueue::TryCancelItemsAsync(
        std::shared_ptr<IConnection> connection,
        const std::vector<int>& requestIds,
        std::stop_token token)
    {
        try {
            if (requestIds.empty()) {
                co_await TryCancelAllAsync(connection, token);
            }
            else {
                co_await TryCancelSpecificAsync(connection, requestIds, token);
            }
        }
        catch (const std::exception& ex) {
            logger_->warn(std::format("{}() - Перехвачено исключение внутри TryCancelItemsAsync.\n{}", __FUNCTION__, ex.what()));
            throw;
        }
    }

    boost::asio::awaitable<void> CommandQueue::TryCancelAllAsync(
        std::shared_ptr<IConnection> connection,
        std::stop_token token)
    {
        std::unique_lock<std::mutex> lock(syncMutex_);

		// Отмена выполняющейся команды, если она принадлежит этому соединению
        if (currentCommand_.has_value() && currentCommand_->connection == connection) {
            currentCommand_->stopSource.request_stop();
            currentCommandCancelRequested_ = true;
        }

		// Отмена всех команд в очереди, принадлежащих этому соединению - проходим циклом и удаляем все совпадения
        auto it = contents_.begin();
        while (it != contents_.end()) {
            if (it->connection == connection) {
				// Уведомляем обработчик о том, что команда отменена, перед удалением из очереди
                co_await it->commandHandler->HandleError(
                    it->command,
                    std::make_exception_ptr(TimeoutCanceledException("", nullptr, true))
                );

                it = contents_.erase(it);
                newItemEvent_.RemovedItem();
            }
            else {
                ++it;
            }
        }
    }

    boost::asio::awaitable<void> CommandQueue::TryCancelSpecificAsync(
        std::shared_ptr<IConnection> connection,
        const std::vector<int>& requestIds,
        std::stop_token token)
    {
        std::unique_lock<std::mutex> lock(syncMutex_);

		// отмена конкретных команд - сначала проверяем выполняющуюся команду, а затем очередь
        for (int id : requestIds) {
			// проверяем выполняющуюся команду
            if (currentCommand_.has_value() &&
                currentCommand_->command->Header().RequestId() == id &&
                currentCommand_->connection == connection)
            {
                currentCommand_->stopSource.request_stop();
                currentCommandCancelRequested_ = true;
            }
            else {
				// проверяем очередь - так как может быть несколько команд с одинаковыми ID, то проходим циклом и удаляем все совпадения
                auto it = std::ranges::find_if(contents_,
                    [id, &connection](const QueueItem& item) {
                        return item.command->Header().RequestId() == id &&
                            item.connection == connection;
                    });

                if (it != contents_.end()) {
                    co_await it->commandHandler->HandleError(
                        it->command,
                        std::make_exception_ptr(TimeoutCanceledException("", nullptr, true))
                    );

                    contents_.erase(it);
                    newItemEvent_.RemovedItem();
                }
            }
        }
    }

    boost::asio::awaitable<CommandQueue::QueueItem> CommandQueue::ReceiveItemAsync(
        std::stop_token token)
    {
        co_await newItemEvent_.WaitAsync(token);

        std::unique_lock<std::mutex> lock(syncMutex_);

        if (contents_.empty()) {
            throw std::runtime_error("После сигнала NewItemEvent ни одного элемента не найдено");
        }

        if (currentCommand_.has_value()) {
            throw std::runtime_error("Ожидалось, что CurrentCommand будет равен null");
        }

        currentCommand_ = std::move(contents_[0]);
        contents_.erase(contents_.begin());

        co_return currentCommand_.value();
    }

    boost::asio::awaitable<void> CommandQueue::RunAsync(std::stop_token token)
    {
		// Очередь для обработки команд - выполняется в отдельном потоке, чтобы не блокировать приём сообщений
        while (!token.stop_requested()) {
			// Ждем нового элемента для обработки
            auto item = co_await ReceiveItemAsync(token);
            auto& [handler, connection, command, stopSource] = item;

            std::exception_ptr exPtr = nullptr;
            bool hadException = false;

            try {
                logger_->trace(std::format("{}() - Выполняется {} id:{}", __FUNCTION__,
                        command->Header().Name(),
                        command->Header().RequestId().value()));

				// исключение может быть брошено как из-за отмены, так и из-за ошибки в обработчике, поэтому проверяем отмену до вызова обработчика
                if (stopSource.get_token().stop_requested()) {
                    throw std::runtime_error("Операция отменена");
                }

                co_await handler->Handle(command, stopSource.get_token());

                logger_->trace(std::format("{}() - Выполнена {} id:{}", __FUNCTION__,
                        command->Header().Name(),
                        command->Header().RequestId().value()));
            }
            catch (const std::exception& ex) {
                logger_->trace(std::format("{}() - Перехвачено исключение при выполнении {} id:{}.\n{}", __FUNCTION__,
                        command->Header().Name(),
                        command->Header().RequestId().value(),
                        ex.what()));

                std::exception_ptr exPtr = std::current_exception();

				// Проверяем, было ли исключение вызвано отменой, и если да, то оборачиваем его в TimeoutCanceledException с флагом cancelRequested
                try {
                    std::rethrow_exception(exPtr);
                }
                catch (const std::runtime_error& e) {
                    if (std::string(e.what()).find("cancel") != std::string::npos) {
                        bool cancelRequested = false;
                        {
                            std::unique_lock<std::mutex> lock(syncMutex_);
                            cancelRequested = currentCommandCancelRequested_;
                        }
                        exPtr = std::make_exception_ptr(
                            TimeoutCanceledException(e.what(), exPtr, cancelRequested)
                        );
                    }
                }

                hadException = true;
            }

            if (hadException) {
                co_await handler->HandleError(command, exPtr);
            }

			// После обработки команды очищаем CurrentCommand и сбрасываем флаг запроса отмены
            {
                std::unique_lock<std::mutex> lock(syncMutex_);
                currentCommand_.reset();
                currentCommandCancelRequested_ = false;
            }
        }
    }

    boost::asio::awaitable<void> CommandQueue::AsyncAutoResetEvent::WaitAsync(
        std::stop_token token)
    {
        auto executor = co_await boost::asio::this_coro::executor;

        std::shared_ptr<boost::asio::steady_timer> waiter;

        {
            std::lock_guard lock(mutex_);

            if (token.stop_requested()) {
                throw std::runtime_error("Операция отменена");
            }

            if (signaled_ > 0) {
                --signaled_;
                co_return;
            }

            waiter = std::make_shared<boost::asio::steady_timer>(executor);
            waiter->expires_at(std::chrono::steady_clock::time_point::max());
            waiters_.push(waiter);
        }

        boost::system::error_code ec;
        co_await waiter->async_wait(
            boost::asio::redirect_error(boost::asio::use_awaitable, ec)
        );

        if (token.stop_requested()) {
            throw std::runtime_error("Операция отменена");
        }

        if (ec && ec != boost::asio::error::operation_aborted) {
            throw boost::system::system_error(ec);
        }
    }

    void CommandQueue::AsyncAutoResetEvent::Set()
    {
        std::shared_ptr<boost::asio::steady_timer> waiter;

        {
            std::lock_guard lock(mutex_);

            if (!waiters_.empty()) {
                waiter = waiters_.front();
                waiters_.pop();
            }
            else {
                ++signaled_;
            }
        }

        if (waiter) {
            waiter->cancel();
        }
    }

    void CommandQueue::AsyncAutoResetEvent::RemovedItem()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (signaled_ > 0) {
            signaled_--;
        }
    }
}