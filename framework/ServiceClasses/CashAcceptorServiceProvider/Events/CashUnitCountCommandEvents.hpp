#pragma once

#include "../../CashManagementServiceProvider/Events/StorageItemErrorCommandEvents.hpp"
#include "../../StorageServiceProvider/IStorageService.hpp"
#include <memory>

namespace XFS4IoTFramework::CashAcceptor
{
    /// <summary>
    /// CashUnitCount Command Events
    /// Provides event handling for the CashUnitCount command
    /// </summary>
    class CashUnitCountCommandEvents final : public XFS4IoTFramework::CashManagement::StorageItemErrorCommandEvents
    {
    public:
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="storage">Storage service interface</param>
        /// <param name="events">CashUnitCount events interface</param>
        CashUnitCountCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<XFS4IoTFramework::CashManagement::ICashUnitCountEvents> events)
            : StorageItemErrorCommandEvents(storage, events)
        {
        }

        /// <summary>
        /// Virtual destructor
        /// </summary>
        virtual ~CashUnitCountCommandEvents() = default;

        // Deleted copy/move constructors and assignment operators
        CashUnitCountCommandEvents(const CashUnitCountCommandEvents&) = delete;
        CashUnitCountCommandEvents& operator=(const CashUnitCountCommandEvents&) = delete;
        CashUnitCountCommandEvents(CashUnitCountCommandEvents&&) = delete;
        CashUnitCountCommandEvents& operator=(CashUnitCountCommandEvents&&) = delete;
    };

} // namespace XFS4IoTFramework::CashAcceptor