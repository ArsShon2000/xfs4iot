#pragma once

#include <memory>
#include <optional>
#include <string>

#include "../../Events.hpp"
#include "../../Message.hpp"
#include "../../common/MessageHeader.hpp"
#include "../CashManagementSchemas.hpp"

namespace XFS4IoT::CashManagement::Events
{
    class ItemsInsertedEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        explicit ItemsInsertedEventPayloadData(
            std::optional<PositionEnum> position = std::nullopt)
            : position_(position)
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

    private:
        std::optional<PositionEnum> position_;
    };

    // ---- enum -> string ----

    inline void to_json(nlohmann::json& j, const PositionEnum& pos)
    {
        j = ToString(pos);
    }

    // ---- payload serialization ----
    inline void to_json(
        nlohmann::json& j,
        const ItemsInsertedEventPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetPosition().has_value())
            j["position"] = p.GetPosition().value();
    }

    class ItemsInsertedEvent final
        : public XFS4IoT::Events::UnsolicitedEvent<ItemsInsertedEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "CashManagement.ItemsInsertedEvent";
        static constexpr const char* Version = "1.0";

        explicit ItemsInsertedEvent(
            std::shared_ptr<ItemsInsertedEventPayloadData> payload)
            : XFS4IoT::Events::UnsolicitedEvent<ItemsInsertedEventPayloadData>(
                EventName,
                Version,
                std::move(payload))
        {
        }

    private:
        static bool registered_;
    };

    inline bool ItemsInsertedEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(ItemsInsertedEvent),
                ItemsInsertedEvent::EventName,
                ItemsInsertedEvent::Version);
            return true;
        }();
}