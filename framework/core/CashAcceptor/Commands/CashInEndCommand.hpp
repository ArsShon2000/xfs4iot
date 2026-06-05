#pragma once

#include <memory>
#include <nlohmann/json.hpp>

#include "../../Command.hpp"
#include "../../MessageBase.hpp"
#include "../../common/Completions/MessagePayload.hpp"

namespace XFS4IoT::CashAcceptor::Commands
{
    class CashInEndCommand final
        : public XFS4IoT::Commands::Command<XFS4IoT::MessagePayloadBase>
    {
    public:
        static constexpr const char* CommandName = "CashAcceptor.CashInEnd";
        static constexpr const char* Version = "2.0";

        CashInEndCommand(int requestId, int timeout)
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
            if (!j.contains("header"))
                return nullptr;

            const auto& h = j.at("header");

            return std::make_shared<CashInEndCommand>(
                h.value("requestId", 0),
                h.value("timeout", 0));
        }

    private:
        void SerializePayload(nlohmann::json& j) const override
        {
            j["payload"] = nlohmann::json::object();
        }

        static bool registered_;
    };

    inline bool CashInEndCommand::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(CashInEndCommand),
                CashInEndCommand::CommandName,
                CashInEndCommand::Version);
            return true;
        }();
}