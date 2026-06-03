#pragma once

#include <memory>
#include <optional>
#include <string>
#include <nlohmann/json.hpp>

#include "../../Command.hpp"
#include "../../MessageBase.hpp"
#include "../../common/Completions/MessagePayload.hpp"

namespace XFS4IoT::Common::Commands
{
    class UnsupportedCommand final
        : public XFS4IoT::Commands::Command<XFS4IoT::MessagePayloadBase>
    {
    public:
        UnsupportedCommand(
            std::string commandName,
            std::string version,
            int requestId,
            int timeout,
            nlohmann::json rawMessage = {})
            : XFS4IoT::Commands::Command<XFS4IoT::MessagePayloadBase>(
                commandName,
                version,
                requestId,
                std::make_shared<XFS4IoT::MessagePayloadBase>(),
                timeout)
            , commandName_(std::move(commandName))
            , version_(std::move(version))
            , rawMessage_(std::move(rawMessage))
        {
        }

        const std::string& GetCommandName() const noexcept
        {
            return commandName_;
        }

        const std::string& GetVersion() const noexcept
        {
            return version_;
        }

        const nlohmann::json& GetRawMessage() const noexcept
        {
            return rawMessage_;
        }

        static std::shared_ptr<XFS4IoT::MessageBase> FromJson(const nlohmann::json& j)
        {
            if (!j.contains("header"))
            {
                return nullptr;
            }

            const auto& h = j.at("header");

            const std::string name = h.value("name", "");
            const std::string version = h.value("version", "1.0");
            const int requestId = h.value("requestId", 0);
            const int timeout = h.value("timeout", 0);

            return std::make_shared<UnsupportedCommand>(
                name,
                version,
                requestId,
                timeout,
                j);
        }

    private:
        void SerializePayload(nlohmann::json& j) const override
        {
            j["payload"] = nlohmann::json::object();
        }

    private:
        std::string commandName_;
        std::string version_;
        nlohmann::json rawMessage_;
    };
}