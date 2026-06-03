#pragma once

#include <memory>
#include <string>

#include "../../Events.hpp"
#include "../../Message.hpp"
#include "../../common/MessageHeader.hpp"

namespace XFS4IoT::CashAcceptor::Events
{
    class InsertItemsEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        InsertItemsEventPayloadData() = default;
    };

    inline void to_json(nlohmann::json& j, const InsertItemsEventPayloadData&)
    {
        j = nlohmann::json::object();
    }

    class InsertItemsEvent final : public XFS4IoT::Events::Event<InsertItemsEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "CashAcceptor.InsertItemsEvent";
        static constexpr const char* Version = "2.0";

        explicit InsertItemsEvent(int requestId)
            : XFS4IoT::Events::Event<InsertItemsEventPayloadData>(
                EventName,
                Version,
                requestId,
                std::make_shared<InsertItemsEventPayloadData>())
        {
        }

    private:
        static bool registered_;
    };

    inline bool InsertItemsEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(InsertItemsEvent),
                InsertItemsEvent::EventName,
                InsertItemsEvent::Version);
            return true;
        }();
}