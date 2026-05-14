#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <queue>
#include <stop_token>
#include "ICommandHandler.hpp"
#include "IConnection.hpp"
#include <boost/asio/awaitable.hpp>
#include "../core/MessageBase.hpp"
#include "../core/Logger/ILogger.hpp"
#include <boost/asio/steady_timer.hpp>

//using namespace XFS4IoT;

namespace XFS4IoTServer
{
    class CommandQueue
    {
    public:
        struct QueueItem
        {
            std::shared_ptr<ICommandHandler> commandHandler;
            std::shared_ptr<IConnection> connection;
            std::shared_ptr<XFS4IoT::MessageBase> command;
            std::stop_source stopSource;

            QueueItem(std::shared_ptr<ICommandHandler> handler,
                std::shared_ptr<IConnection> conn,
                std::shared_ptr<XFS4IoT::MessageBase> cmd,
                std::stop_source src)
                : commandHandler(std::move(handler))
                , connection(std::move(conn))
                , command(std::move(cmd))
                , stopSource(std::move(src)) {
            }
        };

        explicit CommandQueue(std::shared_ptr<ILogger> logger);

        boost::asio::awaitable<void> EnqueueCommandAsync(
            std::shared_ptr<ICommandHandler> commandHandler,
            std::shared_ptr<IConnection> connection,
            std::shared_ptr<XFS4IoT::MessageBase> command,
            std::stop_source stopSource);

        boost::asio::awaitable<bool> AnyValidRequestID(
            std::shared_ptr<IConnection> connection,
            const std::vector<int>& requestIds,
            std::stop_token token);

        boost::asio::awaitable<void> TryCancelItemsAsync(
            std::shared_ptr<IConnection> connection,
            const std::vector<int>& requestIds,
            std::stop_token token);

        boost::asio::awaitable<void> RunAsync(std::stop_token token);

    private:
        boost::asio::awaitable<void> TryCancelAllAsync(
            std::shared_ptr<IConnection> connection,
            std::stop_token token);

        boost::asio::awaitable<void> TryCancelSpecificAsync(
            std::shared_ptr<IConnection> connection,
            const std::vector<int>& requestIds,
            std::stop_token token);

        boost::asio::awaitable<QueueItem> ReceiveItemAsync(std::stop_token token);

        class AsyncAutoResetEvent
        {
        public:
            boost::asio::awaitable<void> WaitAsync(std::stop_token token);
            void Set();
            void RemovedItem();

        private:
            std::mutex mutex_;
            std::queue<std::shared_ptr<boost::asio::steady_timer>> waiters_;
            int signaled_ = 0;
        };

        class DisposableLock
        {
        public:
            explicit DisposableLock(std::mutex& mtx) : lock_(mtx) {}
            ~DisposableLock() = default;

            DisposableLock(const DisposableLock&) = delete;
            DisposableLock& operator=(const DisposableLock&) = delete;
            DisposableLock(DisposableLock&&) = default;
            DisposableLock& operator=(DisposableLock&&) = default;

        private:
            std::unique_lock<std::mutex> lock_;
        };

        std::mutex syncMutex_;
        std::vector<QueueItem> contents_;
        AsyncAutoResetEvent newItemEvent_;
        std::optional<QueueItem> currentCommand_;
        bool currentCommandCancelRequested_ = false;
        std::shared_ptr<ILogger> logger_;
    };
}