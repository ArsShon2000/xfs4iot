#pragma once

#include <map>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "../../core/Persistent/PersistentDatasHandler.hpp"

namespace XFS4IoTFramework::Common
{
    /// <summary>
    /// CashManagementCapabilities
	/// Это класс, который описывает возможности управления наличностью, которые поддерживает устройство. Он может включать в себя информацию о том, какие позиции для выдачи и приема наличности поддерживаются, какие типы обменов поддерживаются, какие типы информации о предметах поддерживаются, какие области для изъятия поддерживаются и какие действия для транспортировки и изъятия поддерживаются. Этот класс может быть использован для определения того, какие функции управления наличностью доступны на устройстве и как они могут быть использованы в приложениях, которые взаимодействуют с устройством.
    /// </summary>
    class CashManagementCapabilitiesClass final
    {
    public:
        // ---------------- ENUMS ----------------

        enum class OutputPositionEnum : uint16_t
        {
            NotSupported = 0,
            Default = 1 << 0,
            Left = 1 << 1,
            Right = 1 << 2,
            Center = 1 << 3,
            Top = 1 << 4,
            Bottom = 1 << 5,
            Front = 1 << 6,
            Rear = 1 << 7,
        };

        inline std::string toString(OutputPositionEnum value)
        {
            switch (value)
            {
            case OutputPositionEnum::NotSupported: return "notSupported";
            case OutputPositionEnum::Default:      return "default";
            case OutputPositionEnum::Left:         return "left";
            case OutputPositionEnum::Right:        return "right";
            case OutputPositionEnum::Center:       return "center";
            case OutputPositionEnum::Top:          return "top";
            case OutputPositionEnum::Bottom:       return "bottom";
            case OutputPositionEnum::Front:        return "front";
            case OutputPositionEnum::Rear:         return "rear";
            }

            throw std::invalid_argument("Unknown OutputPositionEnum value");
        }

        inline void to_json(nlohmann::json& j, const OutputPositionEnum& value)
        {
            j = toString(value);
        }

        enum class PositionEnum : uint64_t
        {
            OutDefault = 1 << 0,
            OutLeft = 1 << 1,
            OutRight = 1 << 2,
            OutCenter = 1 << 3,
            OutTop = 1 << 4,
            OutBottom = 1 << 5,
            OutFront = 1 << 6,
            OutRear = 1 << 7,
            InDefault = 1 << 8,
            InLeft = 1 << 9,
            InRight = 1 << 10,
            InCenter = 1 << 11,
            InTop = 1 << 12,
            InBottom = 1 << 13,
            InFront = 1 << 14,
            InRear = 1 << 15,
        };

        enum class ExchangeTypesEnum : uint8_t
        {
            NotSupported = 0,
            ByHand = 1 << 0,
        };

        inline std::string toString(ExchangeTypesEnum value)
        {
            switch (value)
            {
            case ExchangeTypesEnum::NotSupported: return "notSupported";
            case ExchangeTypesEnum::ByHand:      return "byHand";
            }

            throw std::invalid_argument("Неизвестное значение OutputPositionEnum");
        }

        inline void to_json(nlohmann::json& j, const ExchangeTypesEnum& value)
        {
            j = toString(value);
        }

        enum class ItemInfoTypesEnum : uint8_t
        {
            NotSupported = 0,
            SerialNumber = 1 << 0,
            Signature = 1 << 1,
            ImageFile = 1 << 2,
        };

        inline std::string toString(ItemInfoTypesEnum value)
        {
            switch (value)
            {
            case ItemInfoTypesEnum::NotSupported: return "notSupported";
            case ItemInfoTypesEnum::SerialNumber:      return "serialNumber";
            case ItemInfoTypesEnum::Signature:         return "signature";
            case ItemInfoTypesEnum::ImageFile:        return "imageFile";
            }

            throw std::invalid_argument("Неизвестное значение OutputPositionEnum");
        }

        inline void to_json(nlohmann::json& j, const ItemInfoTypesEnum& value)
        {
            j = toString(value);
        }

