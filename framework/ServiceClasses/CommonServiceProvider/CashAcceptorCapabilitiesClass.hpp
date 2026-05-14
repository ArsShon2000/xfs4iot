#pragma once

#include <map>
#include <string>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>
#include <nlohmann/json.hpp>

#include "CashManagementCapabilitiesClass.hpp"

namespace XFS4IoTFramework::Common
{
    class CashAcceptorCapabilitiesClass
    {
    public:
        enum class CashInLimitEnum : uint8_t
        {
            NotSupported = 0,
            ByTotalItems = 1 << 0,
            ByAmount = 1 << 1,
        };

        enum class CountActionEnum : uint8_t
        {
            NotSupported = 0,
            Individual = 1 << 0,
            All = 1 << 1,
        };

        enum class RetainCounterfeitActionEnum : uint8_t
        {
            NotSupported = 0,
            Level2 = 1 << 0,
            Level3 = 1 << 1,
            Inked = 1 << 2,
        };

        class PositionClass
        {
        public:
            enum class UsageEnum : uint8_t
            {
                NotSupported = 0,
                In = 1 << 0,
                Refuse = 1 << 1,
                Rollback = 1 << 2,
            };

            enum class RetractAreaEnum : uint8_t
            {
                NotSupported = 0,
                Retract = 1 << 0,
                Reject = 1 << 1,
                Transport = 1 << 2,
                Stacker = 1 << 3,
                BillCassettes = 1 << 4,
                CashIn = 1 << 5,
                ItemCassette = 1 << 6,
            };

            PositionClass(
                UsageEnum usage,
                bool shutterControl,
                bool itemsTakenSensor,
                bool itemsInsertedSensor,
                bool presentControl,
                bool preparePresent,
                RetractAreaEnum retractArea)
                : usage_(usage)
                , shutterControl_(shutterControl)
                , itemsTakenSensor_(itemsTakenSensor)
                , itemsInsertedSensor_(itemsInsertedSensor)
                , presentControl_(presentControl)
                , preparePresent_(preparePresent)
                , retractArea_(retractArea)
            {
            }

            PositionClass()
                : usage_(UsageEnum::NotSupported)
                , shutterControl_(false)
                , itemsTakenSensor_(false)
                , itemsInsertedSensor_(false)
                , presentControl_(false)
                , preparePresent_(false)
                , retractArea_(RetractAreaEnum::NotSupported)
            {
            }

            CashManagementCapabilitiesClass::PositionEnum GetPosition() const noexcept { return position_; }
            UsageEnum GetUsage() const noexcept { return usage_; }
            bool GetShutterControl() const noexcept { return shutterControl_; }
            bool GetItemsTakenSensor() const noexcept { return itemsTakenSensor_; }
            bool GetItemsInsertedSensor() const noexcept { return itemsInsertedSensor_; }
            bool GetPresentControl() const noexcept { return presentControl_; }
            bool GetPreparePresent() const noexcept { return preparePresent_; }
            RetractAreaEnum GetRetractArea() const noexcept { return retractArea_; }

            void SetPosition(CashManagementCapabilitiesClass::PositionEnum value) noexcept { position_ = value; }
            void SetUsage(UsageEnum value) noexcept { usage_ = value; }
            void SetShutterControl(bool value) noexcept { shutterControl_ = value; }
            void SetItemsTakenSensor(bool value) noexcept { itemsTakenSensor_ = value; }
            void SetItemsInsertedSensor(bool value) noexcept { itemsInsertedSensor_ = value; }
            void SetPresentControl(bool value) noexcept { presentControl_ = value; }
            void SetPreparePresent(bool value) noexcept { preparePresent_ = value; }
            void SetRetractArea(RetractAreaEnum value) noexcept { retractArea_ = value; }

            static bool HasFlag(UsageEnum value, UsageEnum flag)
            {
                return (static_cast<uint8_t>(value) & static_cast<uint8_t>(flag)) != 0;
            }

            static bool HasFlag(RetractAreaEnum value, RetractAreaEnum flag)
            {
                return (static_cast<uint8_t>(value) & static_cast<uint8_t>(flag)) != 0;
            }

        private:
            CashManagementCapabilitiesClass::PositionEnum position_;
            UsageEnum usage_;
            bool shutterControl_;
            bool itemsTakenSensor_;
            bool itemsInsertedSensor_;
            bool presentControl_;
            bool preparePresent_;
            RetractAreaEnum retractArea_;
        };

