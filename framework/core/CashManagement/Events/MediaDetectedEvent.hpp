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
    class MediaDetectedEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        explicit MediaDetectedEventPayloadData(
            std::optional<ItemTargetEnumEnum> target = std::nullopt,
            std::optional<std::string> unit = std::nullopt,
            std::optional<int> index = std::nullopt)
            : target_(target)
            , unit_(std::move(unit))
            , index_(index)
        {
        }

        std::optional<ItemTargetEnumEnum> GetTarget() const noexcept
        {
            return target_;
        }

        void SetTarget(std::optional<ItemTargetEnumEnum> target)
        {
            target_ = target;
        }

        const std::optional<std::string>& GetUnit() const noexcept
        {
            return unit_;
        }

        void SetUnit(std::optional<std::string> unit)
        {
            unit_ = std::move(unit);
        }

        std::optional<int> GetIndex() const noexcept
        {
            return index_;
        }

        void SetIndex(std::optional<int> index)
        {
            index_ = index;
        }

        bool ValidateUnit() const
        {
            if (!unit_.has_value())
                return true;

            static const std::regex pattern(R"(^unit[0-9A-Za-z]+$)");
            return std::regex_match(unit_.value(), pattern);
        }

        bool ValidateIndex() const
        {
            return !index_.has_value() || index_.value() >= 1;
        }

    private:
        std::optional<ItemTargetEnumEnum> target_;
        std::optional<std::string> unit_;
        std::optional<int> index_;
    };

    inline void to_json(
        nlohmann::json& j,
        const MediaDetectedEventPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetTarget().has_value())
            j["target"] = toString(p.GetTarget().value());

        if (p.GetUnit().has_value())
            j["unit"] = p.GetUnit().value();

        if (p.GetIndex().has_value())
            j["index"] = p.GetIndex().value();
    }

    class MediaDetectedEvent final
        : public XFS4IoT::Events::UnsolicitedEvent<MediaDetectedEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "CashManagement.MediaDetectedEvent";
        static constexpr const char* Version = "2.0";

        explicit MediaDetectedEvent(
            std::shared_ptr<MediaDetectedEventPayloadData> payload)
            : XFS4IoT::Events::UnsolicitedEvent<MediaDetectedEventPayloadData>(
                EventName,
                Version,
                std::move(payload))
        {
        }

    private:
        static bool registered_;
    };

    inline bool MediaDetectedEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(MediaDetectedEvent),
                MediaDetectedEvent::EventName,
                MediaDetectedEvent::Version);
            return true;
        }();
}