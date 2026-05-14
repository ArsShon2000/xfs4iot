#pragma once

#include <optional>
#include <memory>
#include <string>
#include <stdexcept>
#include "../../Events.hpp"
#include "../../Message.hpp"
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>

namespace XFS4IoT::CashAcceptor::Events
{
        class InputRefuseEventPayloadData final : public XFS4IoT::MessagePayloadBase
        {
        public:
            enum class ReasonEnum
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
            /// Constructor
            /// </summary>
            /// <param name="reason">Reason for refusing items</param>
            explicit InputRefuseEventPayloadData(std::optional<ReasonEnum> reason = std::nullopt)
                : reason_(reason)
            {
            }

            /// <summary>
            /// Reason for refusing a part of the amount. The following values are possible:
            /// 
            /// * cashInUnitFull - storage unit is full.
            /// * invalidBill - Recognition of the items took place, but one or more of the items are invalid.
            /// * noBillsToDeposit - There are no items in the input area.
            /// * depositFailure - A deposit has failed for a reason not covered by the other reasons and the
            ///   failure is not a fatal hardware problem, for example failing to pick an item from the input area.
            /// * commonInputComponentFailure - Failure of a common input component which is shared by all storage
            ///   units.
            /// * stackerFull - The intermediate stacker is full.
            /// * foreignItemsDetected - Foreign items have been detected in the input position.
            /// * invalidBunch - Recognition of the items did not take place. The bunch of notes inserted is
            ///   invalid, e.g. it is too large or was inserted incorrectly.
            /// * counterfeit - One or more counterfeit items have been detected and refused. This is only
            ///   applicable where notes are not classified as level 2 and the device is capable of differentiating
            ///   between invalid and counterfeit items.
            /// * limitOverTotalItems - Number of items inserted exceeded the limitation set with the
            ///   CashAcceptor.CashInStart command.
            /// * limitOverAmount - Amount exceeded the limitation set with the CashAcceptor.CashInStart command.
            /// </summary>
            std::optional<ReasonEnum> GetReason() const { return reason_; }
            void SetReason(std::optional<ReasonEnum> reason) { reason_ = reason; }

            virtual ~InputRefuseEventPayloadData() = default;

        private:
            std::optional<ReasonEnum> reason_;
        };



        inline std::string ToString(InputRefuseEventPayloadData::ReasonEnum reason)
        {
            switch (reason)
            {
            case InputRefuseEventPayloadData::ReasonEnum::CashInUnitFull:
                return "cashInUnitFull";
            case InputRefuseEventPayloadData::ReasonEnum::InvalidBill:
                return "invalidBill";
            case InputRefuseEventPayloadData::ReasonEnum::NoBillsToDeposit:
                return "noBillsToDeposit";
            case InputRefuseEventPayloadData::ReasonEnum::DepositFailure:
                return "depositFailure";
            case InputRefuseEventPayloadData::ReasonEnum::CommonInputComponentFailure:
                return "commonInputComponentFailure";
            case InputRefuseEventPayloadData::ReasonEnum::StackerFull:
                return "stackerFull";
            case InputRefuseEventPayloadData::ReasonEnum::ForeignItemsDetected:
                return "foreignItemsDetected";
            case InputRefuseEventPayloadData::ReasonEnum::InvalidBunch:
                return "invalidBunch";
            case InputRefuseEventPayloadData::ReasonEnum::Counterfeit:
                return "counterfeit";
            case InputRefuseEventPayloadData::ReasonEnum::LimitOverTotalItems:
                return "limitOverTotalItems";
            case InputRefuseEventPayloadData::ReasonEnum::LimitOverAmount:
                return "limitOverAmount";
            default:
                throw std::invalid_argument("Unknown ReasonEnum value");
            }
        }

        inline void to_json(
            nlohmann::json& j,
            const InputRefuseEventPayloadData& p)
        {
            j = nlohmann::json::object();

            if (p.GetReason().has_value())
                j["reason"] = ToString(p.GetReason().value());
        }

    /// <summary>
    /// InputRefuseEvent - XFS4 Version 2.0
    /// Event Name: CashAcceptor.InputRefuseEvent
    /// Items were refused during cash-in operation
    /// </summary>
    class InputRefuseEvent final : public XFS4IoT::Events::Event<InputRefuseEventPayloadData>
    {
    public:
        /// <summary>
        /// InputRefuseEvent constructor
        /// </summary>
        /// <param name="request_id">Request id of the command this event relates to</param>
        /// <param name="payload">Event payload data</param>
       /* explicit InputRefuseEvent(int request_id, std::shared_ptr<InputRefuseEventPayloadData> payload)
            : Event<InputRefuseEventPayloadData>(request_id, payload)
        {
        }*/

        /// <summary>
        /// InputRefuseEvent constructor with unique_ptr
        /// </summary>
        InputRefuseEvent(
            int requestId,
            std::shared_ptr<InputRefuseEventPayloadData> payload)
            : XFS4IoT::Events::Event<InputRefuseEventPayloadData>(
                EventName,
                Version,
                requestId,
                std::move(payload))
        {
        }

        virtual ~InputRefuseEvent() = default;


        // Метаданные для регистрации
        static constexpr const char* EventName = "CashAcceptor.InputRefuseEvent";
        static constexpr const char* Version = "1.0";

    private:
         //Статическая регистрация в реестре сообщений
        static bool registered_;
    };

    inline bool InputRefuseEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(InputRefuseEvent),
                InputRefuseEvent::EventName,
                InputRefuseEvent::Version);
            return true;
        }();



} // namespace XFS4IoT::CashAcceptor::Events