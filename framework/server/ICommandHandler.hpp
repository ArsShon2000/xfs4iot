#pragma once

#include <future>
#include <memory>
#include <exception>
#include <stop_token>
//#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/awaitable.hpp>

#include "../core/MessageBase.hpp"

namespace XFS4IoTServer
{
    class ICommandHandler
    {
    public:
        virtual ~ICommandHandler() = default;

        // Must have a constructor of the form:
        // ICommandHandler(std::shared_ptr<IConnection>, 
        //                 std::shared_ptr<ICommandDispatcher>, 
        //                 std::shared_ptr<ILogger>)

        virtual boost::asio::awaitable<void> Handle(
            std::shared_ptr<XFS4IoT::MessageBase> command,
            std::stop_token cancel) = 0;

        virtual boost::asio::awaitable<void> HandleError(
            std::shared_ptr<XFS4IoT::MessageBase> command,
            std::exception_ptr commandException) = 0;

        virtual boost::asio::awaitable<void> CommandPostProcessing(
            std::shared_ptr<void> result)
        {
            co_return;
        }
    };
}
