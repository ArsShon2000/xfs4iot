#pragma once

#include "CommonEvents.hpp"
#include "../../ServiceInterfaces/Common/ISetVersionsEvents.hpp"

namespace XFS4IoTFramework::Common
{
    class SetVersionsEvents final
        : public CommonEvents
        , public ISetVersionsEvents
    {
    public:
        SetVersionsEvents(
            std::shared_ptr<XFS4IoTServer::IConnection> connection,
            int requestId)
            : CommonEvents(std::move(connection), requestId)
        {
        }
    };
}