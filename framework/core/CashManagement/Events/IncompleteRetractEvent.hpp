#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "../../Events.hpp"
#include "../../Message.hpp"
#include "../../common/MessageHeader.hpp"
#include "../CashManagementSchemas.hpp"

namespace XFS4IoT::CashManagement::Events
{
    class IncompleteRetractEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        enum class ReasonEnum
        {
            RetractFailure,
            RetractAreaFull,
            ForeignItemsDetected,
            InvalidBunch
        };

        IncompleteRetractEventPayloadData(
            std::optional<std::unordered_map<std::string, StorageCashInClass>> itemNumberList = std::nullopt,
            std::optional<ReasonEnum> reason = std::nullopt)
            : itemNumberList_(std::move(itemNumberList))
            , reason_(reason)
        {
        }

        const std::optional<std::unordered_map<std::string, StorageCashInClass>>&
            GetItemNumberList() const noexcept
        {
            return itemNumberList_;
        }

        void SetItemNumberList(
            std::optional<std::unordered_map<std::string, StorageCashInClass>> itemNumberList)
        {
            itemNumberList_ = std::move(itemNumberList);
        }

        std::optional<ReasonEnum> GetReason() const noexcept
        {
            return reason_;
        }

        void SetReason(std::optional<ReasonEnum> reason)
        {
            reason_ = reason;
        }

    private:
        std::optional<std::unordered_map<std::string, StorageCashInClass>> itemNumberList_;
        std::optional<ReasonEnum> reason_;
    };

    inline std::string ToString(IncompleteRetractEventPayloadData::ReasonEnum reason)
    {
        switch (reason)
        {
        case IncompleteRetractEventPayloadData::ReasonEnum::RetractFailure:
            return "retractFailure";
        case IncompleteRetractEventPayloadData::ReasonEnum::RetractAreaFull:
            return "retractAreaFull";
        case IncompleteRetractEventPayloadData::ReasonEnum::ForeignItemsDetected:
            return "foreignItemsDetected";
        case IncompleteRetractEventPayloadData::ReasonEnum::InvalidBunch:
            return "invalidBunch";
        default:
            throw std::invalid_argument("Unknown ReasonEnum value");
        }
    }

    inline void to_json(
        nlohmann::json& j,
        const IncompleteRetractEventPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetItemNumberList().has_value())
        {
            j["itemNumberList"] = nlohmann::json::object();

            for (const auto& [key, value] : p.GetItemNumberList().value())
            {
                j["itemNumberList"][key] = value;
            }
        }

        if (p.GetReason().has_value())
            j["reason"] = ToString(p.GetReason().value());
    }

    class IncompleteRetractEvent final
        : public XFS4IoT::Events::Event<IncompleteRetractEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "CashManagement.IncompleteRetractEvent";
        static constexpr const char* Version = "1.0";

        IncompleteRetractEvent(
            int requestId,
            std::shared_ptr<IncompleteRetractEventPayloadData> payload)
            : XFS4IoT::Events::Event<IncompleteRetractEventPayloadData>(
                EventName,
                Version,
                requestId,
                std::move(payload))
        {
        }

    private:
        static bool registered_;
    };

    inline bool IncompleteRetractEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(IncompleteRetractEvent),
                IncompleteRetractEvent::EventName,
                IncompleteRetractEvent::Version);
            return true;
        }();
}