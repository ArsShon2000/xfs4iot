#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "../../Command.hpp"
#include "../../MessageBase.hpp"

namespace XFS4IoT::CashAcceptor::Commands
{
    class ConfigureNoteTypesCommandPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        class ItemsClass final
        {
        public:
            ItemsClass(
                std::optional<std::string> item = std::nullopt,
                std::optional<bool> enabled = std::nullopt)
                : item_(std::move(item))
                , enabled_(enabled)
            {
            }

            const std::optional<std::string>& GetItem() const noexcept
            {
                return item_;
            }

            void SetItem(std::optional<std::string> item)
            {
                item_ = std::move(item);
            }

            const std::optional<bool>& GetEnabled() const noexcept
            {
                return enabled_;
            }

            void SetEnabled(std::optional<bool> enabled)
            {
                enabled_ = enabled;
            }

        private:
            std::optional<std::string> item_;
            std::optional<bool> enabled_;
        };

        explicit ConfigureNoteTypesCommandPayloadData(
            std::optional<std::vector<ItemsClass>> items = std::nullopt)
            : items_(std::move(items))
        {
        }

        const std::optional<std::vector<ItemsClass>>& GetItems() const noexcept
        {
            return items_;
        }

        void SetItems(std::optional<std::vector<ItemsClass>> items)
        {
            items_ = std::move(items);
        }

    private:
        std::optional<std::vector<ItemsClass>> items_;
    };

    inline void to_json(
        nlohmann::json& j,
        const ConfigureNoteTypesCommandPayloadData::ItemsClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetItem().has_value())
        {
            j["item"] = p.GetItem().value();
        }

        if (p.GetEnabled().has_value())
        {
            j["enabled"] = p.GetEnabled().value();
        }
    }

    inline void from_json(
        const nlohmann::json& j,
        ConfigureNoteTypesCommandPayloadData::ItemsClass& p)
    {
        std::optional<std::string> item = std::nullopt;
        std::optional<bool> enabled = std::nullopt;

        if (j.contains("item") && !j.at("item").is_null())
        {
            item = j.at("item").get<std::string>();
        }

        if (j.contains("enabled") && !j.at("enabled").is_null())
        {
            enabled = j.at("enabled").get<bool>();
        }

        p = ConfigureNoteTypesCommandPayloadData::ItemsClass(
            std::move(item),
            enabled);
    }

    inline void to_json(
        nlohmann::json& j,
        const ConfigureNoteTypesCommandPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetItems().has_value())
        {
            j["items"] = p.GetItems().value();
        }
    }

    class ConfigureNoteTypesCommand final
        : public XFS4IoT::Commands::Command<ConfigureNoteTypesCommandPayloadData>
    {
    public:
        static constexpr const char* CommandName = "CashAcceptor.ConfigureNoteTypes";
        static constexpr const char* Version = "1.0";

        ConfigureNoteTypesCommand(
            int requestId,
            std::shared_ptr<ConfigureNoteTypesCommandPayloadData> payload,
            int timeout)
            : XFS4IoT::Commands::Command<ConfigureNoteTypesCommandPayloadData>(
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

            auto payload =
                std::make_shared<ConfigureNoteTypesCommandPayloadData>();

            if (j.contains("payload") && j.at("payload").is_object())
            {
                const auto& p = j.at("payload");

                if (p.contains("items") && p.at("items").is_array())
                {
                    std::vector<ConfigureNoteTypesCommandPayloadData::ItemsClass> items;

                    for (const auto& itemJson : p.at("items"))
                    {
                        items.emplace_back(
                            itemJson.get<ConfigureNoteTypesCommandPayloadData::ItemsClass>());
                    }

                    payload->SetItems(std::move(items));
                }
            }

            return std::make_shared<ConfigureNoteTypesCommand>(
                requestId,
                payload,
                timeout);
        }

    private:
        static bool registered_;
    };

    inline bool ConfigureNoteTypesCommand::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(ConfigureNoteTypesCommand),
                ConfigureNoteTypesCommand::CommandName,
                ConfigureNoteTypesCommand::Version);
            return true;
        }();
}