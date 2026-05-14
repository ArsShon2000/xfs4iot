#pragma once

#include <memory>

#include "CashAcceptorEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashInStartEvents.hpp"

namespace XFS4IoTFramework::CashAcceptor
{
    class CashInStartEvents final
        : public CashAcceptorEvents
        , public ICashInStartEvents
    {
    public:
        CashInStartEvents(
            std::shared_ptr<XFS4IoTServer::IConnection> connection,
            int requestId)
            : CashAcceptorEvents(std::move(connection), requestId)
        {
        }
    };
}