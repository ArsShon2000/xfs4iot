#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "../../Events.hpp"
#include "../../Message.hpp"
#include "../../common/MessageHeader.hpp"
#include "../StorageSchemas.hpp"

namespace XFS4IoT::Storage::Events
{
    // ---- fast validation вместо regex ----
    inline bool IsValidUnitKey(const std::string& key)
    {
        if (key.size() < 5) return false; // минимум "unitX"
        if (key.rfind("unit", 0) != 0) return false; // должен начинаться с "unit"

        for (size_t i = 4; i < key.size(); ++i)
        {
            const char c = key[i];
            if (!std::isalnum(static_cast<unsigned char>(c)))
                return false;
        }

        return true;
    }

    class StorageChangedEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        StorageChangedEventPayloadData() = default;

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
        const StorageChangedEventPayloadData& p)
    {
        j = nlohmann::json::object();

        for (const auto& [key, value] : p.GetExtendedProperties())
        {
            // защита на всякий случай
            if (!IsValidUnitKey(key))
                throw std::invalid_argument("Invalid unit key during serialization: " + key);

            if (value)
                j[key] = *value;
        }
    }

    class StorageChangedEvent final
        : public XFS4IoT::Events::UnsolicitedEvent<StorageChangedEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "Storage.StorageChangedEvent";
        static constexpr const char* Version = "2.1";

        explicit StorageChangedEvent(
            std::shared_ptr<StorageChangedEventPayloadData> payload)
            : XFS4IoT::Events::UnsolicitedEvent<StorageChangedEventPayloadData>(
                EventName,
                Version,
                std::move(payload))
        {
        }

    private:
        static bool registered_;
    };

    inline bool StorageChangedEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(StorageChangedEvent),
                StorageChangedEvent::EventName,
                StorageChangedEvent::Version);
            return true;
        }();
}