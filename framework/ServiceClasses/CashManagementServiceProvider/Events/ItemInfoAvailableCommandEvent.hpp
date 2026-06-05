#pragma once

#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <format>
#include <boost/asio/awaitable.hpp>
#include <concepts>
#include <type_traits>

#include "../../../ServiceClasses/CashManagementServiceProvider/DeviceParameters.hpp"
#include "../../ServiceInterfaces/CashManagement/IRetractEvents.hpp"
#include "../../ServiceInterfaces/CashManagement/IResetEvents.hpp"
#include "../../ServiceInterfaces/CashManagement/ICalibrateCashUnitEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashInEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashInEndEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashInRollbackEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/IPreparePresentEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashUnitCountEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/IReplenishEvents.hpp"
#include "../../../core/CashManagement/Events/InfoAvailableEvent.hpp"
#include "../../ServiceInterfaces/CashDispanser/IRejectEvents.hpp"

namespace XFS4IoTFramework::CashManagement
{
    // Вспомогательные функции проверки (аналог extension methods из C#)
    template<typename T>
    void IsNotNull(const std::shared_ptr<T>& ptr, const std::string& message)
    {
        if (!ptr) {
            throw std::invalid_argument(message);
        }
    }

    template<typename TInterface, typename T>
    void IsA(const std::shared_ptr<T>& ptr, const std::string& message)
    {
        if (!std::dynamic_pointer_cast<TInterface>(ptr)) {
            throw std::invalid_argument(message);
        }
    }

    // Базовый класс ItemInfoAvailableCommandEvent
    class ItemInfoAvailableCommandEvent
    {
    public:
        // Конструкторы для различных интерфейсов событий
        explicit ItemInfoAvailableCommandEvent(std::shared_ptr<IRetractEvents> events)
            : retract_events_(events)
        {
            IsNotNull(events, std::format("Invalid parameter passed in. {}", "ItemInfoAvailableCommandEvent"));
            IsA<IRetractEvents>(events, std::format("Invalid interface passed in. {}", "ItemInfoAvailableCommandEvent"));
        }

        explicit ItemInfoAvailableCommandEvent(std::shared_ptr<IResetEvents> events)
            : reset_events_(events)
        {
            IsNotNull(events, std::format("Invalid parameter passed in. {}", "ItemInfoAvailableCommandEvent"));
            IsA<IResetEvents>(events, std::format("Invalid interface passed in. {}", "ItemInfoAvailableCommandEvent"));
        }

        explicit ItemInfoAvailableCommandEvent(std::shared_ptr<ICalibrateCashUnitEvents> events)
            : calibrate_cash_unit_events_(events)
        {
            IsNotNull(events, std::format("Invalid parameter passed in. {}", "ItemInfoAvailableCommandEvent"));
            IsA<ICalibrateCashUnitEvents>(events, std::format("Invalid interface passed in. {}", "ItemInfoAvailableCommandEvent"));
        }

        explicit ItemInfoAvailableCommandEvent(std::shared_ptr<XFS4IoTFramework::CashAcceptor::ICashInEvents> events)
            : cash_in_events_(events)
        {
            IsNotNull(events, std::format("Invalid parameter passed in. {}", "ItemInfoAvailableCommandEvent"));
            IsA<XFS4IoTFramework::CashAcceptor::ICashInEvents>(events, std::format("Invalid interface passed in. {}", "ItemInfoAvailableCommandEvent"));
        }

        explicit ItemInfoAvailableCommandEvent(std::shared_ptr<XFS4IoTFramework::CashAcceptor::ICashInEndEvents> events)
            : cash_in_end_events_(events)
        {
            IsNotNull(events, std::format("Invalid parameter passed in. {}", "ItemInfoAvailableCommandEvent"));
            IsA<XFS4IoTFramework::CashAcceptor::ICashInEndEvents>(events, std::format("Invalid interface passed in. {}", "ItemInfoAvailableCommandEvent"));
        }

        explicit ItemInfoAvailableCommandEvent(std::shared_ptr<ICashInRollbackEvents> events)
            : cash_in_rollback_events_(events)
        {
            IsNotNull(events, std::format("Invalid parameter passed in. {}", "ItemInfoAvailableCommandEvent"));
            IsA<ICashInRollbackEvents>(events, std::format("Invalid interface passed in. {}", "ItemInfoAvailableCommandEvent"));
        }

        explicit ItemInfoAvailableCommandEvent(std::shared_ptr<IPreparePresentEvents> events)
            : prepare_present_events_(events)
        {
            IsNotNull(events, std::format("Invalid parameter passed in. {}", "ItemInfoAvailableCommandEvent"));
            IsA<IPreparePresentEvents>(events, std::format("Invalid interface passed in. {}", "ItemInfoAvailableCommandEvent"));
        }

        explicit ItemInfoAvailableCommandEvent(std::shared_ptr<ICashUnitCountEvents> events)
            : cash_unit_count_events_(events)
        {
            IsNotNull(events, std::format("Invalid parameter passed in. {}", "ItemInfoAvailableCommandEvent"));
            IsA<ICashUnitCountEvents>(events, std::format("Invalid interface passed in. {}", "ItemInfoAvailableCommandEvent"));
        }

