#pragma once

#include <memory>

#include "CashAcceptorEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/IGetCashInStatusEvents.hpp"

namespace XFS4IoTFramework::CashAcceptor
{
    class GetCashInStatusEvents final
        : public CashAcceptorEvents
        , public IGetCashInStatusEvents
    {
    public:
        GetCashInStatusEvents(
            std::shared_ptr<XFS4IoTServer::IConnection> connection,
            int requestId)
            : CashAcceptorEvents(std::move(connection), requestId)
        {
        }
    };
}