#pragma once

#include <boost/asio/awaitable.hpp>
#include <any>
#include "../core/MessageBase.hpp"

namespace XFS4IoTServer
{
    class IConnection
    {
    public:
        virtual ~IConnection() = default;

        virtual boost::asio::awaitable<void> SendMessageAsync(
            std::shared_ptr<XFS4IoT::MessageBase> message) = 0;
    };
}
