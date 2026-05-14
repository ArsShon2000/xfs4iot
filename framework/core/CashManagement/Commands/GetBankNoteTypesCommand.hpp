#pragma once

#include <memory>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "../../Command.hpp"
#include "../../MessageBase.hpp"
#include "../CashManagementSchemas.hpp"

namespace XFS4IoT::CashManagement::Commands
{
    class GetBankNoteTypesCommand final
        : public XFS4IoT::Commands::Command<MessagePayloadBase>
    {
    public:
        static constexpr const char* CommandName = "CashManagement.GetBankNoteTypes";
        static constexpr const char* Version = "1.0";

        GetBankNoteTypesCommand(
            int requestId,
            std::shared_ptr<MessagePayloadBase> payload,
            int timeout)
            : XFS4IoT::Commands::Command<MessagePayloadBase>(
                CommandName,
                Version,
                requestId,
                std::move(payload),
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

            std::shared_ptr<MessagePayloadBase> payload =
                std::make_shared<MessagePayloadBase>();

            if (j.contains("payload") && j.at("payload").is_object())
            {
                const auto& p = j.at("payload");
            }

            return std::make_shared<GetBankNoteTypesCommand>(requestId, payload, timeout);
        }

    private:
        static bool registered_;
    };

    inline bool GetBankNoteTypesCommand::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(GetBankNoteTypesCommand),
                GetBankNoteTypesCommand::CommandName,
                GetBankNoteTypesCommand::Version);
            return true;
        }();
}