        CashAcceptorCapabilitiesClass(
            std::optional<CashManagementCapabilitiesClass::TypeEnum> type = std::nullopt,
            std::optional<int> maxCashInItems = std::nullopt,
            std::optional<bool> shutter = std::nullopt,
            std::optional<bool> shutterControl = std::nullopt,
            std::optional<int> intermediateStacker = std::nullopt,
            std::optional<bool> itemsTakenSensor = std::nullopt,
            std::optional<std::map<CashManagementCapabilitiesClass::PositionEnum, PositionClass>> positions = std::nullopt,
            std::optional<CashManagementCapabilitiesClass::RetractAreaEnum> retractAreas = std::nullopt,
            std::optional<CashManagementCapabilitiesClass::RetractTransportActionEnum> retractTransportActions = std::nullopt,
            std::optional<CashManagementCapabilitiesClass::RetractStackerActionEnum> retractStackerActions = std::nullopt,
            std::optional<CashInLimitEnum> cashInLimit = std::nullopt,
            std::optional<CountActionEnum> countActions = std::nullopt,
            std::optional<RetainCounterfeitActionEnum> retainCounterfeitAction = std::nullopt)
            : type_(type)
            , maxCashInItems_(maxCashInItems)
            , shutter_(shutter)
            , shutterControl_(shutterControl)
            , intermediateStacker_(intermediateStacker)
            , itemsTakenSensor_(itemsTakenSensor)
            , positions_(std::move(positions))
            , retractAreas_(retractAreas)
            , retractTransportActions_(retractTransportActions)
            , retractStackerActions_(retractStackerActions)
            , cashInLimit_(cashInLimit)
            , countActions_(countActions)
            , retainCounterfeitAction_(retainCounterfeitAction)
        {
        }

        const std::optional<CashManagementCapabilitiesClass::TypeEnum>& GetType() const noexcept { return type_; }
        const std::optional<int>& GetMaxCashInItems() const noexcept { return maxCashInItems_; }
        const std::optional<bool>& GetShutter() const noexcept { return shutter_; }
        const std::optional<bool>& GetShutterControl() const noexcept { return shutterControl_; }
        const std::optional<int>& GetIntermediateStacker() const noexcept { return intermediateStacker_; }
        const std::optional<bool>& GetItemsTakenSensor() const noexcept { return itemsTakenSensor_; }

        const std::optional<std::map<CashManagementCapabilitiesClass::PositionEnum, PositionClass>>&
            GetPositions() const noexcept
        {
            return positions_;
        }

        const std::optional<CashManagementCapabilitiesClass::RetractAreaEnum>& GetRetractAreas() const noexcept { return retractAreas_; }
        const std::optional<CashManagementCapabilitiesClass::RetractTransportActionEnum>& GetRetractTransportActions() const noexcept { return retractTransportActions_; }
        const std::optional<CashManagementCapabilitiesClass::RetractStackerActionEnum>& GetRetractStackerActions() const noexcept { return retractStackerActions_; }
        const std::optional<CashInLimitEnum>& GetCashInLimit() const noexcept { return cashInLimit_; }
        const std::optional<CountActionEnum>& GetCountActions() const noexcept { return countActions_; }
        const std::optional<RetainCounterfeitActionEnum>& GetRetainCounterfeitAction() const noexcept { return retainCounterfeitAction_; }

        static bool HasFlag(CashInLimitEnum value, CashInLimitEnum flag)
        {
            return (static_cast<uint8_t>(value) & static_cast<uint8_t>(flag)) != 0;
        }

        static bool HasFlag(CountActionEnum value, CountActionEnum flag)
        {
            return (static_cast<uint8_t>(value) & static_cast<uint8_t>(flag)) != 0;
        }

        static bool HasFlag(RetainCounterfeitActionEnum value, RetainCounterfeitActionEnum flag)
        {
            return (static_cast<uint8_t>(value) & static_cast<uint8_t>(flag)) != 0;
        }

