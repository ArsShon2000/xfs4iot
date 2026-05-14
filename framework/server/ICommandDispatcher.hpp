#pragma once

#include <future>
#include <vector>
#include <exception>
#include <stop_token>
#include "IConnection.hpp"
#include "../core/MessageBase.hpp"
#include <boost/asio/awaitable.hpp>
#include "CancellationSource.hpp"


namespace XFS4IoTServer
{
    class ICommandDispatcher {
    public:
        virtual ~ICommandDispatcher() = default;
        virtual boost::asio::awaitable<void> Dispatch(
            std::shared_ptr<IConnection> connection,
            std::shared_ptr<XFS4IoT::MessageBase> command,
            std::stop_token token) = 0;

        virtual boost::asio::awaitable<void> DispatchError(
            std::shared_ptr<IConnection> connection,
            std::shared_ptr<XFS4IoT::MessageBase> command,
            std::exception_ptr exception) = 0;

        virtual boost::asio::awaitable<void> RunAsync(
            std::shared_ptr<CancellationSource> cancellationSource) = 0;

        virtual boost::asio::awaitable<bool> AnyValidRequestID(
            std::shared_ptr<IConnection> connection,
            const std::vector<int>& requestIds,
            std::stop_token token) = 0;

        virtual boost::asio::awaitable<void> CancelCommandsAsync(
            std::shared_ptr<IConnection> connection,
            const std::vector<int>& requestIds,
            std::stop_token token) = 0;
    };
}
