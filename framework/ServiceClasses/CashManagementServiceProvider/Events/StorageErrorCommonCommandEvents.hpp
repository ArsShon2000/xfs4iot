// StorageErrorCommonCommandEvents.hpp
#pragma once

#include <memory>
#include <vector>
#include <string>
#include <boost/asio/awaitable.hpp>
#include "ItemInfoAvailableCommandEvent.hpp"
#include "../../StorageServiceProvider/IStorageService.hpp"
#include "../../StorageServiceProvider/Events/StorageErrorCommandEvent.hpp"
#include "../../ServiceInterfaces/CashDispanser/IRejectEvents.hpp"

namespace XFS4IoTFramework::CashManagement
{
    /// <summary>
    /// Base class for command events that support storage errors
    /// </summary>
    class StorageErrorCommonCommandEvents : public ItemInfoAvailableCommandEvent
    {
    public:
        /// <summary>
        /// Constructor for Reset events
        /// </summary>
        StorageErrorCommonCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<IResetEvents> events)
            : ItemInfoAvailableCommandEvent(events)
            , storageErrorCommandEvent_(std::make_shared<XFS4IoTFramework::Storage::StorageErrorCommandEvent>(storage, events))
            , events_(events)
        {
        }

        /// <summary>
        /// Constructor for CashInRollback events
        /// </summary>
        StorageErrorCommonCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<ICashInRollbackEvents> events)
            : ItemInfoAvailableCommandEvent(events)
            , storageErrorCommandEvent_(std::make_shared<XFS4IoTFramework::Storage::StorageErrorCommandEvent>(storage, events))
        {
        }

        /// <summary>
        /// Constructor for PreparePresent events
        /// </summary>
        StorageErrorCommonCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<IPreparePresentEvents> events)
            : ItemInfoAvailableCommandEvent(events)
            , storageErrorCommandEvent_(std::make_shared<XFS4IoTFramework::Storage::StorageErrorCommandEvent>(storage, events))
        {
        }

        /// <summary>
        /// Constructor for Reject events
        /// </summary>
        StorageErrorCommonCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<XFS4IoTFramework::CashDispenser::IRejectEvents> events)
            : ItemInfoAvailableCommandEvent(events)
            , storageErrorCommandEvent_(std::make_shared<XFS4IoTFramework::Storage::StorageErrorCommandEvent>(storage, events))
        {
        }

        /// <summary>
        /// Constructor for Reject events
        /// </summary>
        StorageErrorCommonCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<XFS4IoTFramework::CashManagement::ICalibrateCashUnitEvents> events)
            : ItemInfoAvailableCommandEvent(events)
            , storageErrorCommandEvent_(std::make_shared<XFS4IoTFramework::Storage::StorageErrorCommandEvent>(storage, events))
        {
        }

        /// <summary>
        /// Virtual destructor
        /// </summary>
        virtual ~StorageErrorCommonCommandEvents() = default;

        /// <summary>
        /// Fire storage error event
        /// </summary>
        boost::asio::awaitable<void> StorageErrorEvent(
            XFS4IoTFramework::Storage::FailureEnum failure,
            const std::vector<std::string>& cashUnitIds)
        {
            if (storageErrorCommandEvent_)
            {
                co_return co_await storageErrorCommandEvent_->StorageErrorEvent(failure, cashUnitIds);
            }
            co_return;
        }

        //boost::asio::awaitable<void> MediaDetectedEvent(
        //    XFS4IoT::CashManagement::ItemTargetEnumEnum target,
        //    std::optional<std::string> unit = std::nullopt,
        //    std::optional<int> index = std::nullopt)
        //{
        //    auto payload =
        //        std::make_shared<XFS4IoT::CashManagement::Events::MediaDetectedEventPayloadData>(
        //            target,
        //            std::move(unit),
        //            index);

        //    co_await events_->MediaDetectedEvent(payload);
        //}

    protected:
        /// <summary>
        /// Get the storage error command event handler
        /// </summary>
        std::shared_ptr<XFS4IoTFramework::Storage::StorageErrorCommandEvent> GetStorageErrorCommandEvent() const
        {
            return storageErrorCommandEvent_;
        }

    private:
        std::shared_ptr<IResetEvents> events_;
        std::shared_ptr<XFS4IoTFramework::Storage::StorageErrorCommandEvent> storageErrorCommandEvent_;
    };

} // namespace XFS4IoTFramework::CashManagement