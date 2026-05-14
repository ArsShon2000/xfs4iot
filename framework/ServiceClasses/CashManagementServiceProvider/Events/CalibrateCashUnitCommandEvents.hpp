#pragma once

#include "../../StorageServiceProvider/Events/StorageErrorCommandEvent.hpp"
#include "../../StorageServiceProvider/IStorageService.hpp"
#include "../../../ServiceClasses/CashManagementServiceProvider/Events/StorageErrorCommonCommandEvents.hpp"
#include <memory>

namespace XFS4IoTFramework::CashManagement
{
    /// <summary>
    /// Reset Command Events
    /// Provides event handling for the Reset command
    /// </summary>
    class CalibrateCashUnitCommandEvents final : public XFS4IoTFramework::CashManagement::StorageErrorCommonCommandEvents
    {
    public:
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="storage">Storage service interface</param>
        /// <param name="events">Reset events interface</param>
        CalibrateCashUnitCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<ICalibrateCashUnitEvents> events)
            : XFS4IoTFramework::CashManagement::StorageErrorCommonCommandEvents(storage, events)
        {
        }

        /// <summary>
        /// Virtual destructor
        /// </summary>
        virtual ~CalibrateCashUnitCommandEvents() = default;

        // Deleted copy/move constructors and assignment operators
        CalibrateCashUnitCommandEvents(const CalibrateCashUnitCommandEvents&) = delete;
        CalibrateCashUnitCommandEvents& operator=(const CalibrateCashUnitCommandEvents&) = delete;
        CalibrateCashUnitCommandEvents(CalibrateCashUnitCommandEvents&&) = delete;
        CalibrateCashUnitCommandEvents& operator=(CalibrateCashUnitCommandEvents&&) = delete;
    };

} // namespace XFS4IoTFramework::CashManagement