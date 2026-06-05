#pragma once

#include <memory>
#include "../../StorageServiceProvider/IStorageService.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashInEndEvents.hpp"
#include "../../CashManagementServiceProvider/Events/StorageItemErrorCommandEvents.hpp"

namespace XFS4IoTFramework::CashAcceptor
{
    class CashInEndCommandEvents final : public XFS4IoTFramework::CashManagement::StorageItemErrorCommandEvents
    {
    public:
        CashInEndCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<ICashInEndEvents> events)
            : StorageItemErrorCommandEvents(std::move(storage), std::move(events))
        {
        }
    };
}