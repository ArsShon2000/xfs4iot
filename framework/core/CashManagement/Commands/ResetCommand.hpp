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
    class ResetCommandPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        explicit ResetCommandPayloadData(
            std::shared_ptr<XFS4IoT::CashManagement::ItemTargetDataClass> position = nullptr)
            : position_(std::move(position))
        {
        }

        const std::shared_ptr<XFS4IoT::CashManagement::ItemTargetDataClass>& GetPosition() const noexcept
        {
            return position_;
        }

        void SetPosition(std::shared_ptr<XFS4IoT::CashManagement::ItemTargetDataClass> position)
        {
            position_ = std::move(position);
        }

    private:
        std::shared_ptr<XFS4IoT::CashManagement::ItemTargetDataClass> position_;
    };

    inline void to_json(nlohmann::json& j, const ResetCommandPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetPosition())
        {
            j["position"] = *p.GetPosition();
        }
    }

    class ResetCommand final
        : public XFS4IoT::Commands::Command<ResetCommandPayloadData>
    {
    public:
        static constexpr const char* CommandName = "CashManagement.Reset";
        static constexpr const char* Version = "1.0";

        ResetCommand(
            int requestId,
            std::shared_ptr<ResetCommandPayloadData> payload,
            int timeout)
            : XFS4IoT::Commands::Command<ResetCommandPayloadData>(
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

            std::shared_ptr<ResetCommandPayloadData> payload =
                std::make_shared<ResetCommandPayloadData>();

            if (j.contains("payload") && j.at("payload").is_object())
            {
                const auto& p = j.at("payload");

                if (p.contains("position") && p.at("position").is_object())
                {
                    const auto& pos = p.at("position");

                    std::optional<XFS4IoT::CashManagement::ItemTargetEnumEnum> target = std::nullopt;
                    std::optional<std::string> unit = std::nullopt;
                    std::optional<int> index = std::nullopt;

                    if (pos.contains("target") && !pos.at("target").is_null())
                    {
                        // Предполагается, что у тебя уже есть from_json для ItemTargetEnumEnum
                        target = pos.at("target").get<XFS4IoT::CashManagement::ItemTargetEnumEnum>();
                    }

                    if (pos.contains("unit") && !pos.at("unit").is_null())
                    {
                        unit = pos.at("unit").get<std::string>();
                    }

                    if (pos.contains("index") && !pos.at("index").is_null())
                    {
                        index = pos.at("index").get<int>();
                    }

                    payload->SetPosition(
                        std::make_shared<XFS4IoT::CashManagement::ItemTargetDataClass>(
                            target,
                            unit,
                            index));
                }
            }

            return std::make_shared<ResetCommand>(requestId, payload, timeout);
        }

    private:
        static bool registered_;
    };

    inline bool ResetCommand::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(ResetCommand),
                ResetCommand::CommandName,
                ResetCommand::Version);
            return true;
        }();
}