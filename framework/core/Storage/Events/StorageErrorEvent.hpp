#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "../../Events.hpp"
#include "../../Message.hpp"
#include "../../common/MessageHeader.hpp"
#include "../StorageSchemas.hpp"

namespace XFS4IoT::Storage::Events
{
    class StorageErrorEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        enum class FailureEnum
        {
            Empty,
            Error,
            Full,
            Locked,
            Invalid,
            Config,
            NotConfigured,
            FeedModuleProblem,
            PhysicalLocked,
            PhysicalUnlocked
        };

        StorageErrorEventPayloadData(
            std::optional<FailureEnum> failure = std::nullopt,
            std::optional<std::unordered_map<std::string, std::shared_ptr<StorageUnitClass>>> unit = std::nullopt)
            : failure_(failure)
            , unit_(std::move(unit))
        {
        }

        std::optional<FailureEnum> GetFailure() const noexcept
        {
            return failure_;
        }

        void SetFailure(std::optional<FailureEnum> failure)
        {
            failure_ = failure;
        }

        const std::optional<std::unordered_map<std::string, std::shared_ptr<StorageUnitClass>>>&
            GetUnit() const noexcept
        {
            return unit_;
        }

        void SetUnit(std::optional<std::unordered_map<std::string, std::shared_ptr<StorageUnitClass>>> unit)
        {
            unit_ = std::move(unit);
        }

    private:
        std::optional<FailureEnum> failure_;
        std::optional<std::unordered_map<std::string, std::shared_ptr<StorageUnitClass>>> unit_;
    };

    inline std::string ToString(StorageErrorEventPayloadData::FailureEnum failure)
    {
        switch (failure)
        {
        case StorageErrorEventPayloadData::FailureEnum::Empty:
            return "empty";
        case StorageErrorEventPayloadData::FailureEnum::Error:
            return "error";
        case StorageErrorEventPayloadData::FailureEnum::Full:
            return "full";
        case StorageErrorEventPayloadData::FailureEnum::Locked:
            return "locked";
        case StorageErrorEventPayloadData::FailureEnum::Invalid:
            return "invalid";
        case StorageErrorEventPayloadData::FailureEnum::Config:
            return "config";
        case StorageErrorEventPayloadData::FailureEnum::NotConfigured:
            return "notConfigured";
        case StorageErrorEventPayloadData::FailureEnum::FeedModuleProblem:
            return "feedModuleProblem";
        case StorageErrorEventPayloadData::FailureEnum::PhysicalLocked:
            return "physicalLocked";
        case StorageErrorEventPayloadData::FailureEnum::PhysicalUnlocked:
            return "physicalUnlocked";
        default:
            throw std::invalid_argument("Unknown FailureEnum value");
        }
    }

    inline void to_json(
        nlohmann::json& j,
        const StorageErrorEventPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetFailure().has_value())
            j["failure"] = ToString(p.GetFailure().value());

        if (p.GetUnit().has_value())
        {
            j["unit"] = nlohmann::json::object();

            for (const auto& [key, value] : p.GetUnit().value())
            {
                if (value)
                    j["unit"][key] = *value;
            }
        }
    }

    class StorageErrorEvent final
        : public XFS4IoT::Events::Event<StorageErrorEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "Storage.StorageErrorEvent";
        static constexpr const char* Version = "2.1";

        StorageErrorEvent(
            int requestId,
            std::shared_ptr<StorageErrorEventPayloadData> payload)
            : XFS4IoT::Events::Event<StorageErrorEventPayloadData>(
                EventName,
                Version,
                requestId,
                std::move(payload))
        {
        }

    private:
        static bool registered_;
    };

    inline bool StorageErrorEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(StorageErrorEvent),
                StorageErrorEvent::EventName,
                StorageErrorEvent::Version);
            return true;
        }();
}