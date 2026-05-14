#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <cctype>
#include <stdexcept>

#include "../../Events.hpp"
#include "../../Message.hpp"
#include "../../common/MessageHeader.hpp"
#include "../StorageSchemas.hpp"

namespace XFS4IoT::Storage::Events
{
    inline bool IsValidUnitKey(const std::string& key)
    {
        if (key.size() < 5) return false;
        if (key.rfind("unit", 0) != 0) return false;

        for (size_t i = 4; i < key.size(); ++i)
        {
            const unsigned char c = static_cast<unsigned char>(key[i]);
            if (!std::isalnum(c))
                return false;
        }

        return true;
    }

    class StorageThresholdEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        StorageThresholdEventPayloadData() = default;

        const std::unordered_map<std::string, std::shared_ptr<StorageUnitClass>>&
            GetExtendedProperties() const noexcept
        {
            return extendedProperties_;
        }

        void SetExtendedProperties(
            std::unordered_map<std::string, std::shared_ptr<StorageUnitClass>> extendedProperties)
        {
            for (const auto& [key, _] : extendedProperties)
            {
                if (!IsValidUnitKey(key))
                    throw std::invalid_argument("Invalid unit key: " + key);
            }

            extendedProperties_ = std::move(extendedProperties);
        }

        void AddExtendedProperty(
            const std::string& key,
            std::shared_ptr<StorageUnitClass> value)
        {
            if (!IsValidUnitKey(key))
                throw std::invalid_argument("Invalid unit key: " + key);

            extendedProperties_[key] = std::move(value);
        }

        std::shared_ptr<StorageUnitClass> GetExtendedProperty(const std::string& key) const
        {
            auto it = extendedProperties_.find(key);
            return it != extendedProperties_.end() ? it->second : nullptr;
        }

    private:
        std::unordered_map<std::string, std::shared_ptr<StorageUnitClass>> extendedProperties_;
    };

    inline void to_json(
        nlohmann::json& j,
        const StorageThresholdEventPayloadData& p)
    {
        j = nlohmann::json::object();

        for (const auto& [key, value] : p.GetExtendedProperties())
        {
            if (!IsValidUnitKey(key))
                throw std::invalid_argument("Invalid unit key during serialization: " + key);

            if (value)
                j[key] = *value;
        }
    }

    class StorageThresholdEvent final
        : public XFS4IoT::Events::UnsolicitedEvent<StorageThresholdEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "Storage.StorageThresholdEvent";
        static constexpr const char* Version = "2.1";

        explicit StorageThresholdEvent(
            std::shared_ptr<StorageThresholdEventPayloadData> payload)
            : XFS4IoT::Events::UnsolicitedEvent<StorageThresholdEventPayloadData>(
                EventName,
                Version,
                std::move(payload))
        {
        }

    private:
        static bool registered_;
    };

    inline bool StorageThresholdEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(StorageThresholdEvent),
                StorageThresholdEvent::EventName,
                StorageThresholdEvent::Version);
            return true;
        }();
}