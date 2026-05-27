#pragma once

#include <memory>

#include "../../../server/IConnection.hpp"

namespace XFS4IoTFramework::Common
{
    class CommonEvents
    {
    public:
        CommonEvents(
            std::shared_ptr<XFS4IoTServer::IConnection> connection,
            int requestId)
            : connection_(std::move(connection))
            , requestId_(requestId)
        {
        }

        virtual ~CommonEvents() = default;

    protected:
        std::shared_ptr<XFS4IoTServer::IConnection> connection_;
        int requestId_;
    };
}