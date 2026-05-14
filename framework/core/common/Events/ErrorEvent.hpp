#pragma once

#include <memory>
#include <optional>
#include <string>
#include <stdexcept>

#include "../../Events.hpp"
#include "../../Message.hpp"
#include "../../common/MessageHeader.hpp"

namespace XFS4IoT::Common::Events
{
    class ErrorEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        enum class EventIdEnum
        {
            Hardware,
            Software,
            User,
            FraudAttempt
        };

        enum class ActionEnum
        {
            Reset,
            SoftwareError,
            Configuration,
            Clear,
            Maintenance,
            Suspend
        };

        ErrorEventPayloadData(
            std::optional<EventIdEnum> eventId = std::nullopt,
            std::optional<ActionEnum> action = std::nullopt,
            std::optional<std::string> vendorDescription = std::nullopt)
            : eventId_(eventId)
            , action_(action)
            , vendorDescription_(std::move(vendorDescription))
        {
        }

        const std::optional<EventIdEnum>& GetEventId() const noexcept
        {
            return eventId_;
        }

        void SetEventId(std::optional<EventIdEnum> eventId)
        {
            eventId_ = eventId;
        }

        const std::optional<ActionEnum>& GetAction() const noexcept
        {
            return action_;
        }

        void SetAction(std::optional<ActionEnum> action)
        {
            action_ = action;
        }

        const std::optional<std::string>& GetVendorDescription() const noexcept
        {
            return vendorDescription_;
        }

        void SetVendorDescription(std::optional<std::string> vendorDescription)
        {
            vendorDescription_ = std::move(vendorDescription);
        }

    private:
        std::optional<EventIdEnum> eventId_;
        std::optional<ActionEnum> action_;
        std::optional<std::string> vendorDescription_;
    };

    inline std::string toString(const ErrorEventPayloadData::EventIdEnum value)
    {
        switch (value)
        {
        case ErrorEventPayloadData::EventIdEnum::Hardware:
            return "hardware";
        case ErrorEventPayloadData::EventIdEnum::Software:
            return "software";
        case ErrorEventPayloadData::EventIdEnum::User:
            return "user";
        case ErrorEventPayloadData::EventIdEnum::FraudAttempt:
            return "fraudAttempt";
        }

        throw std::invalid_argument("Unknown ErrorEventPayloadData::EventIdEnum value");
    }

    inline void to_json(nlohmann::json& j, const ErrorEventPayloadData::EventIdEnum& value)
    {
        j = toString(value);
    }

    inline std::string toString(const ErrorEventPayloadData::ActionEnum value)
    {
        switch (value)
        {
        case ErrorEventPayloadData::ActionEnum::Reset:
            return "reset";
        case ErrorEventPayloadData::ActionEnum::SoftwareError:
            return "softwareError";
        case ErrorEventPayloadData::ActionEnum::Configuration:
            return "configuration";
        case ErrorEventPayloadData::ActionEnum::Clear:
            return "clear";
        case ErrorEventPayloadData::ActionEnum::Maintenance:
            return "maintenance";
        case ErrorEventPayloadData::ActionEnum::Suspend:
            return "suspend";
        }

        throw std::invalid_argument("Unknown ErrorEventPayloadData::ActionEnum value");
    }

    inline void to_json(nlohmann::json& j, const ErrorEventPayloadData::ActionEnum& value)
    {
        j = toString(value);
    }

    inline void to_json(
        nlohmann::json& j,
        const ErrorEventPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetEventId().has_value())
        {
            j["eventId"] = p.GetEventId().value();
        }

        if (p.GetAction().has_value())
        {
            j["action"] = p.GetAction().value();
        }

        if (p.GetVendorDescription().has_value())
        {
            j["vendorDescription"] = p.GetVendorDescription().value();
        }
    }

    class ErrorEvent final
        : public XFS4IoT::Events::UnsolicitedEvent<ErrorEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "Common.ErrorEvent";
        static constexpr const char* Version = "1.0";

        explicit ErrorEvent(
            std::shared_ptr<ErrorEventPayloadData> payload)
            : XFS4IoT::Events::UnsolicitedEvent<ErrorEventPayloadData>(
                EventName,
                Version,
                std::move(payload))
        {
        }

    private:
        static bool registered_;
    };

    inline bool ErrorEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(ErrorEvent),
                ErrorEvent::EventName,
                ErrorEvent::Version);
            return true;
        }();
}