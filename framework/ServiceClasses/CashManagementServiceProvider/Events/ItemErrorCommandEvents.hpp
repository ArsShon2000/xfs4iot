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
#include "ItemInfoAvailableCommandEvent.hpp"
#include "../../../core/CashManagement/Events/NoteErrorEvent.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashInEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/IReplenishEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashInEndEvents.hpp"

namespace XFS4IoTFramework::CashManagement
{
	// Класс ItemErrorCommandEvents
	class ItemErrorCommandEvents : public ItemInfoAvailableCommandEvent
	{
	public:
		enum class ItemErrorReasonEnum
		{
			DoubleNote,
			LongNote,
			SkewedNote,
			IncorrectCount,
			NotesTooClose,
			OtherNoteError,
			ShortNote
		};


		explicit ItemErrorCommandEvents(std::shared_ptr<ICalibrateCashUnitEvents> events)
			: ItemInfoAvailableCommandEvent(events)
		{
		}

		explicit ItemErrorCommandEvents(std::shared_ptr<IRetractEvents> events)
			: ItemInfoAvailableCommandEvent(events)
		{
		}

		explicit ItemErrorCommandEvents(std::shared_ptr<XFS4IoTFramework::CashAcceptor::ICashInEvents> events)
			: ItemInfoAvailableCommandEvent(events)
		{
		}

		explicit ItemErrorCommandEvents(std::shared_ptr<XFS4IoTFramework::CashAcceptor::ICashInEndEvents> events)
			: ItemInfoAvailableCommandEvent(events)
		{
		}

		explicit ItemErrorCommandEvents(std::shared_ptr<ICashUnitCountEvents> events)
			: ItemInfoAvailableCommandEvent(events)
		{
		}

		explicit ItemErrorCommandEvents(std::shared_ptr<XFS4IoTFramework::CashAcceptor::IReplenishEvents> events)
			: ItemInfoAvailableCommandEvent(events)
		{
		}

		boost::asio::awaitable<void> NoteErrorEvent(ItemErrorReasonEnum reason)
		{
			auto payload_reason = [reason]() -> std::optional<XFS4IoT::CashManagement::Events::NoteErrorEventPayloadData::ReasonEnum> {
				switch (reason)
				{
				case ItemErrorReasonEnum::DoubleNote:
					return XFS4IoT::CashManagement::Events::NoteErrorEventPayloadData::ReasonEnum::DoubleNote;
				case ItemErrorReasonEnum::IncorrectCount:
					return XFS4IoT::CashManagement::Events::NoteErrorEventPayloadData::ReasonEnum::IncorrectCount;
				case ItemErrorReasonEnum::LongNote:
					return XFS4IoT::CashManagement::Events::NoteErrorEventPayloadData::ReasonEnum::LongNote;
				case ItemErrorReasonEnum::NotesTooClose:
					return XFS4IoT::CashManagement::Events::NoteErrorEventPayloadData::ReasonEnum::NotesTooClose;
				case ItemErrorReasonEnum::OtherNoteError:
					return XFS4IoT::CashManagement::Events::NoteErrorEventPayloadData::ReasonEnum::OtherNoteError;
				case ItemErrorReasonEnum::ShortNote:
					return XFS4IoT::CashManagement::Events::NoteErrorEventPayloadData::ReasonEnum::ShortNote;
				case ItemErrorReasonEnum::SkewedNote:
					return XFS4IoT::CashManagement::Events::NoteErrorEventPayloadData::ReasonEnum::SkewedNote;
				default:
					return std::nullopt;
				}
				}();

			XFS4IoT::CashManagement::Events::NoteErrorEventPayloadData payload{ payload_reason };

			if (calibrate_cash_unit_events_)
			{
				return calibrate_cash_unit_events_->NoteErrorEvent(payload);
			}
			if (retract_events_)
			{
				return retract_events_->NoteErrorEvent(payload);
			}
			if (cash_in_events_)
			{
				return cash_in_events_->NoteErrorEvent(payload);
			}
			if (cash_unit_count_events_)
			{
				return cash_unit_count_events_->NoteErrorEvent(payload);
			}
			if (replenish_events_)
			{
				return replenish_events_->NoteErrorEvent(payload);
			}

			throw std::runtime_error(std::format("Unreachable code. {}", "NoteErrorEvent"));
		}
	};

} // namespace XFS4IoTFramework::CashManagement