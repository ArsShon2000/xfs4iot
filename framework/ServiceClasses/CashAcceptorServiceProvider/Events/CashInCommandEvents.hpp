#pragma once

#include <optional>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <format>
#include <boost/asio/awaitable.hpp>
#include "CashInCommonCommandEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashInEvents.hpp"
#include "../../../core/CashAcceptor/Events/SubCashInEvent.hpp"
#include "../../StorageServiceProvider/CashUnit.hpp"
#include "../../../core/Storage/Events/StorageErrorEvent.hpp"
#include "../../StorageServiceProvider/IStorageService.hpp"
#include "../../StorageServiceProvider/Events/StorageErrorCommandEvent.hpp"

namespace XFS4IoTFramework::CashAcceptor
{
    /// <summary>
    /// CashInCommandEvents - Event handler for cash-in operations
    /// </summary>
    class CashInCommandEvents final : public CashInCommonCommandEvents
    {
    public:
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="storage">Storage service interface</param>
        /// <param name="events">Cash-in events interface</param>
        CashInCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<XFS4IoTFramework::CashAcceptor::ICashInEvents> events)
            : CashInCommonCommandEvents(events)
            , storage_error_command_event_(std::make_unique<XFS4IoTFramework::Storage::StorageErrorCommandEvent>(storage, events))
        {
        }

        /// <summary>
        /// Sub cash-in event - reports intermediate results during cash-in
        /// </summary>
        /// <param name="unrecognized">Number of unrecognized items</param>
        /// <param name="item_counts">Dictionary of item counts by note type</param>
        boost::asio::awaitable<void> SubCashInEvent(
            std::optional<int> unrecognized,
            const std::unordered_map<std::string, Storage::CashItemCountClass>& item_counts)
        {
            if (!cash_in_events_)
            {
                throw std::runtime_error(
                    std::format("No ICashInEvents interface is set. {}", "SubCashInEvent"));
            }

            std::shared_ptr<XFS4IoT::CashAcceptor::Events::SubCashInEventPayloadData> payload;

            if (!item_counts.empty())
            {
                std::unordered_map<std::string, std::shared_ptr<XFS4IoT::CashManagement::StorageCashCountClass>> itemMovementResult;

                for (const auto& [key, item_count] : item_counts)
                {
                    itemMovementResult.emplace(
                        key,
                        std::make_shared<XFS4IoT::CashManagement::StorageCashCountClass>(
                            item_count.GetFit(),
                            item_count.GetUnfit(),
                            item_count.GetSuspect(),
                            item_count.GetCounterfeit(),
                            item_count.GetInked()));
                }

                payload = std::make_shared<XFS4IoT::CashAcceptor::Events::SubCashInEventPayloadData>(unrecognized);
                payload->SetExtendedProperties(std::move(itemMovementResult));
            }
            else if (unrecognized.has_value())
            {
                payload = std::make_shared<XFS4IoT::CashAcceptor::Events::SubCashInEventPayloadData>(unrecognized);
            }

            if (payload)
            {
                co_return co_await cash_in_events_->SubCashInEvent(*payload);
            }

            co_return;
        }

        /// <summary>
        /// Storage error event - reports storage-related errors
        /// </summary>
        /// <param name="failure">Type of failure</param>
        /// <param name="cash_unit_ids">List of affected cash unit IDs</param>
        boost::asio::awaitable<void> StorageErrorEvent(
            XFS4IoTFramework::Storage::FailureEnum failure,
            const std::vector<std::string>& cash_unit_ids)
        {
            if (storage_error_command_event_)
            {
                co_return co_await storage_error_command_event_->StorageErrorEvent(failure, cash_unit_ids);
            }
            co_return;
        }

        virtual ~CashInCommandEvents() = default;

        // Запрещаем копирование, разрешаем перемещение
        CashInCommandEvents(const CashInCommandEvents&) = delete;
        CashInCommandEvents& operator=(const CashInCommandEvents&) = delete;
        CashInCommandEvents(CashInCommandEvents&&) = default;
        CashInCommandEvents& operator=(CashInCommandEvents&&) = default;

    private:
        std::unique_ptr<Storage::StorageErrorCommandEvent> storage_error_command_event_;
    };

} // namespace XFS4IoTFramework::CashAcceptor