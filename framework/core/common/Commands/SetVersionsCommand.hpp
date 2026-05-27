#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "../../Command.hpp"
#include "../../MessageBase.hpp"

namespace XFS4IoT::Common::Commands
{
    class SetVersionsCommandPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        SetVersionsCommandPayloadData(
            std::optional<std::unordered_map<std::string, int>> commands = std::nullopt,
            std::optional<std::unordered_map<std::string, int>> events = std::nullopt)
            : commands_(std::move(commands))
            , events_(std::move(events))
        {
        }

        const std::optional<std::unordered_map<std::string, int>>& GetCommands() const noexcept
        {
            return commands_;
        }

        void SetCommands(std::optional<std::unordered_map<std::string, int>> commands)
        {
            commands_ = std::move(commands);
        }

        const std::optional<std::unordered_map<std::string, int>>& GetEvents() const noexcept
        {
            return events_;
        }

        void SetEvents(std::optional<std::unordered_map<std::string, int>> events)
        {
            events_ = std::move(events);
        }

    private:
        std::optional<std::unordered_map<std::string, int>> commands_;
        std::optional<std::unordered_map<std::string, int>> events_;
    };

    inline void to_json(nlohmann::json& j, const SetVersionsCommandPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetCommands().has_value())
        {
            j["commands"] = p.GetCommands().value();
        }

        if (p.GetEvents().has_value())
        {
            j["events"] = p.GetEvents().value();
        }
    }

    class SetVersionsCommand final
        : public XFS4IoT::Commands::Command<SetVersionsCommandPayloadData>
    {
    public:
        static constexpr const char* CommandName = "Common.SetVersions";
        static constexpr const char* Version = "1.0";

        SetVersionsCommand(
            int requestId,
            std::shared_ptr<SetVersionsCommandPayloadData> payload,
            int timeout)
            : XFS4IoT::Commands::Command<SetVersionsCommandPayloadData>(
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

            auto payload = std::make_shared<SetVersionsCommandPayloadData>();

            if (j.contains("payload") && j.at("payload").is_object())
            {
                const auto& p = j.at("payload");

                if (p.contains("commands") && p.at("commands").is_object())
                {
                    payload->SetCommands(
                        p.at("commands").get<std::unordered_map<std::string, int>>());
                }

                if (p.contains("events") && p.at("events").is_object())
                {
                    payload->SetEvents(
                        p.at("events").get<std::unordered_map<std::string, int>>());
                }
            }

            return std::make_shared<SetVersionsCommand>(
                requestId,
                payload,
                timeout);
        }

    private:
        static bool registered_;
    };

    inline bool SetVersionsCommand::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(SetVersionsCommand),
                SetVersionsCommand::CommandName,
                SetVersionsCommand::Version);
            return true;
        }();
}