        enum class RetractAreaEnum : uint8_t
        {
            NotSupported = 0,
            Retract = 1 << 0,
            Transport = 1 << 1,
            Stacker = 1 << 2,
            Reject = 1 << 3,
            ItemCassette = 1 << 4,
            CashIn = 1 << 5,
        };

        inline std::string toString(RetractAreaEnum value)
        {
            switch (value)
            {
            case RetractAreaEnum::NotSupported: return "notSupported";
            case RetractAreaEnum::Retract: return "retract";
            case RetractAreaEnum::Transport: return "transport";
            case RetractAreaEnum::Stacker: return "stacker";
            case RetractAreaEnum::Reject: return "reject";
            case RetractAreaEnum::ItemCassette: return "itemCassette";
            case RetractAreaEnum::CashIn: return "cashIn";
            }
            throw std::invalid_argument("Неизвестное значение RetractAreaEnum");
		}

        inline void to_json(nlohmann::json& j, const RetractAreaEnum& value)
        {
            j = toString(value);
		}

        enum class RetractTransportActionEnum : uint8_t
        {
            NotSupported = 0,
            Present = 1 << 0,
            Retract = 1 << 1,
            Reject = 1 << 2,
            BillCassette = 1 << 3,
            CashIn = 1 << 4,
        };
        inline std::string toString(RetractTransportActionEnum value)
        {
            switch (value)
            {
            case RetractTransportActionEnum::NotSupported: return "notSupported";
            case RetractTransportActionEnum::Present:      return "present";
            case RetractTransportActionEnum::Retract:      return "retract";
            case RetractTransportActionEnum::Reject:       return "reject";
            case RetractTransportActionEnum::BillCassette: return "billCassette";
            case RetractTransportActionEnum::CashIn:       return "cashIn";
            }
            throw std::invalid_argument("Неизвестное значение RetractTransportActionEnum");
		}
        inline void to_json(nlohmann::json& j, const RetractTransportActionEnum& value)
        {
            j = toString(value);
        }

        enum class RetractStackerActionEnum : uint8_t
        {
            NotSupported = 0,
            Present = 1 << 0,
            Retract = 1 << 1,
            Reject = 1 << 2,
            BillCassette = 1 << 3,
            CashIn = 1 << 4,
        };
        inline std::string toString(RetractStackerActionEnum value)
        {
            switch (value)
            {
            case RetractStackerActionEnum::NotSupported: return "notSupported";
            case RetractStackerActionEnum::Present:      return "present";
            case RetractStackerActionEnum::Retract:      return "retract";
            case RetractStackerActionEnum::Reject:       return "reject";
            case RetractStackerActionEnum::BillCassette: return "billCassette";
            case RetractStackerActionEnum::CashIn:       return "cashIn";
            }
            throw std::invalid_argument("Неизвестное значение RetractStackerActionEnum");
        }
        inline void to_json(nlohmann::json& j, const RetractStackerActionEnum& value)
        {
            j = toString(value);
		}

        enum class TypeEnum
        {
            TellerBill,
            SelfServiceBill,
            TellerCoin,
            SelfServiceCoin
        };

        // ---------------- BANKNOTE ----------------

        class BanknoteItem final
        {
        public:
            BanknoteItem(
                int noteId,
                std::string currency,
                double value,
                int release,
                bool enabled)
                : noteId_(noteId)
                , currency_(std::move(currency))
                , value_(value)
                , release_(release)
                , enabled_(enabled)
            {
            }

            BanknoteItem()
                : noteId_(0)
                , currency_("")
                , value_(0)
                , release_(0)
                , enabled_(false)
            {
            }

            int GetNoteId() const noexcept { return noteId_; }
            const std::string& GetCurrency() const noexcept { return currency_; }
            double GetValue() const noexcept { return value_; }
            int GetRelease() const noexcept { return release_; }
            bool IsEnabled() const noexcept { return enabled_; }
            void SetEnabled(bool enabled) { enabled_ = enabled; }

        private:
            int noteId_;
            std::string currency_;
            double value_;
            int release_;
            bool enabled_;
        };

        // ---------------- CTOR ----------------

