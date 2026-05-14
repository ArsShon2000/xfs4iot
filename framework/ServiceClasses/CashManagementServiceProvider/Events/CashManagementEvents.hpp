#pragma once

#include <memory>
#include "../../../server/IConnection.hpp"

namespace XFS4IoTFramework::CashManagement
{
    class CashManagementEvents
    {
    public:
        CashManagementEvents(
            std::shared_ptr<XFS4IoTServer::IConnection> connection,
            int requestId)
            : connection_(std::move(connection))
            , requestId_(requestId)
        {
        }

        virtual ~CashManagementEvents() = default;

    protected:
        std::shared_ptr<XFS4IoTServer::IConnection> connection_;
        int requestId_;
    };
}