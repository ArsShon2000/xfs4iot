#pragma once

#include <memory>

#include <nlohmann/json.hpp>

#include "../../Command.hpp"
#include "../../MessageBase.hpp"
#include "../../common/Completions/MessagePayload.hpp"

namespace XFS4IoT::CashAcceptor::Commands
{
    class CashInCommand final
        : public XFS4IoT::Commands::Command<XFS4IoT::MessagePayloadBase>
    {
    public:
        static constexpr const char* CommandName = "CashAcceptor.CashIn";
        static constexpr const char* Version = "1.0";

        CashInCommand(int requestId, int timeout)
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
            {
                return nullptr;
            }

            const auto& h = j.at("header");

            const int requestId = h.value("requestId", 0);
            const int timeout = h.value("timeout", 0);

            return std::make_shared<CashInCommand>(
                requestId,
                timeout);
        }

    private:
        void SerializePayload(nlohmann::json& j) const override
        {
            j["payload"] = nlohmann::json::object();
        }

        static bool registered_;
    };

    inline bool CashInCommand::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(CashInCommand),
                CashInCommand::CommandName,
                CashInCommand::Version);
            return true;
        }();
}