        CashManagementCapabilitiesClass(
            std::optional<PositionEnum> positions = std::nullopt,
            std::optional<bool> shutterControl = std::nullopt,
            std::optional<RetractAreaEnum> retractAreas = std::nullopt,
            std::optional<RetractTransportActionEnum> retractTransportActions = std::nullopt,
            std::optional<RetractStackerActionEnum> retractStackerActions = std::nullopt,
            std::optional<ExchangeTypesEnum> exchangeTypes = std::nullopt,
            std::optional<ItemInfoTypesEnum> itemInfoTypes = std::nullopt,
            std::optional<bool> cashBox = std::nullopt,
            std::optional<bool> classificationList = std::nullopt
            //, std::optional<std::map<std::string, BanknoteItem>> allBanknoteItems = std::nullopt
        )
            : positions_(positions)
            , shutterControl_(shutterControl)
            , retractAreas_(retractAreas)
            , retractTransportActions_(retractTransportActions)
            , retractStackerActions_(retractStackerActions)
            , exchangeTypes_(exchangeTypes)
            , itemInfoTypes_(itemInfoTypes)
            , cashBox_(cashBox)
            , classificationList_(classificationList)
            //, allBanknoteItems_(std::move(allBanknoteItems))
        {
            allBanknoteItems_ = loadPersistentNotes();
        }

        // ---------------- GETTERS ----------------

        const std::optional<PositionEnum>& GetPositions() const noexcept { return positions_; }
        const std::optional<bool>& GetShutterControl() const noexcept { return shutterControl_; }
        const std::optional<RetractAreaEnum>& GetRetractAreas() const noexcept { return retractAreas_; }
        const std::optional<RetractTransportActionEnum>& GetRetractTransportActions() const noexcept { return retractTransportActions_; }
        const std::optional<RetractStackerActionEnum>& GetRetractStackerActions() const noexcept { return retractStackerActions_; }
        const std::optional<ExchangeTypesEnum>& GetExchangeTypes() const noexcept { return exchangeTypes_; }
        const std::optional<ItemInfoTypesEnum>& GetItemInfoTypes() const noexcept { return itemInfoTypes_; }
        const std::optional<bool>& GetCashBox() const noexcept { return cashBox_; }
        const std::optional<bool>& GetClassificationList() const noexcept { return classificationList_; }
        const std::optional<std::map<std::string, BanknoteItem>>& GetAllBanknoteItems() const noexcept { return allBanknoteItems_; }
		const uint32_t GetBillTypes() const noexcept { return m_lBillTypes; }
        bool saveBanknotes()
        {
            if (allBanknoteItems_->empty() || !allBanknoteItems_)
                return false;

            SetBillTypes(); 

            nlohmann::json notes = nlohmann::json::object();
            for (const auto&[banknoteType, banknote] : *allBanknoteItems_)
            {
                notes[banknoteType] = {
                    {
                        "cashItem",
                        {
                            {"noteID", banknote.GetNoteId()},
                            {"currency", banknote.GetCurrency()},
                            {"value", banknote.GetValue()},
                            {"release", banknote.GetRelease()},
                        }
                    },
                    {
                        "enabled",  banknote.IsEnabled()
                    }
                };
            }

            return PersistentDatasHandler::GetInstance()->setConfiguredNotes(notes);
        }

    private:
        std::optional<PositionEnum> positions_;
        std::optional<bool> shutterControl_;
        std::optional<RetractAreaEnum> retractAreas_;
        std::optional<RetractTransportActionEnum> retractTransportActions_;
        std::optional<RetractStackerActionEnum> retractStackerActions_;
        std::optional<ExchangeTypesEnum> exchangeTypes_;
        std::optional<ItemInfoTypesEnum> itemInfoTypes_;
        std::optional<bool> cashBox_;
        std::optional<bool> classificationList_;
        std::optional<std::map<std::string, BanknoteItem>> allBanknoteItems_;
        /// Список купюр, которые распознает купюроприемник (Битовая маска для передачи в устройство)
        uint32_t m_lBillTypes;

