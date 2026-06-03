#pragma once

#include <typeindex>
#include <vector>
#include <map>
#include <memory>
#include <stop_token>
#include <ranges>
#include <stdexcept>

#include "ICommandDispatcher.hpp"
#include "ICommandHandler.hpp"
#include "CommandQueue.hpp"
#include "CancellationSource.hpp"
#include "../core/MessageBase.hpp"
#include "../core/common/XFSConstants.hpp"
#include "../core/Acknowledge.hpp"
#include "../core/MessageTypeInfo.hpp"
#include "../core/Logger/ILogger.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>

namespace XFS4IoTServer
{

    // где-либо при инициализации
    //auto dispatcher = std::make_shared<CommandDispatcher>(services, logger, assemblyName);

    class CommandDispatcher : public virtual ICommandDispatcher
        //, std::enable_shared_from_this<CommandDispatcher> 
    {

    protected:
        virtual std::shared_ptr<ICommandDispatcher> GetDispatcherPtr() = 0;

    private:
        struct HandlerDetails {
            std::type_index messageType;
            std::function<std::shared_ptr<ICommandHandler>(
                std::shared_ptr<IConnection>,
                std::shared_ptr<ICommandDispatcher>,
                std::shared_ptr<ILogger>)> factory;
            bool isAsync;

            HandlerDetails(
                std::type_index msgType,
                std::function<std::shared_ptr<ICommandHandler>(
                    std::shared_ptr<IConnection>,
                    std::shared_ptr<ICommandDispatcher>,
                    std::shared_ptr<ILogger>)> fact,
                bool async)
                : messageType(msgType), factory(std::move(fact)), isAsync(async) {
            }
        };

        std::map<std::type_index, HandlerDetails> messageHandlers_;
        std::shared_ptr<ILogger> logger_;
        std::vector<XFS4IoT::XFSConstants::ServiceClass> serviceClasses_;
        std::unique_ptr<CommandQueue> commandQueue_;

    public:
        std::map<std::string, XFS4IoT::MessageTypeInfo> m_messagesSupported;
        std::map<std::string, XFS4IoT::MessageTypeInfo> m_eventsSupported;

        // Конструктор по умолчанию (добавлен для устранения ошибки E0291)
        CommandDispatcher()
            : logger_(nullptr)
            , serviceClasses_()
            , commandQueue_(nullptr)
            , queueWorkGuard_(boost::asio::make_work_guard(queueIoContext_))
        {
        }

        ~CommandDispatcher()
        {
            queueWorkGuard_.reset();
            queueIoContext_.stop();

            if (queueThread_.joinable()) {
                queueThread_.join();
            }
        }


        CommandDispatcher(
            const std::vector<XFS4IoT::XFSConstants::ServiceClass>& services,
            std::shared_ptr<ILogger> logger,
            const std::string& assemblyName = "");

        boost::asio::awaitable<void> Dispatch(
            std::shared_ptr<IConnection> connection,
            std::shared_ptr<XFS4IoT::MessageBase> command,
            std::stop_token token) override;

        boost::asio::awaitable<void> DispatchError(
            std::shared_ptr<IConnection> connection,
            std::shared_ptr<XFS4IoT::MessageBase> command,
            std::exception_ptr exception) override;

        boost::asio::awaitable<void> RunAsync(
            std::shared_ptr<CancellationSource> cancellationSource) override;

        boost::asio::awaitable<bool> AnyValidRequestID(
            std::shared_ptr<IConnection> connection,
            const std::vector<int>& requestIds,
            std::stop_token token) override;

        boost::asio::awaitable<void> CancelCommandsAsync(
            std::shared_ptr<IConnection> connection,
            const std::vector<int>& requestIds,
            std::stop_token token) override;

        void StartCommandQueue(
            std::shared_ptr<CancellationSource> cancellationSource);

        auto GetCommands() const {
            return messageHandlers_
                | std::views::keys;
        }

        const std::vector<XFS4IoT::XFSConstants::ServiceClass>& GetServiceClasses() const {
            return serviceClasses_;
        }

        void SetServiceClasses(std::vector<XFS4IoT::XFSConstants::ServiceClass>& services)  noexcept {
            serviceClasses_ = std::move(services);
        }

        // Registration function for command handlers
        template<typename MessageType, typename HandlerType>
        void RegisterHandler(XFS4IoT::XFSConstants::ServiceClass serviceClass, bool isAsync = false) {
            if (std::ranges::find(serviceClasses_, serviceClass) == serviceClasses_.end()) {
                return;
            }

            auto factory = [](std::shared_ptr<IConnection> conn,
                std::shared_ptr<ICommandDispatcher> dispatcher,
                std::shared_ptr<ILogger> log) -> std::shared_ptr<ICommandHandler> {
                    return std::make_shared<HandlerType>(conn, dispatcher, log);
                };

            messageHandlers_.emplace(
                std::type_index(typeid(MessageType)),
                HandlerDetails(std::type_index(typeid(MessageType)), factory, isAsync)
            );
        }

    private:
        boost::asio::io_context queueIoContext_;
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> queueWorkGuard_;
        std::jthread queueThread_;
        std::pair<std::shared_ptr<ICommandHandler>, bool> CreateHandler(
            std::type_index type,
            std::shared_ptr<IConnection> connection);

        void Add(const std::vector<std::tuple<std::type_index,
            std::function<std::shared_ptr<ICommandHandler>(
                std::shared_ptr<IConnection>,
                std::shared_ptr<ICommandDispatcher>,
                std::shared_ptr<ILogger>)>,
            bool>>&types);
    };

    // Exception classes
    class UnsupportedCommandException : public std::runtime_error {
    public:
        explicit UnsupportedCommandException(const std::string& msg)
            : std::runtime_error(msg) {}
    };

    class TimeoutCanceledException : public std::runtime_error
    {
    public:
        TimeoutCanceledException(const std::string& msg,
            std::exception_ptr inner,
            bool cancelRequested)
            : std::runtime_error(msg)
            , innerException_(inner)
            , cancelRequested_(cancelRequested) {
        }

        std::exception_ptr GetInnerException() const { return innerException_; }
        bool IsCancelRequested() const { return cancelRequested_; }

    private:
        std::exception_ptr innerException_;
        bool cancelRequested_;
    };
}