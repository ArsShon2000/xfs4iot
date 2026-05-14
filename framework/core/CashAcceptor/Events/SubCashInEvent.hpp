#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "../../Events.hpp"
#include "../../Message.hpp"
#include "../../common/MessageHeader.hpp"
#include "../../CashManagement/CashManagementSchemas.hpp"

namespace XFS4IoT::CashAcceptor::Events
{
    class SubCashInEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        explicit SubCashInEventPayloadData(
            std::optional<int> unrecognized = std::nullopt)
            : unrecognized_(unrecognized)
        {
        }

        std::optional<int> GetUnrecognized() const { return unrecognized_; }
        void SetUnrecognized(std::optional<int> unrecognized) { unrecognized_ = unrecognized; }

        const std::unordered_map<std::string, std::shared_ptr<CashManagement::StorageCashCountClass>>&
            GetExtendedProperties() const
        {
            return extendedProperties_;
        }

        void SetExtendedProperties(
            std::unordered_map<std::string, std::shared_ptr<CashManagement::StorageCashCountClass>> extendedProperties)
        {
            extendedProperties_ = std::move(extendedProperties);
        }

        void AddExtendedProperty(
            const std::string& key,
            std::shared_ptr<CashManagement::StorageCashCountClass> value)
        {
            extendedProperties_[key] = std::move(value);
        }

        std::shared_ptr<CashManagement::StorageCashCountClass> GetExtendedProperty(const std::string& key) const
        {
            auto it = extendedProperties_.find(key);
            return it != extendedProperties_.end() ? it->second : nullptr;
        }

    private:
        std::optional<int> unrecognized_;
        std::unordered_map<std::string, std::shared_ptr<CashManagement::StorageCashCountClass>> extendedProperties_;
    };

    inline void to_json(
        nlohmann::json& j,
        const SubCashInEventPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetUnrecognized().has_value())
            j["unrecognized"] = p.GetUnrecognized().value();

        for (const auto& [key, value] : p.GetExtendedProperties())
        {
            if (value)
                j[key] = *value;
        }
    }

    class SubCashInEvent final : public XFS4IoT::Events::Event<SubCashInEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "CashAcceptor.SubCashInEvent";
        static constexpr const char* Version = "1.0";

        SubCashInEvent(
            int requestId,
            std::shared_ptr<SubCashInEventPayloadData> payload)
            : XFS4IoT::Events::Event<SubCashInEventPayloadData>(
                EventName,
                Version,
                requestId,
                std::move(payload))
        {
        }

    private:
        static bool registered_;
    };

    inline bool SubCashInEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(SubCashInEvent),
                SubCashInEvent::EventName,
                SubCashInEvent::Version);
            return true;
        }();
}