    private:
        std::optional<CashManagementCapabilitiesClass::TypeEnum> type_;
        std::optional<int> maxCashInItems_;
        std::optional<bool> shutter_;
        std::optional<bool> shutterControl_;
        std::optional<int> intermediateStacker_;
        std::optional<bool> itemsTakenSensor_;
        std::optional<std::map<CashManagementCapabilitiesClass::PositionEnum, PositionClass>> positions_;
        std::optional<CashManagementCapabilitiesClass::RetractAreaEnum> retractAreas_;
        std::optional<CashManagementCapabilitiesClass::RetractTransportActionEnum> retractTransportActions_;
        std::optional<CashManagementCapabilitiesClass::RetractStackerActionEnum> retractStackerActions_;
        std::optional<CashInLimitEnum> cashInLimit_;
        std::optional<CountActionEnum> countActions_;
        std::optional<RetainCounterfeitActionEnum> retainCounterfeitAction_;
    };

    inline std::string toString(CashAcceptorCapabilitiesClass::CashInLimitEnum value)
    {
        switch (value)
        {
        case CashAcceptorCapabilitiesClass::CashInLimitEnum::NotSupported: return "notSupported";
        case CashAcceptorCapabilitiesClass::CashInLimitEnum::ByTotalItems:  return "byTotalItems";
        case CashAcceptorCapabilitiesClass::CashInLimitEnum::ByAmount:      return "byAmount";
        }

        throw std::invalid_argument("Unknown CashInLimitEnum value");
    }

    inline std::string toString(CashAcceptorCapabilitiesClass::CountActionEnum value)
    {
        switch (value)
        {
        case CashAcceptorCapabilitiesClass::CountActionEnum::NotSupported: return "notSupported";
        case CashAcceptorCapabilitiesClass::CountActionEnum::Individual:   return "individual";
        case CashAcceptorCapabilitiesClass::CountActionEnum::All:          return "all";
        }

        throw std::invalid_argument("Unknown CountActionEnum value");
    }

    inline std::string toString(CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum value)
    {
        switch (value)
        {
        case CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum::NotSupported: return "notSupported";
        case CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum::Level2:       return "level2";
        case CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum::Level3:       return "level3";
        case CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum::Inked:        return "inked";
        }

        throw std::invalid_argument("Unknown RetainCounterfeitActionEnum value");
    }

    inline std::string toString(CashAcceptorCapabilitiesClass::PositionClass::UsageEnum value)
    {
        switch (value)
        {
        case CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::NotSupported: return "notSupported";
        case CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::In:           return "in";
        case CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::Refuse:       return "refuse";
        case CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::Rollback:     return "rollback";
        }

        throw std::invalid_argument("Unknown PositionClass::UsageEnum value");
    }

    inline std::string toString(CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum value)
    {
        switch (value)
        {
        case CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::NotSupported: return "notSupported";
        case CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::Retract:      return "retract";
        case CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::Reject:       return "reject";
        case CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::Transport:    return "transport";
        case CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::Stacker:      return "stacker";
        case CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::BillCassettes:return "billCassettes";
        case CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::CashIn:       return "cashIn";
        case CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::ItemCassette: return "itemCassette";
        }

        throw std::invalid_argument("Unknown PositionClass::RetractAreaEnum value");
    }

    inline void to_json(nlohmann::json& j, const CashAcceptorCapabilitiesClass::CashInLimitEnum& value)
    {
        j = toString(value);
    }

    inline void to_json(nlohmann::json& j, const CashAcceptorCapabilitiesClass::CountActionEnum& value)
    {
        j = toString(value);
    }

    inline void to_json(nlohmann::json& j, const CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum& value)
    {
        j = toString(value);
    }

    inline void to_json(nlohmann::json& j, const CashAcceptorCapabilitiesClass::PositionClass::UsageEnum& value)
    {
        j = toString(value);
    }

    inline void to_json(nlohmann::json& j, const CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum& value)
    {
        j = toString(value);
    }

    inline void to_json(nlohmann::json& j, const CashAcceptorCapabilitiesClass::PositionClass& p)
    {
        j = nlohmann::json{
            {"position", PositionEnumToString(p.GetPosition())},
            {"usage", static_cast<uint8_t>(p.GetUsage())},
            {"shutterControl", p.GetShutterControl()},
            {"itemsTakenSensor", p.GetItemsTakenSensor()},
            {"itemsInsertedSensor", p.GetItemsInsertedSensor()},
            {"presentControl", p.GetPresentControl()},
            {"preparePresent", p.GetPreparePresent()},
            {"retractArea", static_cast<uint8_t>(p.GetRetractArea())}
        };
    }

