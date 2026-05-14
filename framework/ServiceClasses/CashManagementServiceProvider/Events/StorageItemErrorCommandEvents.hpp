#pragma once

#include <memory>
#include <vector>
#include <string>
#include <boost/asio/awaitable.hpp>
#include "ItemErrorCommandEvents.hpp"
#include "../../StorageServiceProvider/Events/StorageErrorCommandEvent.hpp"
#include "../../StorageServiceProvider/IStorageService.hpp"

namespace XFS4IoTFramework::CashManagement
{
    /// <summary>
    /// Abstract base class for command events that handle storage and item errors
    /// </summary>
    class StorageItemErrorCommandEvents : public XFS4IoTFramework::CashManagement::ItemErrorCommandEvents
    {
    public:
        /// <summary>
        /// Constructor for CalibrateCashUnit events
        /// </summary>
        StorageItemErrorCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<ICalibrateCashUnitEvents> events)
            : ItemErrorCommandEvents(events)
            , storageErrorCommandEvent_(
                std::make_shared<XFS4IoTFramework::Storage::StorageErrorCommandEvent>(storage, events))
        {
        }

        /// <summary>
        /// Constructor for CashInEnd events
        /// </summary>
        StorageItemErrorCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<ICashInEndEvents> events)
            : ItemErrorCommandEvents(events)
            , storageErrorCommandEvent_(
                std::make_shared<XFS4IoTFramework::Storage::StorageErrorCommandEvent>(storage, events))
        {
        }

        /// <summary>
        /// Constructor for CashUnitCount events
        /// </summary>
        StorageItemErrorCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<ICashUnitCountEvents> events)
            : ItemErrorCommandEvents(events)
            , storageErrorCommandEvent_(
                std::make_shared<XFS4IoTFramework::Storage::StorageErrorCommandEvent>(storage, events))
        {
        }

        /*/// <summary>
        /// Constructor for Count events
        /// </summary>
        StorageItemErrorCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<ICountEvents> events)
            : ItemErrorCommandEvents(events)
            , storageErrorCommandEvent_(
                std::make_shared<XFS4IoTFramework::Storage::StorageErrorCommandEvent>(storage, events))
        {
        }

        /// <summary>
        /// Constructor for TestCashUnits events
        /// </summary>
        StorageItemErrorCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<ITestCashUnitsEvents> events)
            : ItemErrorCommandEvents(events)
            , storageErrorCommandEvent_(
                std::make_shared<XFS4IoTFramework::Storage::StorageErrorCommandEvent>(storage, events))
        {
        }*/

        virtual ~StorageItemErrorCommandEvents() = default;

        /// <summary>
        /// Trigger storage error event
        /// </summary>
        boost::asio::awaitable<void> storageErrorEvent(
            XFS4IoTFramework::Storage::FailureEnum failure,
            const std::vector<std::string>& cashUnitIds)
        {
            if (storageErrorCommandEvent_)
            {
                co_await storageErrorCommandEvent_->StorageErrorEvent(failure, cashUnitIds);
            }
            co_return;
        }

    protected:
        std::shared_ptr<XFS4IoTFramework::Storage::StorageErrorCommandEvent> getStorageErrorCommandEvent() const
        {
            return storageErrorCommandEvent_;
        }

    private:
        std::shared_ptr<XFS4IoTFramework::Storage::StorageErrorCommandEvent> storageErrorCommandEvent_;
    };
}