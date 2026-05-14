#pragma once

#include <memory>
#include "../../StorageServiceProvider/IStorageService.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashInEndEvents.hpp"
#include "../../CashManagementServiceProvider/Events/StorageItemErrorCommandEvents.hpp"

namespace XFS4IoTFramework::CashAcceptor
{

    /// <summary>
    /// CashInEndCommandEvents - handles events for CashInEnd command
    /// </summary>
    class CashInEndCommandEvents final : public XFS4IoTFramework::CashManagement::StorageItemErrorCommandEvents
    {
    public:
        /// <summary>
        /// Constructor with storage service and event interface
        /// </summary>
        CashInEndCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<XFS4IoTFramework::CashManagement::ICashInEndEvents> events)
            : StorageItemErrorCommandEvents(storage, events)
        {
        }

        // Disable copy and move
        CashInEndCommandEvents(const CashInEndCommandEvents&) = delete;
        CashInEndCommandEvents& operator=(const CashInEndCommandEvents&) = delete;
        CashInEndCommandEvents(CashInEndCommandEvents&&) = delete;
        CashInEndCommandEvents& operator=(CashInEndCommandEvents&&) = delete;

        virtual ~CashInEndCommandEvents() = default;
    };
}