    inline void to_json(nlohmann::json& j, const CashAcceptorCapabilitiesClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetType()) j["type"] = toString(*p.GetType());
        if (p.GetMaxCashInItems()) j["maxCashInItems"] = *p.GetMaxCashInItems();
        if (p.GetShutter()) j["shutter"] = *p.GetShutter();
        if (p.GetShutterControl()) j["shutterControl"] = *p.GetShutterControl();
        if (p.GetIntermediateStacker()) j["intermediateStacker"] = *p.GetIntermediateStacker();
        if (p.GetItemsTakenSensor()) j["itemsTakenSensor"] = *p.GetItemsTakenSensor();

        if (p.GetPositions())
        {
            nlohmann::json positionsJson = nlohmann::json::object();

            for (const auto& [position, positionCaps] : *p.GetPositions())
            {
                positionsJson[PositionEnumToString(position)] = positionCaps;
            }

            j["positions"] = std::move(positionsJson);
        }

        if (p.GetRetractAreas()) j["retractAreas"] = static_cast<uint8_t>(*p.GetRetractAreas());
        if (p.GetRetractTransportActions()) j["retractTransportActions"] = static_cast<uint8_t>(*p.GetRetractTransportActions());
        if (p.GetRetractStackerActions()) j["retractStackerActions"] = static_cast<uint8_t>(*p.GetRetractStackerActions());
        if (p.GetCashInLimit()) j["cashInLimit"] = static_cast<uint8_t>(*p.GetCashInLimit());
        if (p.GetCountActions()) j["countActions"] = static_cast<uint8_t>(*p.GetCountActions());
        if (p.GetRetainCounterfeitAction()) j["retainCounterfeitAction"] = static_cast<uint8_t>(*p.GetRetainCounterfeitAction());
    }

    inline CashAcceptorCapabilitiesClass::CashInLimitEnum operator|(
        CashAcceptorCapabilitiesClass::CashInLimitEnum a,
        CashAcceptorCapabilitiesClass::CashInLimitEnum b)
    {
        return static_cast<CashAcceptorCapabilitiesClass::CashInLimitEnum>(
            static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    inline CashAcceptorCapabilitiesClass::CashInLimitEnum operator&(
        CashAcceptorCapabilitiesClass::CashInLimitEnum a,
        CashAcceptorCapabilitiesClass::CashInLimitEnum b)
    {
        return static_cast<CashAcceptorCapabilitiesClass::CashInLimitEnum>(
            static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }

    inline CashAcceptorCapabilitiesClass::CountActionEnum operator|(
        CashAcceptorCapabilitiesClass::CountActionEnum a,
        CashAcceptorCapabilitiesClass::CountActionEnum b)
    {
        return static_cast<CashAcceptorCapabilitiesClass::CountActionEnum>(
            static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    inline CashAcceptorCapabilitiesClass::CountActionEnum operator&(
        CashAcceptorCapabilitiesClass::CountActionEnum a,
        CashAcceptorCapabilitiesClass::CountActionEnum b)
    {
        return static_cast<CashAcceptorCapabilitiesClass::CountActionEnum>(
            static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }

    inline CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum operator|(
        CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum a,
        CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum b)
    {
        return static_cast<CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum>(
            static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    inline CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum operator&(
        CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum a,
        CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum b)
    {
        return static_cast<CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum>(
            static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }

    inline CashAcceptorCapabilitiesClass::PositionClass::UsageEnum operator|(
        CashAcceptorCapabilitiesClass::PositionClass::UsageEnum a,
        CashAcceptorCapabilitiesClass::PositionClass::UsageEnum b)
    {
        return static_cast<CashAcceptorCapabilitiesClass::PositionClass::UsageEnum>(
            static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    inline CashAcceptorCapabilitiesClass::PositionClass::UsageEnum operator&(
        CashAcceptorCapabilitiesClass::PositionClass::UsageEnum a,
        CashAcceptorCapabilitiesClass::PositionClass::UsageEnum b)
    {
        return static_cast<CashAcceptorCapabilitiesClass::PositionClass::UsageEnum>(
            static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }

    inline CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum operator|(
        CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum a,
        CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum b)
    {
        return static_cast<CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum>(
            static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    inline CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum operator&(
        CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum a,
        CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum b)
    {
        return static_cast<CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum>(
            static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }
}