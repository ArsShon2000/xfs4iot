#pragma once

#include <memory>
#include <optional>
#include <string>

#include "../../Events.hpp"
#include "../../Message.hpp"
#include "../../common/MessageHeader.hpp"

namespace XFS4IoT::CashDispenser::Events
{
    class DelayedDispenseEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        explicit DelayedDispenseEventPayloadData(
            std::optional<double> delay = std::nullopt)
            : delay_(delay)
        {
        }

        std::optional<double> GetDelay() const { return delay_; }
        void SetDelay(std::optional<double> delay) { delay_ = delay; }

    private:
        std::optional<double> delay_;
    };

    inline void to_json(
        nlohmann::json& j,
        const DelayedDispenseEventPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetDelay().has_value())
            j["delay"] = p.GetDelay().value();
    }

    class DelayedDispenseEvent final
        : public XFS4IoT::Events::Event<DelayedDispenseEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "CashDispenser.DelayedDispenseEvent";
        static constexpr const char* Version = "1.0";

        DelayedDispenseEvent(
            int requestId,
            std::shared_ptr<DelayedDispenseEventPayloadData> payload)
            : XFS4IoT::Events::Event<DelayedDispenseEventPayloadData>(
                EventName,
                Version,
                requestId,
                std::move(payload))
        {
        }

    private:
        static bool registered_;
    };

    inline bool DelayedDispenseEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(DelayedDispenseEvent),
                DelayedDispenseEvent::EventName,
                DelayedDispenseEvent::Version);
            return true;
        }();
}