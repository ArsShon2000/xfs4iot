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
    class CountsChangedEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        CountsChangedEventPayloadData() = default;

        const std::unordered_map<std::string, std::shared_ptr<StorageUnitClass>>&
            GetExtendedProperties() const noexcept
        {
            return extendedProperties_;
        }

        void SetExtendedProperties(
            std::unordered_map<std::string, std::shared_ptr<StorageUnitClass>> extendedProperties)
        {
            extendedProperties_ = std::move(extendedProperties);
        }

        void AddExtendedProperty(
            const std::string& key,
            std::shared_ptr<StorageUnitClass> value)
        {
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
        const CountsChangedEventPayloadData& p)
    {
        j = nlohmann::json::object();

        for (const auto& [key, value] : p.GetExtendedProperties())
        {
            if (value)
                j[key] = *value;
        }
    }

    class CountsChangedEvent final
        : public XFS4IoT::Events::UnsolicitedEvent<CountsChangedEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "Storage.CountsChangedEvent";
        static constexpr const char* Version = "1.0";

        explicit CountsChangedEvent(
            std::shared_ptr<CountsChangedEventPayloadData> payload)
            : XFS4IoT::Events::UnsolicitedEvent<CountsChangedEventPayloadData>(
                EventName,
                Version,
                std::move(payload))
        {
        }

    private:
        static bool registered_;
    };

    inline bool CountsChangedEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(CountsChangedEvent),
                CountsChangedEvent::EventName,
                CountsChangedEvent::Version);
            return true;
        }();
}