        void SetBillTypes() noexcept
        {
            for (const auto&[banknoteType, banknote] : *allBanknoteItems_)
            {
                if (banknote.IsEnabled())
                {
                    m_lBillTypes |= (1 << banknote.GetNoteId());
                }
			}
		}
        std::optional<std::map<std::string, BanknoteItem>> loadPersistentNotes()
        {
            try
            {
                auto notesJson = PersistentDatasHandler::GetInstance()->getConfiguredNotes();


                if (!notesJson.is_object() || notesJson.empty())
                {
                    return std::nullopt;
                }

                std::map<std::string, BanknoteItem> result;

                for (const auto& [banknoteType, item] : notesJson.items())
                {
                    const auto& cashItem = item["cashItem"];

                    BanknoteItem banknote(
                        cashItem["noteID"].get<int>(),
                        cashItem["currency"].get<std::string>(),
                        cashItem["value"].get<double>(),
                        cashItem["release"].get<int>(),
                        item["enabled"].get<bool>()
                    );

                    result.emplace(banknoteType, std::move(banknote));
                }

                if (result.empty())
                {
                    return std::nullopt;
                }

                return result;
            }
            catch (const std::exception& e)
            {
                std::cout << std::format(
                            "{}(): failed to parse configured banknote items: {}",
                            __FUNCTION__,
                            e.what()) << std::endl;
                

                return std::nullopt;
            }
        }
    };

