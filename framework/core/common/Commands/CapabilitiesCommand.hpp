#pragma once

#include <memory>
#include <nlohmann/json.hpp>

#include "../../Command.hpp"
#include "../../common/Completions/MessagePayload.hpp"
#include "../../MessageBase.hpp"

namespace XFS4IoT::Common::Commands
{
    class CapabilitiesCommand final : public XFS4IoT::Commands::Command<XFS4IoT::MessagePayloadBase>
    {
    public:
        static constexpr const char* CommandName = "Common.Capabilities";
        static constexpr const char* Version = "1.0";

        CapabilitiesCommand(int requestId, int timeout)
            : XFS4IoT::Commands::Command<XFS4IoT::MessagePayloadBase>(
                CommandName,
                Version,
                requestId,
                std::make_shared<XFS4IoT::MessagePayloadBase>(),
                timeout)
        {
        }

        static std::shared_ptr<XFS4IoT::MessageBase> FromJson(const nlohmann::json& j)
        {
            if (!j.contains("header")) {
                return nullptr;
            }

            const auto& h = j.at("header");

            int requestId = h.value("requestId", 0);
            int timeout = h.value("timeout", 0);

            return std::make_shared<CapabilitiesCommand>(requestId, timeout);
        }

    private:
        void SerializePayload(nlohmann::json& j) const override
        {
            j["payload"] = nlohmann::json::object();
        }

        static bool registered_;
    };

    inline bool CapabilitiesCommand::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(CapabilitiesCommand),
                CapabilitiesCommand::CommandName,
                CapabilitiesCommand::Version);
            return true;
        }();
}