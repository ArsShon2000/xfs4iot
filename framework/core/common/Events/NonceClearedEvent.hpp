#pragma once

#include <memory>
#include <optional>
#include <string>

#include "../../Events.hpp"
#include "../../Message.hpp"
#include "../../common/MessageHeader.hpp"

namespace XFS4IoT::Common::Events
{
    class NonceClearedEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        explicit NonceClearedEventPayloadData(
            std::optional<std::string> reasonDescription = std::nullopt)
            : reasonDescription_(std::move(reasonDescription))
        {
        }

        const std::optional<std::string>& GetReasonDescription() const noexcept
        {
            return reasonDescription_;
        }

        void SetReasonDescription(std::optional<std::string> reasonDescription)
        {
            reasonDescription_ = std::move(reasonDescription);
        }

    private:
        std::optional<std::string> reasonDescription_;
    };

    inline void to_json(
        nlohmann::json& j,
        const NonceClearedEventPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetReasonDescription().has_value())
        {
            j["reasonDescription"] = p.GetReasonDescription().value();
        }
    }

    class NonceClearedEvent final
        : public XFS4IoT::Events::UnsolicitedEvent<NonceClearedEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "Common.NonceClearedEvent";
        static constexpr const char* Version = "1.0";

        explicit NonceClearedEvent(
            std::shared_ptr<NonceClearedEventPayloadData> payload)
            : XFS4IoT::Events::UnsolicitedEvent<NonceClearedEventPayloadData>(
                EventName,
                Version,
                std::move(payload))
        {
        }

    private:
        static bool registered_;
    };

    inline bool NonceClearedEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(NonceClearedEvent),
                NonceClearedEvent::EventName,
                NonceClearedEvent::Version);
            return true;
        }();
}