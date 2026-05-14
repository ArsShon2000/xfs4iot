#pragma once

#include <memory>
#include <stdexcept>
#include <format>
#include <boost/asio/awaitable.hpp>
#include "../../CashManagementServiceProvider/Events/ItemErrorCommandEvents.hpp"
#include "../../../core/CashAcceptor/Events/InputRefuseEvent.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashInEvents.hpp"

namespace XFS4IoTFramework::CashAcceptor
{
    /// <summary>
    /// Base class for common cash-in command events
    /// </summary>
    class CashInCommonCommandEvents : public XFS4IoTFramework::CashManagement::ItemErrorCommandEvents
    {
    public:
        enum class RefusedReasonEnum
        {
            CashInUnitFull,
            InvalidBill,
            NoBillsToDeposit,
            DepositFailure,
            CommonInputComponentFailure,
            StackerFull,
            ForeignItemsDetected,
            InvalidBunch,
            Counterfeit,
            LimitOverTotalItems,
            LimitOverAmount
        };

        /// <summary>
        /// Constructor with ICashInEvents
        /// </summary>
        explicit CashInCommonCommandEvents(std::shared_ptr<XFS4IoTFramework::CashAcceptor::ICashInEvents> events)
            : ItemErrorCommandEvents(events)
        {
        }


        virtual ~CashInCommonCommandEvents() = default;

        /// <summary>
        /// Insert items event - prompts user to insert items
        /// </summary>
        boost::asio::awaitable<void> InsertItemsEvent()
        {
            if (cash_in_events_)
            {
                co_return co_await cash_in_events_->InsertItemsEvent();
            }

            throw std::runtime_error(std::format("Unreachable code. {}", "InsertItemsEvent"));
        }

        /// <summary>
        /// Input refuse event - items were refused during cash-in
        /// </summary>
        boost::asio::awaitable<void> InputRefuseEvent(RefusedReasonEnum reason)
        {
            auto payload_reason = [reason]() -> XFS4IoT::CashAcceptor::Events::InputRefuseEventPayloadData::ReasonEnum
                {
                    switch (reason)
                    {
                    case RefusedReasonEnum::CashInUnitFull:
                        return XFS4IoT::CashAcceptor::Events::InputRefuseEventPayloadData::ReasonEnum::CashInUnitFull;
                    case RefusedReasonEnum::InvalidBill:
                        return XFS4IoT::CashAcceptor::Events::InputRefuseEventPayloadData::ReasonEnum::InvalidBill;
                    case RefusedReasonEnum::NoBillsToDeposit:
                        return XFS4IoT::CashAcceptor::Events::InputRefuseEventPayloadData::ReasonEnum::NoBillsToDeposit;
                    case RefusedReasonEnum::DepositFailure:
                        return XFS4IoT::CashAcceptor::Events::InputRefuseEventPayloadData::ReasonEnum::DepositFailure;
                    case RefusedReasonEnum::CommonInputComponentFailure:
                        return XFS4IoT::CashAcceptor::Events::InputRefuseEventPayloadData::ReasonEnum::CommonInputComponentFailure;
                    case RefusedReasonEnum::StackerFull:
                        return XFS4IoT::CashAcceptor::Events::InputRefuseEventPayloadData::ReasonEnum::StackerFull;
                    case RefusedReasonEnum::ForeignItemsDetected:
                        return XFS4IoT::CashAcceptor::Events::InputRefuseEventPayloadData::ReasonEnum::ForeignItemsDetected;
                    case RefusedReasonEnum::InvalidBunch:
                        return XFS4IoT::CashAcceptor::Events::InputRefuseEventPayloadData::ReasonEnum::InvalidBunch;
                    case RefusedReasonEnum::Counterfeit:
                        return XFS4IoT::CashAcceptor::Events::InputRefuseEventPayloadData::ReasonEnum::Counterfeit;
                    case RefusedReasonEnum::LimitOverTotalItems:
                        return XFS4IoT::CashAcceptor::Events::InputRefuseEventPayloadData::ReasonEnum::LimitOverTotalItems;
                    case RefusedReasonEnum::LimitOverAmount:
                    default:
                        return XFS4IoT::CashAcceptor::Events::InputRefuseEventPayloadData::ReasonEnum::LimitOverAmount;
                    }
                }();

            XFS4IoT::CashAcceptor::Events::InputRefuseEventPayloadData payload(payload_reason);

            if (cash_in_events_)
            {
                co_return co_await cash_in_events_->InputRefuseEvent(payload);
            }

            throw std::runtime_error(std::format("Unreachable code. {}", "InputRefuseEvent"));
        }

    protected:
        using ItemErrorCommandEvents::cash_in_events_;
    };

} // namespace XFS4IoTFramework::CashAcceptor