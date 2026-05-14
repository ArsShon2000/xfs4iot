#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../../Events.hpp"
#include "../../Message.hpp"
#include "../../common/MessageHeader.hpp"
#include "../CashManagementSchemas.hpp"

namespace XFS4IoT::CashManagement::Events
{
    class InfoAvailableEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        class ItemInfoSummaryClass final
        {
        public:
            ItemInfoSummaryClass(
                std::optional<NoteLevelEnum> level = std::nullopt,
                std::optional<int> numOfItems = std::nullopt)
                : level_(level)
                , numOfItems_(numOfItems)
            {
            }

            const std::optional<NoteLevelEnum>& GetLevel() const noexcept
            {
                return level_;
            }

            void SetLevel(std::optional<NoteLevelEnum> level)
            {
                level_ = level;
            }

            const std::optional<int>& GetNumOfItems() const noexcept
            {
                return numOfItems_;
            }

            void SetNumOfItems(std::optional<int> numOfItems)
            {
                numOfItems_ = numOfItems;
            }

        private:
            std::optional<NoteLevelEnum> level_;
            std::optional<int> numOfItems_;
        };

        explicit InfoAvailableEventPayloadData(
            std::optional<std::vector<ItemInfoSummaryClass>> itemInfoSummary = std::nullopt)
            : itemInfoSummary_(std::move(itemInfoSummary))
        {
        }

        const std::optional<std::vector<ItemInfoSummaryClass>>& GetItemInfoSummary() const noexcept
        {
            return itemInfoSummary_;
        }

        void SetItemInfoSummary(std::optional<std::vector<ItemInfoSummaryClass>> itemInfoSummary)
        {
            itemInfoSummary_ = std::move(itemInfoSummary);
        }

    private:
        std::optional<std::vector<ItemInfoSummaryClass>> itemInfoSummary_;
    };

    inline void to_json(
        nlohmann::json& j,
        const InfoAvailableEventPayloadData::ItemInfoSummaryClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetLevel().has_value())
            j["level"] = toString(p.GetLevel().value());

        if (p.GetNumOfItems().has_value())
            j["numOfItems"] = p.GetNumOfItems().value();
    }

    inline void to_json(
        nlohmann::json& j,
        const InfoAvailableEventPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetItemInfoSummary().has_value())
            j["itemInfoSummary"] = p.GetItemInfoSummary().value();
    }

    class InfoAvailableEvent final
        : public XFS4IoT::Events::Event<InfoAvailableEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "CashManagement.InfoAvailableEvent";
        static constexpr const char* Version = "1.0";

        InfoAvailableEvent(
            int requestId,
            std::shared_ptr<InfoAvailableEventPayloadData> payload)
            : XFS4IoT::Events::Event<InfoAvailableEventPayloadData>(
                EventName,
                Version,
                requestId,
                std::move(payload))
        {
        }

    private:
        static bool registered_;
    };

    inline bool InfoAvailableEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(InfoAvailableEvent),
                InfoAvailableEvent::EventName,
                InfoAvailableEvent::Version);
            return true;
        }();
}