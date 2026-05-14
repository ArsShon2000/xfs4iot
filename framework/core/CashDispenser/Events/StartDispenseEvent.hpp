#pragma once

#include <memory>
#include <string>

#include "../../Events.hpp"
#include "../../Message.hpp"
#include "../../common/MessageHeader.hpp"

namespace XFS4IoT::CashDispenser::Events
{
    class StartDispenseEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        StartDispenseEventPayloadData() = default;
    };

    inline void to_json(nlohmann::json& j, const StartDispenseEventPayloadData&)
    {
        j = nlohmann::json::object();
    }

    class StartDispenseEvent final
        : public XFS4IoT::Events::Event<StartDispenseEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "CashDispenser.StartDispenseEvent";
        static constexpr const char* Version = "1.0";

        explicit StartDispenseEvent(int requestId)
            : XFS4IoT::Events::Event<StartDispenseEventPayloadData>(
                EventName,
                Version,
                requestId,
                std::make_shared<StartDispenseEventPayloadData>())
        {
        }

    private:
        static bool registered_;
    };

    inline bool StartDispenseEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(StartDispenseEvent),
                StartDispenseEvent::EventName,
                StartDispenseEvent::Version);
            return true;
        }();
}