        explicit ItemInfoAvailableCommandEvent(std::shared_ptr<XFS4IoTFramework::CashAcceptor::IReplenishEvents> events)
            : replenish_events_(events)
        {
            IsNotNull(events, std::format("Invalid parameter passed in. {}", "ItemInfoAvailableCommandEvent"));
            IsA<XFS4IoTFramework::CashAcceptor::IReplenishEvents>(events, std::format("Invalid interface passed in. {}", "ItemInfoAvailableCommandEvent"));
        }

        explicit ItemInfoAvailableCommandEvent(std::shared_ptr<XFS4IoTFramework::CashDispenser::IRejectEvents> events)
            : reject_events_(events)
        {
            IsNotNull(events, std::format("Invalid parameter passed in. {}", "ItemInfoAvailableCommandEvent"));
            IsA<XFS4IoTFramework::CashDispenser::IRejectEvents>(events, std::format("Invalid interface passed in. {}", "ItemInfoAvailableCommandEvent"));
        }

        virtual ~ItemInfoAvailableCommandEvent() = default;

        // Метод InfoAvailableEvent
        boost::asio::awaitable<void> InfoAvailableEvent(const std::vector<ItemInfoSummary>& item_info_summary)
        {
            std::vector<XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData::ItemInfoSummaryClass> item_info;

            for (const auto& item : item_info_summary)
            {
                auto note_level = [&item]() -> XFS4IoT::CashManagement::NoteLevelEnum {
                    switch (item.level())
                    {
                    case NoteLevelEnum::Counterfeit:
                        return XFS4IoT::CashManagement::NoteLevelEnum::Counterfeit;
                    case NoteLevelEnum::Fit:
                        return XFS4IoT::CashManagement::NoteLevelEnum::Fit;
                    case NoteLevelEnum::Inked:
                        return XFS4IoT::CashManagement::NoteLevelEnum::Inked;
                    case NoteLevelEnum::Suspect:
                        return XFS4IoT::CashManagement::NoteLevelEnum::Suspect;
                    case NoteLevelEnum::Unfit:
                        return XFS4IoT::CashManagement::NoteLevelEnum::Unfit;
                    default:
                        return XFS4IoT::CashManagement::NoteLevelEnum::Unrecognized;
                    }
                    }();

                item_info.emplace_back(note_level, item.num_of_items());
            }

            if (retract_events_)
            {
                return retract_events_->InfoAvailableEvent(XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData{ item_info });
            }
            if (reset_events_)
            {
                return reset_events_->InfoAvailableEvent(std::make_shared<XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData>(item_info));
            }
            if (calibrate_cash_unit_events_)
            {
                return calibrate_cash_unit_events_->InfoAvailableEvent(XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData{ item_info });
            }
            if (cash_in_events_)
            {
                return cash_in_events_->InfoAvailableEvent(XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData{ item_info });
            }
            if (cash_in_end_events_)
            {
                return cash_in_end_events_->InfoAvailableEvent(std::make_shared<XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData>(item_info));
            }
            if (cash_in_rollback_events_)
            {
                return cash_in_rollback_events_->InfoAvailableEvent(XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData{ item_info });
            }
            if (prepare_present_events_)
            {
                return prepare_present_events_->InfoAvailableEvent(XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData{ item_info });
            }
            if (cash_unit_count_events_)
            {
                return cash_unit_count_events_->InfoAvailableEvent(XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData{ item_info });
            }
            if (replenish_events_)
            {
                return replenish_events_->InfoAvailableEvent(XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData{ item_info });
            }
            if (reject_events_)
            {
                return reject_events_->InfoAvailableEvent(XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData{ item_info });
            }

            throw std::runtime_error(std::format("Unreachable code. {}", "InfoAvailableEvent"));
        }

    protected:
        // CashManagement
        std::shared_ptr<IRetractEvents> retract_events_ = nullptr;
        std::shared_ptr<IResetEvents> reset_events_ = nullptr;
        std::shared_ptr<ICalibrateCashUnitEvents> calibrate_cash_unit_events_ = nullptr;

        // CashAcceptor
        std::shared_ptr<XFS4IoTFramework::CashAcceptor::ICashInEvents> cash_in_events_ = nullptr;
        std::shared_ptr<XFS4IoTFramework::CashAcceptor::ICashInEndEvents> cash_in_end_events_ = nullptr;
        std::shared_ptr<ICashInRollbackEvents> cash_in_rollback_events_ = nullptr;
        std::shared_ptr<IPreparePresentEvents> prepare_present_events_ = nullptr;
        std::shared_ptr<ICashUnitCountEvents> cash_unit_count_events_ = nullptr;
        std::shared_ptr<XFS4IoTFramework::CashDispenser::IRejectEvents> reject_events_ = nullptr;
        std::shared_ptr<XFS4IoTFramework::CashAcceptor::IReplenishEvents> replenish_events_ = nullptr;
    };

} // namespace XFS4IoTFramework::CashManagement