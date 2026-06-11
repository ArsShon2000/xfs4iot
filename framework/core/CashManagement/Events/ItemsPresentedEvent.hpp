#pragma once

#include <memory>
#include <optional>
#include <string>
#include <regex>

#include "../../Events.hpp"
#include "../../Message.hpp"
#include "../../common/MessageHeader.hpp"
#include "../CashManagementSchemas.hpp"

namespace XFS4IoT::CashManagement::Events
{
    class ItemsPresentedEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        explicit ItemsPresentedEventPayloadData(
            std::optional<PositionEnum> position = std::nullopt,
            std::optional<std::string> additionalBunches = std::nullopt)
            : position_(position)
            , additionalBunches_(std::move(additionalBunches))
        {
        }

        std::optional<PositionEnum> GetPosition() const noexcept
        {
            return position_;
        }

        void SetPosition(std::optional<PositionEnum> position)
        {
            position_ = position;
        }

        const std::optional<std::string>& GetAdditionalBunches() const noexcept
        {
            return additionalBunches_;
        }

        void SetAdditionalBunches(std::optional<std::string> additionalBunches)
        {
            additionalBunches_ = std::move(additionalBunches);
        }

        bool ValidateAdditionalBunches() const
        {
            if (!additionalBunches_.has_value())
                return true;

            static const std::regex pattern(R"(^unknown$|^[0-9]*$)");
            return std::regex_match(additionalBunches_.value(), pattern);
        }

    private:
        std::optional<PositionEnum> position_;
        std::optional<std::string> additionalBunches_;
    };

    inline void to_json(
        nlohmann::json& j,
        const ItemsPresentedEventPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetPosition().has_value())
            j["position"] = ToString(p.GetPosition().value());

        if (p.GetAdditionalBunches().has_value())
            j["additionalBunches"] = p.GetAdditionalBunches().value();
    }

    class ItemsPresentedEvent final
        : public XFS4IoT::Events::UnsolicitedEvent<ItemsPresentedEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "CashManagement.ItemsPresentedEvent";
        static constexpr const char* Version = "2.0";

        explicit ItemsPresentedEvent(
            std::shared_ptr<ItemsPresentedEventPayloadData> payload)
            : XFS4IoT::Events::UnsolicitedEvent<ItemsPresentedEventPayloadData>(
                EventName,
                Version,
                std::move(payload))
        {
        }

    private:
        static bool registered_;
    };

    inline bool ItemsPresentedEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(ItemsPresentedEvent),
                ItemsPresentedEvent::EventName,
                ItemsPresentedEvent::Version);
            return true;
        }();
}