	// Ниже операторы для удобной работы с флагами в перечислениях
    inline CashManagementCapabilitiesClass::OutputPositionEnum operator|(
        CashManagementCapabilitiesClass::OutputPositionEnum a,
        CashManagementCapabilitiesClass::OutputPositionEnum b)
    {
        return static_cast<CashManagementCapabilitiesClass::OutputPositionEnum>(
            static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
    }

    inline CashManagementCapabilitiesClass::OutputPositionEnum operator&(
        CashManagementCapabilitiesClass::OutputPositionEnum a,
        CashManagementCapabilitiesClass::OutputPositionEnum b)
    {
        return static_cast<CashManagementCapabilitiesClass::OutputPositionEnum>(
            static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
    }

    inline CashManagementCapabilitiesClass::PositionEnum operator|(
        CashManagementCapabilitiesClass::PositionEnum a,
        CashManagementCapabilitiesClass::PositionEnum b)
    {
        return static_cast<CashManagementCapabilitiesClass::PositionEnum>(
            static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    inline CashManagementCapabilitiesClass::PositionEnum operator&(
        CashManagementCapabilitiesClass::PositionEnum a,
        CashManagementCapabilitiesClass::PositionEnum b)
    {
        return static_cast<CashManagementCapabilitiesClass::PositionEnum>(
            static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }

    inline CashManagementCapabilitiesClass::RetractAreaEnum operator|(
        CashManagementCapabilitiesClass::RetractAreaEnum a,
        CashManagementCapabilitiesClass::RetractAreaEnum b)
    {
        return static_cast<CashManagementCapabilitiesClass::RetractAreaEnum>(
            static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    inline CashManagementCapabilitiesClass::RetractAreaEnum operator&(
        CashManagementCapabilitiesClass::RetractAreaEnum a,
        CashManagementCapabilitiesClass::RetractAreaEnum b)
    {
        return static_cast<CashManagementCapabilitiesClass::RetractAreaEnum>(
            static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }


    static std::string PositionEnumToString(
        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position)
    {
        using PositionEnum =
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum;

        switch (position)
        {
        case PositionEnum::OutDefault:   return "OutDefault";
        case PositionEnum::OutLeft:      return "OutLeft";
        case PositionEnum::OutRight:     return "OutRight";
        case PositionEnum::OutCenter:    return "OutCenter";
        case PositionEnum::OutTop:       return "OutTop";
        case PositionEnum::OutBottom:    return "OutBottom";
        case PositionEnum::OutFront:     return "OutFront";
        case PositionEnum::OutRear:      return "OutRear";
        case PositionEnum::InDefault:    return "InDefault";
        case PositionEnum::InLeft:       return "InLeft";
        case PositionEnum::InRight:      return "InRight";
        case PositionEnum::InCenter:     return "InCenter";
        case PositionEnum::InTop:        return "InTop";
        case PositionEnum::InBottom:     return "InBottom";
        case PositionEnum::InFront:      return "InFront";
        case PositionEnum::InRear:       return "InRear";
        default:                         return "NotSupported";
        }
    }

    static XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum
        PositionEnumFromString(const std::string& value)
    {
        using PositionEnum =
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum;

        if (value == "OutDefault") return PositionEnum::OutDefault;
        if (value == "OutLeft")    return PositionEnum::OutLeft;
        if (value == "OutRight")   return PositionEnum::OutRight;
        if (value == "OutCenter")  return PositionEnum::OutCenter;
        if (value == "OutTop")     return PositionEnum::OutTop;
        if (value == "OutBottom")  return PositionEnum::OutBottom;
        if (value == "OutFront")   return PositionEnum::OutFront;
        if (value == "OutRear")    return PositionEnum::OutRear;
        if (value == "InDefault")  return PositionEnum::InDefault;
        if (value == "InLeft")     return PositionEnum::InLeft;
        if (value == "InRight")    return PositionEnum::InRight;
        if (value == "InCenter")   return PositionEnum::InCenter;
        if (value == "InTop")      return PositionEnum::InTop;
        if (value == "InBottom")   return PositionEnum::InBottom;
        if (value == "InFront")    return PositionEnum::InFront;
        if (value == "InRear")     return PositionEnum::InRear;

        return PositionEnum::OutDefault;
    }

    inline void to_json(nlohmann::json& j, const CashManagementCapabilitiesClass::PositionEnum& value)
    {
        j = PositionEnumToString(value);
    }

    inline void to_json(nlohmann::json& j, const CashManagementCapabilitiesClass::BanknoteItem& p)
    {
        j = nlohmann::json{
            {"noteId", p.GetNoteId()},
            {"currency", p.GetCurrency()},
            {"value", p.GetValue()},
            {"release", p.GetRelease()},
            {"enabled", p.IsEnabled()}
        };
    }

    inline void to_json(nlohmann::json& j, const CashManagementCapabilitiesClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetPositions()) j["positions"] = static_cast<uint32_t>(*p.GetPositions());
        if (p.GetShutterControl()) j["shutterControl"] = *p.GetShutterControl();
        if (p.GetRetractAreas()) j["retractAreas"] = static_cast<uint8_t>(*p.GetRetractAreas());
        if (p.GetRetractTransportActions()) j["retractTransportActions"] = static_cast<uint8_t>(*p.GetRetractTransportActions());
        if (p.GetRetractStackerActions()) j["retractStackerActions"] = static_cast<uint8_t>(*p.GetRetractStackerActions());
        if (p.GetExchangeTypes()) j["exchangeTypes"] = static_cast<uint8_t>(*p.GetExchangeTypes());
        if (p.GetItemInfoTypes()) j["itemInfoTypes"] = static_cast<uint8_t>(*p.GetItemInfoTypes());
        if (p.GetCashBox()) j["cashBox"] = *p.GetCashBox();
        if (p.GetClassificationList()) j["classificationList"] = *p.GetClassificationList();

        if (p.GetAllBanknoteItems())
            j["banknoteItems"] = *p.GetAllBanknoteItems();
    }


    inline std::string toString(CashManagementCapabilitiesClass::TypeEnum value)
    {
        switch (value)
        {
        case CashManagementCapabilitiesClass::TypeEnum::TellerBill: return "tellerBill";
        case CashManagementCapabilitiesClass::TypeEnum::SelfServiceBill: return "selfServiceBill";
        case CashManagementCapabilitiesClass::TypeEnum::TellerCoin: return "tellerCoin";
        case CashManagementCapabilitiesClass::TypeEnum::SelfServiceCoin: return "selfServiceCoin";
        }
        throw std::invalid_argument("Неизвестное значение TypeEnum");
    }

    inline void to_json(nlohmann::json& j, const CashManagementCapabilitiesClass::TypeEnum& value)
    {
        j = toString(value);
    }

}