#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <boost/asio/awaitable.hpp>
#include "ItemErrorCommandEvents.hpp"
#include "../../StorageServiceProvider/Events/StorageErrorCommandEvent.hpp"
#include "../../StorageServiceProvider/IStorageService.hpp"
#include "../../StorageServiceProvider/CashUnit.hpp"

namespace XFS4IoTFramework::CashManagement
{
    /// <summary>
    /// Retract command events handler
    /// </summary>
    class RetractCommandEvents final : public ItemErrorCommandEvents
    {
    public:
        /// <summary>
        /// Incomplete retract reason enumeration
        /// </summary>
        enum class IncompleteRetractReasonEnum
        {
            RetractFailure,
            RetractAreaFull,
            ForeignItemsDetected,
            InvalidBunch
        };

        /// <summary>
        /// Constructor
        /// </summary>
        RetractCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<IRetractEvents> events)
            : ItemErrorCommandEvents(events)
            , storageErrorCommandEvent_(std::make_shared<XFS4IoTFramework::Storage::StorageErrorCommandEvent>(storage, events))
            , retractEvents_(events)
        {
        }


        virtual ~RetractCommandEvents() = default;

        /// <summary>
        /// Trigger incomplete retract event
        /// </summary>
        boost::asio::awaitable<void> incompleteRetractEvent(
            const std::unordered_map<std::string, std::shared_ptr<XFS4IoTFramework::Storage::CashUnitCountClass>>& movements,
            IncompleteRetractReasonEnum reason)
        {
            if (!retractEvents_)
            {
                throw std::logic_error(
                    std::string("Unreachable code. ") + "incompleteRetractEvent");
            }

            // Теперь itemMovementResult хранит map значений StorageCashInClass (не shared_ptr)
            std::optional<std::unordered_map<std::string, XFS4IoT::CashManagement::StorageCashInClass>>
                itemMovementResult;

            if (!movements.empty())
            {
                std::unordered_map<std::string, XFS4IoT::CashManagement::StorageCashInClass> resultMap;

                for (const auto& [movementKey, movementValue] : movements)
                {
                    // Build deposited counts
                    std::unordered_map<std::string, std::shared_ptr<XFS4IoT::CashManagement::StorageCashCountClass>> deposited;
                    for (const auto& [itemKey, itemValue] : movementValue->GetStorageCashInCount()->GetDeposited()->GetItemCounts())
                    {
                        deposited[itemKey] = std::make_shared<XFS4IoT::CashManagement::StorageCashCountClass>(
                            itemValue.GetFit(),
                            itemValue.GetUnfit(),
                            itemValue.GetSuspect(),
                            itemValue.GetCounterfeit(),
                            itemValue.GetInked()
                        );
                    }

                    // Build retracted counts
                    std::unordered_map<std::string, std::shared_ptr<XFS4IoT::CashManagement::StorageCashCountClass>> retracted;
                    for (const auto& [itemKey, itemValue] : movementValue->GetStorageCashInCount()->GetRetracted()->GetItemCounts())
                    {
                        retracted[itemKey] = std::make_shared<XFS4IoT::CashManagement::StorageCashCountClass>(
                            itemValue.GetFit(),
                            itemValue.GetUnfit(),
                            itemValue.GetSuspect(),
                            itemValue.GetCounterfeit(),
                            itemValue.GetInked()
                        );
                    }

                    // Build rejected counts
                    std::unordered_map<std::string, std::shared_ptr<XFS4IoT::CashManagement::StorageCashCountClass>> rejected;
                    for (const auto& [itemKey, itemValue] : movementValue->GetStorageCashInCount()->GetRejected()->GetItemCounts())
                    {
                        rejected[itemKey] = std::make_shared<XFS4IoT::CashManagement::StorageCashCountClass>(
                            itemValue.GetFit(),
                            itemValue.GetUnfit(),
                            itemValue.GetSuspect(),
                            itemValue.GetCounterfeit(),
                            itemValue.GetInked()
                        );
                    }

                    // Build distributed counts
                    std::unordered_map<std::string, std::shared_ptr<XFS4IoT::CashManagement::StorageCashCountClass>> distributed;
                    for (const auto& [itemKey, itemValue] : movementValue->GetStorageCashInCount()->GetDistributed()->GetItemCounts())
                    {
                        distributed[itemKey] = std::make_shared<XFS4IoT::CashManagement::StorageCashCountClass>(
                            itemValue.GetFit(),
                            itemValue.GetUnfit(),
                            itemValue.GetSuspect(),
                            itemValue.GetCounterfeit(),
                            itemValue.GetInked()
                        );
                    }

                    // Build transport counts
                    std::unordered_map<std::string, std::shared_ptr<XFS4IoT::CashManagement::StorageCashCountClass>> transport;
                    for (const auto& [itemKey, itemValue] : movementValue->GetStorageCashInCount()->GetTransport()->GetItemCounts())
                    {
                        transport[itemKey] = std::make_shared<XFS4IoT::CashManagement::StorageCashCountClass>(
                            itemValue.GetFit(),
                            itemValue.GetUnfit(),
                            itemValue.GetSuspect(),
                            itemValue.GetCounterfeit(),
                            itemValue.GetInked()
                        );
                    }

                    // Create StorageCashCountsClass instances (shared_ptrs are OK)
                    auto depositedCount = std::make_shared<XFS4IoT::CashManagement::StorageCashCountsClass>(
                        movementValue->GetStorageCashInCount()->GetDeposited()->GetUnrecognized());
                    depositedCount->setExtendedProperties(deposited);

                    auto retractedCount = std::make_shared<XFS4IoT::CashManagement::StorageCashCountsClass>(
                        movementValue->GetStorageCashInCount()->GetRetracted()->GetUnrecognized());
                    retractedCount->setExtendedProperties(retracted);

                    auto rejectedCount = std::make_shared<XFS4IoT::CashManagement::StorageCashCountsClass>(
                        movementValue->GetStorageCashInCount()->GetRejected()->GetUnrecognized());
                    rejectedCount->setExtendedProperties(rejected);

                    auto distributedCount = std::make_shared<XFS4IoT::CashManagement::StorageCashCountsClass>(
                        movementValue->GetStorageCashInCount()->GetDistributed()->GetUnrecognized());
                    distributedCount->setExtendedProperties(distributed);

                    auto transportCount = std::make_shared<XFS4IoT::CashManagement::StorageCashCountsClass>(
                        movementValue->GetStorageCashInCount()->GetTransport()->GetUnrecognized());
                    transportCount->setExtendedProperties(transport);

                    // Create StorageCashInClass as a value and place into resultMap
                    XFS4IoT::CashManagement::StorageCashInClass storageCashIn(
                        movementValue->GetStorageCashInCount()->GetRetractOperations(),
                        depositedCount,
                        retractedCount,
                        rejectedCount,
                        distributedCount,
                        transportCount
                    );

                    resultMap.emplace(movementKey, std::move(storageCashIn));
                }

                itemMovementResult = std::move(resultMap);
            }

            // Convert reason enum
            XFS4IoT::CashManagement::Events::IncompleteRetractEventPayloadData::ReasonEnum xfsReason;
            switch (reason)
            {
            case IncompleteRetractReasonEnum::ForeignItemsDetected:
                xfsReason = XFS4IoT::CashManagement::Events::IncompleteRetractEventPayloadData::ReasonEnum::ForeignItemsDetected;
                break;
            case IncompleteRetractReasonEnum::InvalidBunch:
                xfsReason = XFS4IoT::CashManagement::Events::IncompleteRetractEventPayloadData::ReasonEnum::InvalidBunch;
                break;
            case IncompleteRetractReasonEnum::RetractAreaFull:
                xfsReason = XFS4IoT::CashManagement::Events::IncompleteRetractEventPayloadData::ReasonEnum::RetractAreaFull;
                break;
            default:
                xfsReason = XFS4IoT::CashManagement::Events::IncompleteRetractEventPayloadData::ReasonEnum::RetractFailure;
                break;
            }

            // Create and send event
            auto payload = std::make_shared<XFS4IoT::CashManagement::Events::IncompleteRetractEventPayloadData>(
                itemMovementResult,
                xfsReason
            );

            co_await retractEvents_->IncompleteRetractEvent(*payload);
        }

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

    private:
        std::shared_ptr<XFS4IoTFramework::Storage::StorageErrorCommandEvent> storageErrorCommandEvent_;
        std::shared_ptr<IRetractEvents> retractEvents_;
    };
}