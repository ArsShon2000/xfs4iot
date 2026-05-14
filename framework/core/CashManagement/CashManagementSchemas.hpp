#pragma once

#include <optional>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "../MessageBase.hpp"
#include <regex>

//namespace XFS4IoT::CashManagement
//{
    // Forward declarations
    //class StorageCashCountClass;
    //class StorageCashCountsClass;
    //class StorageCashInClass;
    //class StorageCashOutClass;
    //class StorageCashItemTypesClass;
    //class StorageCashTypesClass;
    //class StorageCashCapabilitiesClass;
    //class StorageCashConfigurationClass;
    //class StorageCashStatusClass;
    //class StorageCashClass;

//}


namespace XFS4IoT::CashManagement
{
    class StorageCashCountClass;
    class StorageCashCountsClass;
    class StorageCashInClass;
    class StorageCashOutClass;
    class StorageCashItemTypesClass;
    class StorageCashTypesClass;
    class StorageCashCapabilitiesClass;
    class StorageCashConfigurationClass;
    class StorageCashStatusClass;
    class StorageCashClass;

    /// <summary>
        /// Common output shutter position
        /// </summary>
    enum class OutputPositionEnum : uint16_t
    {
        NotSupported = 0,
        outDefault = 1 << 0,
        outLeft = 1 << 1,
        outRight = 1 << 2,
        outCenter = 1 << 3,
        outTop = 1 << 4,
        outBottom = 1 << 5,
        outFront = 1 << 6,
        outRear = 1 << 7,
    };

    inline std::string toString(OutputPositionEnum value)
    {
        switch (value)
        {
        case OutputPositionEnum::NotSupported: return "notSupported";
        case OutputPositionEnum::outDefault:      return "outDefault";
        case OutputPositionEnum::outLeft:         return "outLeft";
        case OutputPositionEnum::outRight:        return "outRight";
        case OutputPositionEnum::outCenter:       return "outCenter";
        case OutputPositionEnum::outTop:          return "outTop";
        case OutputPositionEnum::outBottom:       return "outBottom";
        case OutputPositionEnum::outFront:        return "outFront";
        case OutputPositionEnum::outRear:         return "outRear";
        }

        throw std::invalid_argument("Unknown OutputPositionEnum value");
    }

    inline OutputPositionEnum fromStringToOutputPositionEnum(std::string str)
    {
        if (str == "notSupported") return OutputPositionEnum::NotSupported;
        if (str == "outDefault")      return OutputPositionEnum::outDefault;
        if (str == "outLeft")         return OutputPositionEnum::outLeft;
        if (str == "outRight")        return OutputPositionEnum::outRight;
        if (str == "outCenter")       return OutputPositionEnum::outCenter;
        if (str == "outTop")          return OutputPositionEnum::outTop;
        if (str == "outBottom")       return OutputPositionEnum::outBottom;
        if (str == "outFront")        return OutputPositionEnum::outFront;
        if (str == "outRear")         return OutputPositionEnum::outRear;
        throw std::invalid_argument("Unknown string value for OutputPositionEnum: " + std::string(str));
	}

    inline void to_json(nlohmann::json& j, const OutputPositionEnum& value)
    {
        j = toString(value);
    }


    /// <summary>
    /// Common shutter position - prefix 'Out' is an output position and 'In' is an input position
    /// </summary>
    enum class PositionEnum : uint64_t
    {
        InDefault,
        InLeft,
        InRight,
        InCenter,
        InTop,
        InBottom,
        InFront,
        InRear,
        OutDefault,
        OutLeft,
        OutRight,
        OutCenter,
        OutTop,
        OutBottom,
        OutFront,
        OutRear
    };

    inline std::string ToString(PositionEnum pos)
    {
        switch (pos)
        {
        case PositionEnum::InDefault:
            return "inDefault";
        case PositionEnum::InLeft:
            return "inLeft";
        case PositionEnum::InRight:
            return "inRight";
        case PositionEnum::OutDefault:
            return "outDefault";
        case PositionEnum::OutLeft:
            return "outLeft";
        case PositionEnum::OutRight:
            return "outRight";
        case PositionEnum::OutCenter:
            return "outCenter";
        case PositionEnum::OutTop:
            return "outTop";
        case PositionEnum::OutBottom:
            return "outBottom";
        case PositionEnum::OutFront:
            return "outFront";
        case PositionEnum::OutRear:
            return "outRear";
        case PositionEnum::InCenter:
            return "inCenter";
        case PositionEnum::InTop:
            return "inTop";
        case PositionEnum::InBottom:
            return "inBottom";
        case PositionEnum::InFront:
            return "inFront";
        case PositionEnum::InRear:
            return "inRear";
        default:
            throw std::invalid_argument("Unknown PositionEnum value");
        }
    }

    enum class InputPositionEnum
    {
        NotSupported = 0,
        InDefault = 1 << 0,
        InLeft = 1 << 1,
        InRight = 1 << 2,
        InCenter = 1 << 3,
        InTop = 1 << 4,
        InBottom = 1 << 5,
        InFront = 1 << 6,
        InRear = 1 << 7
    };

    inline std::string toString(InputPositionEnum value)
    {
        switch (value)
        {
        case InputPositionEnum::NotSupported: return "notSupported";
        case InputPositionEnum::InDefault:    return "inDefault";
        case InputPositionEnum::InLeft:       return "inLeft";
        case InputPositionEnum::InRight:      return "inRight";
        case InputPositionEnum::InCenter:     return "inCenter";
        case InputPositionEnum::InTop:        return "inTop";
        case InputPositionEnum::InBottom:     return "inBottom";
        case InputPositionEnum::InFront:      return "inFront";
        case InputPositionEnum::InRear:       return "inRear";
        }

        throw std::invalid_argument("Unknown InputPositionEnum value");
    }

    inline InputPositionEnum fromStringToInputPositionEnum(std::string str)
    {
        if (str == "notSupported") return InputPositionEnum::NotSupported;
        if (str == "inDefault")    return InputPositionEnum::InDefault;
        if (str == "inLeft")       return InputPositionEnum::InLeft;
        if (str == "inRight")      return InputPositionEnum::InRight;
        if (str == "inCenter")     return InputPositionEnum::InCenter;
        if (str == "inTop")        return InputPositionEnum::InTop;
        if (str == "inBottom")     return InputPositionEnum::InBottom;
        if (str == "inFront")      return InputPositionEnum::InFront;
        if (str == "inRear")       return InputPositionEnum::InRear;
        throw std::invalid_argument("Unknown string value for InputPositionEnum: " + std::string(str));
	}

    inline void to_json(nlohmann::json& j, const InputPositionEnum& value)
    {
        j = toString(value);
    }

    enum class NoteLevelEnum
    {
        Unrecognized,
        Counterfeit,
        Suspect,
        Fit,
        Unfit,
        Inked
    };

    inline std::string toString(NoteLevelEnum value)
    {
        switch (value)
        {
        case NoteLevelEnum::Unrecognized: return "unrecognized";
        case NoteLevelEnum::Counterfeit: return "counterfeit";
        case NoteLevelEnum::Suspect: return "suspect";
        case NoteLevelEnum::Fit: return "fit";
        case NoteLevelEnum::Unfit: return "unfit";
        case NoteLevelEnum::Inked: return "inked";
        }

        throw std::invalid_argument("Unknown ItemTargetEnumEnum value");
    }

    inline void to_json(nlohmann::json& j, const NoteLevelEnum& value)
    {
        j = toString(value);
    }

    class CapabilitiesClass final
    {
    public:
        class ExchangeTypeClass final
        {
        public:
            explicit ExchangeTypeClass(std::optional<bool> byHand = std::nullopt)
                : byHand_(byHand)
            {
            }

            const std::optional<bool>& GetByHand() const noexcept { return byHand_; }

        private:
            std::optional<bool> byHand_;
        };

        class ItemInfoTypesClass final
        {
        public:
            ItemInfoTypesClass(
                std::optional<bool> serialNumber = std::nullopt,
                std::optional<bool> signature = std::nullopt,
                std::optional<bool> image = std::nullopt)
                : serialNumber_(serialNumber), signature_(signature), image_(image)
            {
            }

            const std::optional<bool>& GetSerialNumber() const noexcept { return serialNumber_; }
            const std::optional<bool>& GetSignature() const noexcept { return signature_; }
            const std::optional<bool>& GetImage() const noexcept { return image_; }

        private:
            std::optional<bool> serialNumber_;
            std::optional<bool> signature_;
            std::optional<bool> image_;
        };

        class ClassificationsClass final
        {
        public:
            ClassificationsClass(
                std::optional<bool> unrecognized = std::nullopt,
                std::optional<bool> counterfeit = std::nullopt,
                std::optional<bool> suspect = std::nullopt,
                std::optional<bool> inked = std::nullopt,
                std::optional<bool> fit = std::nullopt,
                std::optional<bool> unfit = std::nullopt)
                : unrecognized_(unrecognized), counterfeit_(counterfeit), suspect_(suspect),
                inked_(inked), fit_(fit), unfit_(unfit)
            {
            }

            const std::optional<bool>& GetUnrecognized() const noexcept { return unrecognized_; }
            const std::optional<bool>& GetCounterfeit() const noexcept { return counterfeit_; }
            const std::optional<bool>& GetSuspect() const noexcept { return suspect_; }
            const std::optional<bool>& GetInked() const noexcept { return inked_; }
            const std::optional<bool>& GetFit() const noexcept { return fit_; }
            const std::optional<bool>& GetUnfit() const noexcept { return unfit_; }

        private:
            std::optional<bool> unrecognized_;
            std::optional<bool> counterfeit_;
            std::optional<bool> suspect_;
            std::optional<bool> inked_;
            std::optional<bool> fit_;
            std::optional<bool> unfit_;
        };

        CapabilitiesClass(
            std::optional<bool> cashBox = std::nullopt,
            std::shared_ptr<ExchangeTypeClass> exchangeType = nullptr,
            std::shared_ptr<ItemInfoTypesClass> itemInfoTypes = nullptr,
            std::optional<bool> classificationList = std::nullopt,
            std::shared_ptr<ClassificationsClass> classifications = nullptr)
            : cashBox_(cashBox),
            exchangeType_(std::move(exchangeType)),
            itemInfoTypes_(std::move(itemInfoTypes)),
            classificationList_(classificationList),
            classifications_(std::move(classifications))
        {
        }

        const std::optional<bool>& GetCashBox() const noexcept { return cashBox_; }
        const std::shared_ptr<ExchangeTypeClass>& GetExchangeType() const noexcept { return exchangeType_; }
        const std::shared_ptr<ItemInfoTypesClass>& GetItemInfoTypes() const noexcept { return itemInfoTypes_; }
        const std::optional<bool>& GetClassificationList() const noexcept { return classificationList_; }
        const std::shared_ptr<ClassificationsClass>& GetClassifications() const noexcept { return classifications_; }

    private:
        std::optional<bool> cashBox_;
        std::shared_ptr<ExchangeTypeClass> exchangeType_;
        std::shared_ptr<ItemInfoTypesClass> itemInfoTypes_;
        std::optional<bool> classificationList_;
        std::shared_ptr<ClassificationsClass> classifications_;
    };

    inline void to_json(nlohmann::json& j, const CapabilitiesClass::ExchangeTypeClass& p)
    {
        j = nlohmann::json::object();
        if (p.GetByHand()) j["byHand"] = *p.GetByHand();
    }

    inline void to_json(nlohmann::json& j, const CapabilitiesClass::ItemInfoTypesClass& p)
    {
        j = nlohmann::json::object();
        if (p.GetSerialNumber()) j["serialNumber"] = *p.GetSerialNumber();
        if (p.GetSignature()) j["signature"] = *p.GetSignature();
        if (p.GetImage()) j["image"] = *p.GetImage();
    }

    inline void to_json(nlohmann::json& j, const CapabilitiesClass::ClassificationsClass& p)
    {
        j = nlohmann::json::object();
        if (p.GetUnrecognized()) j["unrecognized"] = *p.GetUnrecognized();
        if (p.GetCounterfeit()) j["counterfeit"] = *p.GetCounterfeit();
        if (p.GetSuspect()) j["suspect"] = *p.GetSuspect();
        if (p.GetInked()) j["inked"] = *p.GetInked();
        if (p.GetFit()) j["fit"] = *p.GetFit();
        if (p.GetUnfit()) j["unfit"] = *p.GetUnfit();
    }

    inline void to_json(nlohmann::json& j, const CapabilitiesClass& p)
    {
        j = nlohmann::json::object();
        if (p.GetCashBox()) j["cashBox"] = *p.GetCashBox();
        if (p.GetExchangeType()) j["exchangeType"] = *p.GetExchangeType();
        if (p.GetItemInfoTypes()) j["itemInfoTypes"] = *p.GetItemInfoTypes();
        if (p.GetClassificationList()) j["classificationList"] = *p.GetClassificationList();
        if (p.GetClassifications()) j["classifications"] = *p.GetClassifications();
    }


    class CashItemClass final
    {
    public:
        CashItemClass(
            std::optional<int> noteID = std::nullopt,
            std::optional<std::string> currency = std::nullopt,
            std::optional<double> value = std::nullopt,
            std::optional<int> release = std::nullopt)
            : noteID_(noteID)
            , currency_(std::move(currency))
            , value_(value)
            , release_(release)
        {
        }

        const std::optional<int>& GetNoteID() const noexcept
        {
            return noteID_;
        }

        void SetNoteID(std::optional<int> noteID)
        {
            noteID_ = noteID;
        }

        const std::optional<std::string>& GetCurrency() const noexcept
        {
            return currency_;
        }

        void SetCurrency(std::optional<std::string> currency)
        {
            currency_ = std::move(currency);
        }

        const std::optional<double>& GetValue() const noexcept
        {
            return value_;
        }

        void SetValue(std::optional<double> value)
        {
            value_ = value;
        }

        const std::optional<int>& GetRelease() const noexcept
        {
            return release_;
        }

        void SetRelease(std::optional<int> release)
        {
            release_ = release;
        }

    private:
        std::optional<int> noteID_;
        std::optional<std::string> currency_;
        std::optional<double> value_;
        std::optional<int> release_;
    };

    inline void to_json(nlohmann::json& j, const CashItemClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetNoteID().has_value())
        {
            j["noteID"] = p.GetNoteID().value();
        }

        if (p.GetCurrency().has_value())
        {
            j["currency"] = p.GetCurrency().value();
        }

        if (p.GetValue().has_value())
        {
            j["value"] = p.GetValue().value();
        }

        if (p.GetRelease().has_value())
        {
            j["release"] = p.GetRelease().value();
        }
    }


    class BankNoteClass final
    {
    public:
        BankNoteClass(
            std::shared_ptr<CashItemClass> cashItem = nullptr,
            std::optional<bool> enabled = std::nullopt)
            : cashItem_(std::move(cashItem))
            , enabled_(enabled)
        {
        }

        const std::shared_ptr<CashItemClass>& GetCashItem() const noexcept
        {
            return cashItem_;
        }

        void SetCashItem(std::shared_ptr<CashItemClass> cashItem)
        {
            cashItem_ = std::move(cashItem);
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
        std::shared_ptr<CashItemClass> cashItem_;
        std::optional<bool> enabled_;
    };

    inline void to_json(nlohmann::json& j, const BankNoteClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetCashItem())
        {
            j["cashItem"] = *p.GetCashItem();
        }

        if (p.GetEnabled().has_value())
        {
            j["enabled"] = p.GetEnabled().value();
        }
    }


    class StorageCashCountClass final
    {
    public:
        explicit StorageCashCountClass(
            std::optional<int> fit = std::nullopt,
            std::optional<int> unfit = std::nullopt,
            std::optional<int> suspect = std::nullopt,
            std::optional<int> counterfeit = std::nullopt,
            std::optional<int> inked = std::nullopt
        ) : fit_(fit)
            , unfit_(unfit)
            , suspect_(suspect)
            , counterfeit_(counterfeit)
            , inked_(inked)
        {
        }


        /// <summary>
        /// Number of cash retract operations which resulted in items entering this storage unit.
        /// </summary>
        [[nodiscard]] const std::optional<int>& fit() const noexcept { return fit_; }

        /// <summary>
        /// Number of cash retract operations which resulted in items entering this storage unit.
        /// </summary>
        [[nodiscard]] const std::optional<int>& unfit() const noexcept { return unfit_; }

        /// <summary>
        /// Number of cash retract operations which resulted in items entering this storage unit.
        /// </summary>
        [[nodiscard]] const std::optional<int>& suspect() const noexcept { return suspect_; }
        /// <summary>
        /// Number of cash retract operations which resulted in items entering this storage unit.
        /// </summary>
        [[nodiscard]] const std::optional<int>& counterfeit() const noexcept { return counterfeit_; }
        /// <summary>
        /// Number of cash retract operations which resulted in items entering this storage unit.
        /// </summary>
        [[nodiscard]] const std::optional<int>& inked() const noexcept { return inked_; }

    private:
        std::optional<int> fit_;
        std::optional<int> unfit_;
        std::optional<int> suspect_;
        std::optional<int> counterfeit_;
        std::optional<int> inked_;
    };

    inline void to_json(nlohmann::json& j, const StorageCashCountClass& r)
    {
        j = nlohmann::json::object();

        if (r.fit().has_value())
            j["fit"] = r.fit().value();

        if (r.unfit().has_value())
            j["unfit"] = r.unfit().value();

        if (r.suspect().has_value())
            j["suspect"] = r.suspect().value();

        if (r.counterfeit().has_value())
            j["counterfeit"] = r.counterfeit().value();

        if (r.inked().has_value())
            j["inked"] = r.inked().value();
    }

    class StatusClass final
    {
    public:
        enum class DispenserEnum
        {
            Ok,
            Attention,
            Stop,
            Unknown
        };

        enum class AcceptorEnum
        {
            Ok,
            Attention,
            Stop,
            Unknown
        };

        StatusClass(
            std::optional<DispenserEnum> dispenser = std::nullopt,
            std::optional<AcceptorEnum> acceptor = std::nullopt)
            : dispenser_(dispenser)
            , acceptor_(acceptor)
        {
        }

        const std::optional<DispenserEnum>& GetDispenser() const noexcept { return dispenser_; }
        const std::optional<AcceptorEnum>& GetAcceptor() const noexcept { return acceptor_; }

    private:
        std::optional<DispenserEnum> dispenser_;
        std::optional<AcceptorEnum> acceptor_;
    };

    inline void to_json(nlohmann::json& j, const StatusClass::DispenserEnum& p)
    {
        switch (p)
        {
        case StatusClass::DispenserEnum::Ok:        j = "ok"; break;
        case StatusClass::DispenserEnum::Attention: j = "attention"; break;
        case StatusClass::DispenserEnum::Stop:      j = "stop"; break;
        case StatusClass::DispenserEnum::Unknown:   j = "unknown"; break;
        }
    }

    inline void to_json(nlohmann::json& j, const StatusClass::AcceptorEnum& p)
    {
        switch (p)
        {
        case StatusClass::AcceptorEnum::Ok:        j = "ok"; break;
        case StatusClass::AcceptorEnum::Attention: j = "attention"; break;
        case StatusClass::AcceptorEnum::Stop:      j = "stop"; break;
        case StatusClass::AcceptorEnum::Unknown:   j = "unknown"; break;
        }
    }

    inline void to_json(nlohmann::json& j, const StatusClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetDispenser()) j["dispenser"] = *p.GetDispenser();
        if (p.GetAcceptor())  j["acceptor"] = *p.GetAcceptor();
    }

    /// <summary>
    /// StorageCashCountsClass - Cash item counts with extension data support
    /// </summary>
    class StorageCashCountsClass final
    {
    public:
        /// <summary>
        /// Constructor
        /// </summary>
        explicit StorageCashCountsClass(std::optional<int> unrecognized = std::nullopt)
            : unrecognized_(unrecognized)
            , extendedProperties_()
        {
        }

        /// <summary>
        /// Count of unrecognized items handled by the cash interface. May be null in command data and events if
        /// not changed or not to be changed.
        /// Minimum: 0
        /// </summary>
        std::optional<int> getUnrecognized() const { return unrecognized_; }
        void setUnrecognized(std::optional<int> unrecognized) { unrecognized_ = unrecognized; }

        /// <summary>
        /// Extended properties (denomination-specific counts)
        /// Key: denomination identifier (e.g., "typeEUR5", "typeUSD20")
        /// Value: StorageCashCountClass with fit/unfit/suspect/counterfeit/inked counts
        /// </summary>
        const std::unordered_map<std::string, std::shared_ptr<StorageCashCountClass>>&
            getExtendedProperties() const
        {
            return extendedProperties_;
        }

        void setExtendedProperties(
            std::unordered_map<std::string, std::shared_ptr<XFS4IoT::CashManagement::StorageCashCountClass>> extendedProperties)
        {
            extendedProperties_ = std::move(extendedProperties);
        }

        /// <summary>
        /// Add an extended property
        /// </summary>
        void addExtendedProperty(
            const std::string& key,
            std::shared_ptr<StorageCashCountClass> value)
        {
            extendedProperties_[key] = value;
        }

        /// <summary>
        /// Check if extended properties exist
        /// </summary>
        bool hasExtendedProperties() const
        {
            return !extendedProperties_.empty();
        }

        /// <summary>
        /// Get extended property by key
        /// </summary>
        std::shared_ptr<StorageCashCountClass> getExtendedProperty(const std::string& key) const
        {
            auto it = extendedProperties_.find(key);
            return (it != extendedProperties_.end()) ? it->second : nullptr;
        }

        /// <summary>
        /// Validate that unrecognized is >= 0
        /// </summary>
        bool validateUnrecognized() const
        {
            return !unrecognized_.has_value() || unrecognized_.value() >= 0;
        }



    private:
        std::optional<int> unrecognized_;
        std::unordered_map<std::string, std::shared_ptr<StorageCashCountClass>> extendedProperties_;
    };

    inline void to_json(
        nlohmann::json& j, const StorageCashCountsClass& r)
    {
        j = nlohmann::json::object();
        // Serialize unrecognized
        if (r.getUnrecognized().has_value())
        {
            j["unrecognized"] = r.getUnrecognized().value();
        }

        // Serialize extended properties
        for (const auto& [key, value] : r.getExtendedProperties())
        {
            if (value)
            {
                j[key] = *value;
            }
        }
    }

    //void fromJson(const std::string& jsonStr)
    //{
    //    json j = json::parse(jsonStr);

    //    // Parse unrecognized
    //    if (j.contains("unrecognized") && !j["unrecognized"].is_null())
    //    {
    //        int value = j["unrecognized"].get<int>();

    //        // Validate minimum
    //        if (value < 0)
    //        {
    //            throw std::invalid_argument("unrecognized must be >= 0");
    //        }

    //        unrecognized_ = value;
    //    }
    //    else
    //    {
    //        unrecognized_ = std::nullopt;
    //    }

    //    // Parse extended properties (all other keys)
    //    extendedProperties_.clear();
    //    for (auto it = j.begin(); it != j.end(); ++it)
    //    {
    //        // Skip known properties
    //        if (it.key() == "unrecognized")
    //        {
    //            continue;
    //        }

    //        // Parse as StorageCashCountClass
    //        auto countClass = std::make_shared<StorageCashCountClass>();
    //        countClass->fromJson(it.value().dump());
    //        extendedProperties_[it.key()] = countClass;
    //    }
    //}

    /// <summary>
    /// StorageCashInClass - Items moved into storage unit
    /// </summary>
    class StorageCashInClass final
    {
    public:
        explicit StorageCashInClass(
            std::optional<int> retract_operations = std::nullopt,
            std::shared_ptr<StorageCashCountsClass> deposited = nullptr,
            std::shared_ptr<StorageCashCountsClass> retracted = nullptr,
            std::shared_ptr<StorageCashCountsClass> rejected = nullptr,
            std::shared_ptr<StorageCashCountsClass> distributed = nullptr,
            std::shared_ptr<StorageCashCountsClass> transport = nullptr)
            : retract_operations_(retract_operations)
            , deposited_(deposited)
            , retracted_(retracted)
            , rejected_(rejected)
            , distributed_(distributed)
            , transport_(transport)
        {
        }

        /// <summary>
        /// Number of cash retract operations which resulted in items entering this storage unit.
        /// </summary>
        [[nodiscard]] const std::optional<int>& retract_operations() const noexcept { return retract_operations_; }

        /// <summary>
        /// The items deposited in the storage unit during a cash-in transaction.
        /// </summary>
        [[nodiscard]] const std::shared_ptr<StorageCashCountsClass>& deposited() const noexcept { return deposited_; }

        /// <summary>
        /// The items retracted into the storage unit after being accessible to a customer.
        /// </summary>
        [[nodiscard]] const std::shared_ptr<StorageCashCountsClass>& retracted() const noexcept { return retracted_; }

        /// <summary>
        /// The items deposited in this storage unit originating from another storage unit but rejected.
        /// </summary>
        [[nodiscard]] const std::shared_ptr<StorageCashCountsClass>& rejected() const noexcept { return rejected_; }

        /// <summary>
        /// The items deposited in this storage unit originating from another storage unit but not rejected.
        /// </summary>
        [[nodiscard]] const std::shared_ptr<StorageCashCountsClass>& distributed() const noexcept { return distributed_; }

        /// <summary>
        /// The items which were intended to be deposited in this storage unit but are not yet deposited.
        /// </summary>
        [[nodiscard]] const std::shared_ptr<StorageCashCountsClass>& transport() const noexcept { return transport_; }

    private:
        std::optional<int> retract_operations_;
        std::shared_ptr<StorageCashCountsClass> deposited_;
        std::shared_ptr<StorageCashCountsClass> retracted_;
        std::shared_ptr<StorageCashCountsClass> rejected_;
        std::shared_ptr<StorageCashCountsClass> distributed_;
        std::shared_ptr<StorageCashCountsClass> transport_;
    };

    inline void to_json(nlohmann::json& j, const StorageCashInClass& r)
    {
        j = nlohmann::json::object();

        if (r.retract_operations().has_value())
        {
            j["retractOperations"] = r.retract_operations().value();
        }

        if (r.deposited())
        {
            j["deposited"] = *r.deposited();
        }

        if (r.retracted())
        {
            j["retracted"] = *r.retracted();
        }

        if (r.rejected())
        {
            j["rejected"] = *r.rejected();
        }

        if (r.distributed())
        {
            j["distributed"] = *r.distributed();
        }

        if (r.transport())
        {
            j["transport"] = *r.transport();
        }
    }


    /// <summary>
    /// StorageCashOutClass - Items moved from storage unit
    /// </summary>
    class StorageCashOutClass final
    {
    public:
        explicit StorageCashOutClass(
            std::shared_ptr<StorageCashCountsClass> presented = nullptr,
            std::shared_ptr<StorageCashCountsClass> rejected = nullptr,
            std::shared_ptr<StorageCashCountsClass> distributed = nullptr,
            std::shared_ptr<StorageCashCountsClass> unknown = nullptr,
            std::shared_ptr<StorageCashCountsClass> stacked = nullptr,
            std::shared_ptr<StorageCashCountsClass> diverted = nullptr,
            std::shared_ptr<StorageCashCountsClass> transport = nullptr)
            : presented_(presented)
            , rejected_(rejected)
            , distributed_(distributed)
            , unknown_(unknown)
            , stacked_(stacked)
            , diverted_(diverted)
            , transport_(transport)
        {
        }

        /// <summary>
        /// The items dispensed from this storage unit which are or were customer accessible.
        /// </summary>
        [[nodiscard]] const std::shared_ptr<StorageCashCountsClass>& presented() const noexcept { return presented_; }

        /// <summary>
        /// The items dispensed from this storage unit which were invalid and diverted to reject.
        /// </summary>
        [[nodiscard]] const std::shared_ptr<StorageCashCountsClass>& rejected() const noexcept { return rejected_; }

        /// <summary>
        /// The items dispensed from this storage unit which were moved to another storage unit.
        /// </summary>
        [[nodiscard]] const std::shared_ptr<StorageCashCountsClass>& distributed() const noexcept { return distributed_; }

        /// <summary>
        /// The items dispensed from this storage unit which moved to an unknown position.
        /// </summary>
        [[nodiscard]] const std::shared_ptr<StorageCashCountsClass>& unknown() const noexcept { return unknown_; }

        /// <summary>
        /// The items dispensed from this storage unit which are stacked awaiting presentation.
        /// </summary>
        [[nodiscard]] const std::shared_ptr<StorageCashCountsClass>& stacked() const noexcept { return stacked_; }

        /// <summary>
        /// The items dispensed from this storage unit which were diverted to a temporary location.
        /// </summary>
        [[nodiscard]] const std::shared_ptr<StorageCashCountsClass>& diverted() const noexcept { return diverted_; }

        /// <summary>
        /// The items dispensed from this storage unit which have jammed in the transport.
        /// </summary>
        [[nodiscard]] const std::shared_ptr<StorageCashCountsClass>& transport() const noexcept { return transport_; }

    private:
        std::shared_ptr<StorageCashCountsClass> presented_;
        std::shared_ptr<StorageCashCountsClass> rejected_;
        std::shared_ptr<StorageCashCountsClass> distributed_;
        std::shared_ptr<StorageCashCountsClass> unknown_;
        std::shared_ptr<StorageCashCountsClass> stacked_;
        std::shared_ptr<StorageCashCountsClass> diverted_;
        std::shared_ptr<StorageCashCountsClass> transport_;
    };

    inline void to_json(nlohmann::json& j, const StorageCashOutClass& obj)
    {
        j = nlohmann::json::object();

        if (obj.presented())
            j["presented"] = *obj.presented();

        if (obj.rejected())
            j["rejected"] = *obj.rejected();

        if (obj.distributed())
            j["distributed"] = *obj.distributed();

        if (obj.unknown())
            j["unknown"] = *obj.unknown();

        if (obj.stacked())
            j["stacked"] = *obj.stacked();

        if (obj.diverted())
            j["diverted"] = *obj.diverted();

        if (obj.transport())
            j["transport"] = *obj.transport();
    }
    

    /// <summary>
    /// StorageCashItemTypesClass - Types of cash items that can be stored
    /// </summary>
    class StorageCashItemTypesClass final
    {
    public:
        explicit StorageCashItemTypesClass(
            std::optional<bool> fit = std::nullopt,
            std::optional<bool> unfit = std::nullopt,
            std::optional<bool> unrecognized = std::nullopt,
            std::optional<bool> counterfeit = std::nullopt,
            std::optional<bool> suspect = std::nullopt,
            std::optional<bool> inked = std::nullopt,
            std::optional<bool> coupon = std::nullopt,
            std::optional<bool> document = std::nullopt)
            : fit_(fit)
            , unfit_(unfit)
            , unrecognized_(unrecognized)
            , counterfeit_(counterfeit)
            , suspect_(suspect)
            , inked_(inked)
            , coupon_(coupon)
            , document_(document)
        {
        }

        /// <summary>
        /// The storage unit can store cash items which are fit for recycling.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& fit() const noexcept { return fit_; }

        /// <summary>
        /// The storage unit can store cash items which are unfit for recycling.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& unfit() const noexcept { return unfit_; }

        /// <summary>
        /// The storage unit can store unrecognized cash items.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& unrecognized() const noexcept { return unrecognized_; }

        /// <summary>
        /// The storage unit can store counterfeit cash items.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& counterfeit() const noexcept { return counterfeit_; }

        /// <summary>
        /// The storage unit can store suspect counterfeit cash items.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& suspect() const noexcept { return suspect_; }

        /// <summary>
        /// The storage unit can store cash items which have been identified as ink stained.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& inked() const noexcept { return inked_; }

        /// <summary>
        /// Storage unit containing coupons or advertising material.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& coupon() const noexcept { return coupon_; }

        /// <summary>
        /// Storage unit containing documents.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& document() const noexcept { return document_; }

    private:
        std::optional<bool> fit_;
        std::optional<bool> unfit_;
        std::optional<bool> unrecognized_;
        std::optional<bool> counterfeit_;
        std::optional<bool> suspect_;
        std::optional<bool> inked_;
        std::optional<bool> coupon_;
        std::optional<bool> document_;
    };

    inline void to_json(nlohmann::json& j, const StorageCashItemTypesClass& obj)
    {
        j = nlohmann::json::object();

        if (obj.fit().has_value())
            j["fit"] = obj.fit().value();

        if (obj.unfit().has_value())
            j["unfit"] = obj.unfit().value();

        if (obj.unrecognized().has_value())
            j["unrecognized"] = obj.unrecognized().value();

        if (obj.counterfeit().has_value())
            j["counterfeit"] = obj.counterfeit().value();

        if (obj.suspect().has_value())
            j["suspect"] = obj.suspect().value();

        if (obj.inked().has_value())
            j["inked"] = obj.inked().value();

        if (obj.coupon().has_value())
            j["coupon"] = obj.coupon().value();

        if (obj.document().has_value())
            j["document"] = obj.document().value();
    }

    /// <summary>
    /// StorageCashTypesClass - Types of operations supported by storage unit
    /// </summary>
    class StorageCashTypesClass final
    {
    public:
        explicit StorageCashTypesClass(
            std::optional<bool> cash_in = std::nullopt,
            std::optional<bool> cash_out = std::nullopt,
            std::optional<bool> replenishment = std::nullopt,
            std::optional<bool> cash_in_retract = std::nullopt,
            std::optional<bool> cash_out_retract = std::nullopt,
            std::optional<bool> reject = std::nullopt)
            : cash_in_(cash_in)
            , cash_out_(cash_out)
            , replenishment_(replenishment)
            , cash_in_retract_(cash_in_retract)
            , cash_out_retract_(cash_out_retract)
            , reject_(reject)
        {
        }

        /// <summary>
        /// The unit can accept cash items. If cash_out is also true, then the unit can recycle.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& cash_in() const noexcept { return cash_in_; }

        /// <summary>
        /// The unit can dispense cash items. If cash_in is also true, then the unit can recycle.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& cash_out() const noexcept { return cash_out_; }

        /// <summary>
        /// Replenishment container.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& replenishment() const noexcept { return replenishment_; }

        /// <summary>
        /// Retract unit. Items can be retracted into this unit during cash-in operations.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& cash_in_retract() const noexcept { return cash_in_retract_; }

        /// <summary>
        /// Retract unit. Items can be retracted into this unit during cash-out operations.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& cash_out_retract() const noexcept { return cash_out_retract_; }

        /// <summary>
        /// Reject unit. Items can be rejected into this unit.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& reject() const noexcept { return reject_; }

    private:
        std::optional<bool> cash_in_;
        std::optional<bool> cash_out_;
        std::optional<bool> replenishment_;
        std::optional<bool> cash_in_retract_;
        std::optional<bool> cash_out_retract_;
        std::optional<bool> reject_;
    };

    inline void to_json(nlohmann::json& j, const StorageCashTypesClass& obj)
    {
        j = nlohmann::json::object();

        if (obj.cash_in().has_value())
            j["cashIn"] = obj.cash_in().value();

        if (obj.cash_out().has_value())
            j["cashOut"] = obj.cash_out().value();

        if (obj.replenishment().has_value())
            j["replenishment"] = obj.replenishment().value();

        if (obj.cash_in_retract().has_value())
            j["cashInRetract"] = obj.cash_in_retract().value();

        if (obj.cash_out_retract().has_value())
            j["cashOutRetract"] = obj.cash_out_retract().value();

        if (obj.reject().has_value())
            j["reject"] = obj.reject().value();
    }


    /// <summary>
    /// StorageCashCapabilitiesClass - Capabilities of cash storage unit
    /// </summary>
    class StorageCashCapabilitiesClass final
    {
    public:
        explicit StorageCashCapabilitiesClass(
            std::shared_ptr<StorageCashTypesClass> types = nullptr,
            std::shared_ptr<StorageCashItemTypesClass> items = nullptr,
            std::optional<bool> hardware_sensors = std::nullopt,
            std::optional<int> retract_areas = std::nullopt,
            std::optional<bool> retract_thresholds = std::nullopt,
            std::optional<std::vector<std::string>> cash_items = std::nullopt)
            : types_(types)
            , items_(items)
            , hardware_sensors_(hardware_sensors)
            , retract_areas_(retract_areas)
            , retract_thresholds_(retract_thresholds)
            , cash_items_(cash_items)
        {
        }

        [[nodiscard]] const std::shared_ptr<StorageCashTypesClass>& types() const noexcept { return types_; }
        [[nodiscard]] const std::shared_ptr<StorageCashItemTypesClass>& items() const noexcept { return items_; }

        /// <summary>
        /// Indicates whether the storage unit has sensors which report the status.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& hardware_sensors() const noexcept { return hardware_sensors_; }

        /// <summary>
        /// Number of areas within the storage unit which allow physical separation of different bunches.
        /// </summary>
        [[nodiscard]] const std::optional<int>& retract_areas() const noexcept { return retract_areas_; }

        /// <summary>
        /// If true, retract capacity is based on counts. If false, based on number of commands.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& retract_thresholds() const noexcept { return retract_thresholds_; }

        /// <summary>
        /// Array containing multiple cash items the storage unit is capable of or configured to handle.
        /// </summary>
        [[nodiscard]] const std::optional<std::vector<std::string>>& cash_items() const noexcept { return cash_items_; }

    private:
        std::shared_ptr<StorageCashTypesClass> types_;
        std::shared_ptr<StorageCashItemTypesClass> items_;
        std::optional<bool> hardware_sensors_;
        std::optional<int> retract_areas_;
        std::optional<bool> retract_thresholds_;
        std::optional<std::vector<std::string>> cash_items_;
    };

    inline void to_json(nlohmann::json& j, const StorageCashCapabilitiesClass& obj)
    {
        j = nlohmann::json::object();

        if (obj.types())
            j["types"] = *obj.types();

        if (obj.items())
            j["items"] = *obj.items();

        if (obj.hardware_sensors().has_value())
            j["hardwareSensors"] = obj.hardware_sensors().value();

        if (obj.retract_areas().has_value())
            j["retractAreas"] = obj.retract_areas().value();

        if (obj.retract_thresholds().has_value())
            j["retractThresholds"] = obj.retract_thresholds().value();

        if (obj.cash_items().has_value())
            j["cashItems"] = obj.cash_items().value();
    }

    /// <summary>
    /// StorageCashConfigurationClass - Configuration of cash storage unit
    /// </summary>
    class StorageCashConfigurationClass final
    {
    public:
        explicit StorageCashConfigurationClass(
            std::shared_ptr<StorageCashTypesClass> types = nullptr,
            std::shared_ptr<StorageCashItemTypesClass> items = nullptr,
            std::optional<std::string> currency = std::nullopt,
            std::optional<double> value = std::nullopt,
            std::optional<int> high_threshold = std::nullopt,
            std::optional<int> low_threshold = std::nullopt,
            std::optional<bool> app_lock_in = std::nullopt,
            std::optional<bool> app_lock_out = std::nullopt,
            std::optional<std::vector<std::string>> cash_items = std::nullopt,
            std::optional<std::string> name = std::nullopt,
            std::optional<int> max_retracts = std::nullopt)
            : types_(types)
            , items_(items)
            , currency_(currency)
            , value_(value)
            , high_threshold_(high_threshold)
            , low_threshold_(low_threshold)
            , app_lock_in_(app_lock_in)
            , app_lock_out_(app_lock_out)
            , cash_items_(cash_items)
            , name_(name)
            , max_retracts_(max_retracts)
        {
        }

        [[nodiscard]] const std::shared_ptr<StorageCashTypesClass>& types() const noexcept { return types_; }
        [[nodiscard]] const std::shared_ptr<StorageCashItemTypesClass>& items() const noexcept { return items_; }

        /// <summary>
        /// ISO 4217 currency identifier. Pattern: ^[A-Z]{3}$
        /// </summary>
        [[nodiscard]] const std::optional<std::string>& currency() const noexcept { return currency_; }

        /// <summary>
        /// Absolute value of a cash item or items.
        /// </summary>
        [[nodiscard]] const std::optional<double>& value() const noexcept { return value_; }

        /// <summary>
        /// High threshold for replenishment status.
        /// </summary>
        [[nodiscard]] const std::optional<int>& high_threshold() const noexcept { return high_threshold_; }

        /// <summary>
        /// Low threshold for replenishment status.
        /// </summary>
        [[nodiscard]] const std::optional<int>& low_threshold() const noexcept { return low_threshold_; }

        /// <summary>
        /// If true, items cannot be accepted into the storage unit in cash-in operations.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& app_lock_in() const noexcept { return app_lock_in_; }

        /// <summary>
        /// If true, items cannot be dispensed from the storage unit in cash-out operations.
        /// </summary>
        [[nodiscard]] const std::optional<bool>& app_lock_out() const noexcept { return app_lock_out_; }

        /// <summary>
        /// Array containing multiple cash items the storage unit is capable of or configured to handle.
        /// </summary>
        [[nodiscard]] const std::optional<std::vector<std::string>>& cash_items() const noexcept { return cash_items_; }

        /// <summary>
        /// Application configured name of the unit.
        /// </summary>
        [[nodiscard]] const std::optional<std::string>& name() const noexcept { return name_; }

        /// <summary>
        /// Maximum number of retract operations allowed into the unit.
        /// </summary>
        [[nodiscard]] const std::optional<int>& max_retracts() const noexcept { return max_retracts_; }

    private:
        std::shared_ptr<StorageCashTypesClass> types_;
        std::shared_ptr<StorageCashItemTypesClass> items_;
        std::optional<std::string> currency_;
        std::optional<double> value_;
        std::optional<int> high_threshold_;
        std::optional<int> low_threshold_;
        std::optional<bool> app_lock_in_;
        std::optional<bool> app_lock_out_;
        std::optional<std::vector<std::string>> cash_items_;
        std::optional<std::string> name_;
        std::optional<int> max_retracts_;
    };

    inline void to_json(nlohmann::json& j, const StorageCashConfigurationClass& obj)
    {
        j = nlohmann::json::object();

        if (obj.types())
            j["types"] = *obj.types();

        if (obj.items())
            j["items"] = *obj.items();

        if (obj.currency().has_value())
            j["currency"] = obj.currency().value();

        if (obj.value().has_value())
            j["value"] = obj.value().value();

        if (obj.high_threshold().has_value())
            j["highThreshold"] = obj.high_threshold().value();

        if (obj.low_threshold().has_value())
            j["lowThreshold"] = obj.low_threshold().value();

        if (obj.app_lock_in().has_value())
            j["appLockIn"] = obj.app_lock_in().value();

        if (obj.app_lock_out().has_value())
            j["appLockOut"] = obj.app_lock_out().value();

        if (obj.cash_items().has_value())
            j["cashItems"] = obj.cash_items().value();

        if (obj.name().has_value())
            j["name"] = obj.name().value();

        if (obj.max_retracts().has_value())
            j["maxRetracts"] = obj.max_retracts().value();
    }

    /// <summary>
    /// StorageCashStatusClass - Status of cash storage unit
    /// </summary>
    class StorageCashStatusClass final
    {
    public:
        enum class AccuracyEnum
        {
            Accurate,
            AccurateSet,
            Inaccurate,
            Unknown
        };

        enum class ReplenishmentStatusEnum
        {
            Ok,
            Full,
            High,
            Low,
            Empty
        };

        enum class OperationStatusEnum
        {
            DispenseInoperative,
            DepositInoperative
        };

        explicit StorageCashStatusClass(
            std::optional<int> index = std::nullopt,
            std::shared_ptr<StorageCashCountsClass> initial = nullptr,
            std::shared_ptr<StorageCashOutClass> out = nullptr,
            std::shared_ptr<StorageCashInClass> in = nullptr,
            std::optional<AccuracyEnum> accuracy = std::nullopt,
            std::optional<ReplenishmentStatusEnum> replenishment_status = std::nullopt,
            std::optional<OperationStatusEnum> operation_status = std::nullopt)
            : index_(index)
            , initial_(initial)
            , out_(out)
            , in_(in)
            , accuracy_(accuracy)
            , replenishment_status_(replenishment_status)
            , operation_status_(operation_status)
        {
        }

        /// <summary>
        /// Assigned by the Service. Unique number which can be used to determine usNumber in XFS 3.x.
        /// </summary>
        [[nodiscard]] const std::optional<int>& index() const noexcept { return index_; }

        /// <summary>
        /// The cash related items which were in the storage unit at the last replenishment.
        /// </summary>
        [[nodiscard]] const std::shared_ptr<StorageCashCountsClass>& initial() const noexcept { return initial_; }

        /// <summary>
        /// The items moved from this storage unit by cash commands to another destination.
        /// </summary>
        [[nodiscard]] const std::shared_ptr<StorageCashOutClass>& out() const noexcept { return out_; }

        /// <summary>
        /// List of items inserted in this storage unit by cash commands from another source.
        /// </summary>
        [[nodiscard]] const std::shared_ptr<StorageCashInClass>& in() const noexcept { return in_; }

        /// <summary>
        /// Describes the accuracy of the counts reported by out and in.
        /// </summary>
        [[nodiscard]] const std::optional<AccuracyEnum>& accuracy() const noexcept { return accuracy_; }

        [[nodiscard]] const std::optional<ReplenishmentStatusEnum>& replenishment_status() const noexcept { return replenishment_status_; }

        /// <summary>
        /// On some devices it may be possible to allow items to be dispensed while deposit is inoperable.
        /// </summary>
        [[nodiscard]] const std::optional<OperationStatusEnum>& operation_status() const noexcept { return operation_status_; }

    private:
        std::optional<int> index_;
        std::shared_ptr<StorageCashCountsClass> initial_;
        std::shared_ptr<StorageCashOutClass> out_;
        std::shared_ptr<StorageCashInClass> in_;
        std::optional<AccuracyEnum> accuracy_;
        std::optional<ReplenishmentStatusEnum> replenishment_status_;
        std::optional<OperationStatusEnum> operation_status_;
    };

    inline void to_json(nlohmann::json& j, const StorageCashStatusClass& obj)
    {
        j = nlohmann::json::object();

        if (obj.index().has_value())
            j["index"] = obj.index().value();

        if (obj.initial())
            j["initial"] = *obj.initial();

        if (obj.out())
            j["out"] = *obj.out();

        if (obj.in())
            j["in"] = *obj.in();

        if (obj.accuracy().has_value())
            j["accuracy"] = obj.accuracy().value();

        if (obj.replenishment_status().has_value())
            j["replenishmentStatus"] = obj.replenishment_status().value();

        if (obj.operation_status().has_value())
            j["operationStatus"] = obj.operation_status().value();
    }


    inline std::string ToString(StorageCashStatusClass::AccuracyEnum value)
    {
        switch (value)
        {
        case StorageCashStatusClass::AccuracyEnum::Accurate:    return "accurate";
        case StorageCashStatusClass::AccuracyEnum::AccurateSet: return "accurateSet";
        case StorageCashStatusClass::AccuracyEnum::Inaccurate:  return "inaccurate";
        case StorageCashStatusClass::AccuracyEnum::Unknown:     return "unknown";
        }

        throw std::invalid_argument("Unknown StorageCashStatusClass::AccuracyEnum value");
    }

    inline void to_json(nlohmann::json& j, const StorageCashStatusClass::AccuracyEnum& value)
    {
        j = ToString(value);
    }

    //*****************************************************************************

    inline std::string ToString(StorageCashStatusClass::ReplenishmentStatusEnum value)
    {
        switch (value)
        {
        case StorageCashStatusClass::ReplenishmentStatusEnum::Ok:    return "ok";
        case StorageCashStatusClass::ReplenishmentStatusEnum::Full:  return "full";
        case StorageCashStatusClass::ReplenishmentStatusEnum::High:  return "high";
        case StorageCashStatusClass::ReplenishmentStatusEnum::Low:   return "low";
        case StorageCashStatusClass::ReplenishmentStatusEnum::Empty: return "empty";
        }

        throw std::invalid_argument("Unknown StorageCashStatusClass::ReplenishmentStatusEnum value");
    }

    inline void to_json(nlohmann::json& j, const StorageCashStatusClass::ReplenishmentStatusEnum& value)
    {
        j = ToString(value);
    }

    //*****************************************************************************

    inline std::string ToString(StorageCashStatusClass::OperationStatusEnum value)
    {
        switch (value)
        {
        case StorageCashStatusClass::OperationStatusEnum::DispenseInoperative: return "dispenseInoperative";
        case StorageCashStatusClass::OperationStatusEnum::DepositInoperative:  return "depositInoperative";
        }

        throw std::invalid_argument("Unknown StorageCashStatusClass::OperationStatusEnum value");
    }

    inline void to_json(nlohmann::json& j, const StorageCashStatusClass::OperationStatusEnum& value)
    {
        j = ToString(value);
    }

    //*****************************************************************************

    /// <summary>
    /// StorageCashClass - Complete cash storage unit information
    /// </summary>
    class StorageCashClass final
    {
    public:
        explicit StorageCashClass(
            std::shared_ptr<StorageCashCapabilitiesClass> capabilities = nullptr,
            std::shared_ptr<StorageCashConfigurationClass> configuration = nullptr,
            std::shared_ptr<StorageCashStatusClass> status = nullptr)
            : capabilities_(capabilities)
            , configuration_(configuration)
            , status_(status)
        {
        }

        [[nodiscard]] const std::shared_ptr<StorageCashCapabilitiesClass>& capabilities() const noexcept { return capabilities_; }
        [[nodiscard]] const std::shared_ptr<StorageCashConfigurationClass>& configuration() const noexcept { return configuration_; }
        [[nodiscard]] const std::shared_ptr<StorageCashStatusClass>& status() const noexcept { return status_; }

    private:
        std::shared_ptr<StorageCashCapabilitiesClass> capabilities_;
        std::shared_ptr<StorageCashConfigurationClass> configuration_;
        std::shared_ptr<StorageCashStatusClass> status_;
    };

    inline void to_json(nlohmann::json& j, const StorageCashClass& obj)
    {
        j = nlohmann::json::object();

        if (obj.capabilities())
            j["capabilities"] = *obj.capabilities();

        if (obj.configuration())
            j["configuration"] = *obj.configuration();

        if (obj.status())
            j["status"] = *obj.status();
    }

	//---------------------------------------------------------

	class StorageSetCashStatusClass final
	{
	public:
		StorageSetCashStatusClass(StorageCashCountsClass initial)
			: initial_(initial)
		{
		}
		[[nodiscard]] const StorageCashCountsClass& initial() const noexcept { return initial_; }

        [[nodiscard]] StorageCashCountsClass& getInitial() noexcept { return initial_; }

	private:
		StorageCashCountsClass initial_;
		};




    /// <summary>
    /// StorageSetCashClass - Used for setting/updating cash storage unit information
    /// </summary>
    class StorageSetCashClass final
    {
    public:
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="configuration">Configuration data to set</param>
        /// <param name="status">Status data to set</param>
        explicit StorageSetCashClass(
            std::shared_ptr<StorageCashConfigurationClass> configuration = nullptr,
            std::shared_ptr<StorageSetCashStatusClass> status = nullptr)
            : configuration_(configuration)
            , status_(status)
        {
        }

        /// <summary>
        /// Configuration data for the cash storage unit
        /// </summary>
        [[nodiscard]] const std::shared_ptr<StorageCashConfigurationClass>& configuration() const noexcept
        {
            return configuration_;
        }

        /// <summary>
        /// Status data for the cash storage unit
        /// </summary>
        [[nodiscard]] const std::shared_ptr<StorageSetCashStatusClass>& status() const noexcept
        {
            return status_;
        }

        // Setters (if mutable access is needed)
        void set_configuration(std::shared_ptr<StorageCashConfigurationClass> configuration)
        {
            configuration_ = configuration;
        }

        void set_status(std::shared_ptr<StorageSetCashStatusClass> status)
        {
            status_ = status;
        }

        virtual ~StorageSetCashClass() = default;

        // Copy and move semantics
        //StorageSetCashClass(const StorageSetCashClass&) = default;
        //StorageSetCashClass& operator=(const StorageSetCashClass&) = default;
        //StorageSetCashClass(StorageSetCashClass&&) noexcept = default;
        //StorageSetCashClass& operator=(StorageSetCashClass&&) noexcept = default;

    private:
        std::shared_ptr<StorageCashConfigurationClass> configuration_;
        std::shared_ptr<StorageSetCashStatusClass> status_;
    };

    /// <summary>
    /// Specifies the target location for items
    /// </summary>
    enum class ItemTargetEnumEnum
    {
        SingleUnit,
        Retract,
        Transport,
        Stacker,
        Reject,
        ItemCassette,
        CashIn,
        OutDefault,
        OutLeft,
        OutRight,
        OutCenter,
        OutTop,
        OutBottom,
        OutFront,
        OutRear
    };

    inline std::string toString(ItemTargetEnumEnum value)
    {
        switch (value)
        {
        case ItemTargetEnumEnum::SingleUnit: return "singleUnit";
        case ItemTargetEnumEnum::Retract:    return "retract";
        case ItemTargetEnumEnum::Transport:  return "transport";
        case ItemTargetEnumEnum::Stacker:    return "stacker";
        case ItemTargetEnumEnum::Reject:     return "reject";
        case ItemTargetEnumEnum::ItemCassette:return "itemCassette";
        case ItemTargetEnumEnum::CashIn:     return "cashIn";
        case ItemTargetEnumEnum::OutDefault: return "outDefault";
        case ItemTargetEnumEnum::OutLeft:    return "outLeft";
        case ItemTargetEnumEnum::OutRight:   return "outRight";
        case ItemTargetEnumEnum::OutCenter:  return "outCenter";
        case ItemTargetEnumEnum::OutTop:     return "outTop";
        case ItemTargetEnumEnum::OutBottom:  return "outBottom";
        case ItemTargetEnumEnum::OutFront:   return "outFront";
        case ItemTargetEnumEnum::OutRear:    return "outRear";
        }

        throw std::invalid_argument("Unknown ItemTargetEnumEnum value");
    }

    inline void to_json(nlohmann::json& j, const ItemTargetEnumEnum& value)
    {
        j = toString(value);
    }

    inline void from_json(const nlohmann::json& j, ItemTargetEnumEnum& value)
    {
        const auto s = j.get<std::string>();

        if (s == "singleUnit") value = ItemTargetEnumEnum::SingleUnit;
        else if (s == "retract") value = ItemTargetEnumEnum::Retract;
        else if (s == "transport") value = ItemTargetEnumEnum::Transport;
        else if (s == "stacker") value = ItemTargetEnumEnum::Stacker;
        else if (s == "itemCassette") value = ItemTargetEnumEnum::ItemCassette;
        else if (s == "cashIn") value = ItemTargetEnumEnum::CashIn;
        else if (s == "reject") value = ItemTargetEnumEnum::Reject;
        else if (s == "outDefault") value = ItemTargetEnumEnum::OutDefault;
        else if (s == "outLeft") value = ItemTargetEnumEnum::OutLeft;
        else if (s == "outRight") value = ItemTargetEnumEnum::OutRight;
        else if (s == "outCenter") value = ItemTargetEnumEnum::OutCenter;
        else if (s == "outTop") value = ItemTargetEnumEnum::OutTop;
        else if (s == "outBottom") value = ItemTargetEnumEnum::OutBottom;
        else if (s == "outFront") value = ItemTargetEnumEnum::OutFront;
        else if (s == "outRear") value = ItemTargetEnumEnum::OutRear;
        else
            throw std::invalid_argument("Unknown ItemTargetEnumEnum value: " + s);
    }

    /// <summary>
    /// Defines where items have been moved to or should be moved to
    /// </summary>
    class ItemTargetDataClass final
    {
    public:
        /// <summary>
        /// Constructor
        /// </summary>
        explicit ItemTargetDataClass(
            std::optional<ItemTargetEnumEnum> target = std::nullopt,
            std::optional<std::string> unit = std::nullopt,
            std::optional<int> index = std::nullopt)
            : target_(target)
            , unit_(std::move(unit))
            , index_(index)
        {
        }

        /// <summary>
        /// This property specifies the target. Following values are possible:
        /// 
        /// * singleUnit - A single storage unit defined by *unit*.
        /// * retract - A retract storage unit defined by *index*.
        /// * transport - The transport.
        /// * stacker - Intermediate stacker area.
        /// * reject - Reject storage unit.
        /// * itemCassette - Storage units which would be used during a cash-in transaction including recycling storage units.
        /// * cashIn - Storage units which would be used during a cash-in transaction but not including recycling storage units.
        /// * outDefault - Default output position.
        /// * outLeft - Left output position.
        /// * outRight - Right output position.
        /// * outCenter - Center output position.
        /// * outTop - Top output position.
        /// * outBottom - Bottom output position.
        /// * outFront - Front output position.
        /// * outRear - Rear output position.
        /// </summary>
        std::optional<ItemTargetEnumEnum> getTarget() const { return target_; }
        void setTarget(std::optional<ItemTargetEnumEnum> target) { target_ = target; }

        /// <summary>
        /// If *target* is set to *singleUnit*, this property specifies the object name (as stated by the
        /// Storage.GetStorage command) of a single storage unit. Ignored and may be null
        /// for all other cases.
        /// Pattern: ^unit[0-9A-Za-z]+$
        /// </summary>
        const std::optional<std::string>& getUnit() const { return unit_; }
        void setUnit(std::optional<std::string> unit) { unit_ = std::move(unit); }

        /// <summary>
        /// If *target* is set to *retract* this property defines a position inside the retract storage units. *index*
        /// starts with a value of 1 for the first retract position and increments by one for each subsequent position.
        /// Minimum: 1
        /// </summary>
        std::optional<int> getIndex() const { return index_; }
        void setIndex(std::optional<int> index) { index_ = index; }

        /// <summary>
        /// Validate that unit matches pattern: ^unit[0-9A-Za-z]+$
        /// </summary>
        bool validateUnit() const
        {
            if (!unit_.has_value())
                return true;

            static const std::regex unitPattern("^unit[0-9A-Za-z]+$");
            return std::regex_match(unit_.value(), unitPattern);
        }

        /// <summary>
        /// Validate that index is >= 1
        /// </summary>
        bool validateIndex() const
        {
            return !index_.has_value() || index_.value() >= 1;
        }

        /// <summary>
        /// Validate all constraints
        /// </summary>
        bool validate() const
        {
            return validateUnit() && validateIndex();
        }

        /// <summary>
        /// Check if target requires a unit to be specified
        /// </summary>
        bool requiresUnit() const
        {
            return target_.has_value() &&
                target_.value() == ItemTargetEnumEnum::SingleUnit;
        }

        /// <summary>
        /// Check if target requires an index to be specified
        /// </summary>
        bool requiresIndex() const
        {
            return target_.has_value() &&
                target_.value() == ItemTargetEnumEnum::Retract;
        }

        // Serialization support

        void fromJson(const std::string& json);

        // Equality operators
        bool operator==(const ItemTargetDataClass& other) const
        {
            return target_ == other.target_ &&
                unit_ == other.unit_ &&
                index_ == other.index_;
        }

        bool operator!=(const ItemTargetDataClass& other) const
        {
            return !(*this == other);
        }

    private:
        std::optional<ItemTargetEnumEnum> target_;
        std::optional<std::string> unit_;
        std::optional<int> index_;
    };

    inline void to_json(nlohmann::json& j, const ItemTargetDataClass& obj)
    {
        j = nlohmann::json::object();

        if (obj.getTarget().has_value())
            j["target"] = obj.getTarget().value();

        if (obj.getUnit().has_value())
            j["unit"] = obj.getUnit().value();

        if (obj.getIndex().has_value())
            j["index"] = obj.getIndex().value();
    }


	// ---------------------------------------------------------


    class TellerTotalsClass final
    {
    public:
        explicit TellerTotalsClass(
            std::optional<double> itemsReceived = std::nullopt,
            std::optional<double> itemsDispensed = std::nullopt,
            std::optional<double> coinsReceived = std::nullopt,
            std::optional<double> coinsDispensed = std::nullopt,
            std::optional<double> cashBoxReceived = std::nullopt,
            std::optional<double> cashBoxDispensed = std::nullopt
            ) 
            : itemsReceived_(itemsReceived)
            , itemsDispensed_(itemsDispensed)
            , coinsReceived_(coinsReceived)
            , coinsDispensed_(coinsDispensed)
            , cashBoxReceived_(cashBoxReceived)
			, cashBoxDispensed_(cashBoxDispensed)
        {
        }

        /// <summary>
       /// The total absolute value of items (other than coins) of the specified currency accepted.
       /// The amount is expressed as a floating-point value.
       /// <example>1405.00</example>
       /// </summary>
        [[nodiscard]] const std::optional<double>& getItemsReceived() const noexcept { return itemsReceived_; }

            /// <summary>
            /// The total absolute value of items (other than coins) of the specified currency dispensed.
            /// The amount is expressed as a floating-point value.
            /// <example>1405.00</example>
            /// </summary>
        [[nodiscard]] const std::optional<double>& getItemsDispensed() const noexcept { return itemsDispensed_; }

            /// <summary>
            /// The total absolute value of coin currency accepted.
            /// The amount is expressed as a floating-point value.
            /// <example>0.05</example>
            /// </summary>
        [[nodiscard]] const std::optional<double>& getCoinsReceived() const noexcept { return coinsReceived_; }

            /// <summary>
            /// The total absolute value of coin currency dispensed.
            /// The amount is expressed as a floating-point value.
            /// <example>0.05</example>
            /// </summary>
        [[nodiscard]] const std::optional<double>& getCoinsDispensed() const noexcept { return coinsDispensed_; }

            /// <summary>
            /// The total absolute value of cash box currency accepted.
            /// The amount is expressed as a floating-point value.
            /// <example>1407.15</example>
            /// </summary>
        [[nodiscard]] const std::optional<double>& getCashBoxReceived() const noexcept { return cashBoxReceived_; }

            /// <summary>
            /// The total absolute value of cash box currency dispensed.
            /// The amount is expressed as a floating-point value.
            /// <example>1407.15</example>
            /// </summary>
        [[nodiscard]] const std::optional<double>& getCashBoxDispensed() const noexcept { return cashBoxDispensed_; }

    private:
        std::optional<double> itemsReceived_;
        std::optional<double> itemsDispensed_;
        std::optional<double> coinsReceived_;
        std::optional<double> coinsDispensed_;
        std::optional<double> cashBoxReceived_;
        std::optional<double> cashBoxDispensed_;
    };

    inline void to_json(nlohmann::json& j, const TellerTotalsClass& obj)
    {
        j = nlohmann::json::object();

        if (obj.getItemsReceived().has_value())
            j["itemsReceived"] = obj.getItemsReceived().value();

        if (obj.getItemsDispensed().has_value())
            j["itemsDispensed"] = obj.getItemsDispensed().value();

        if (obj.getCoinsReceived().has_value())
            j["coinsReceived"] = obj.getCoinsReceived().value();

        if (obj.getCoinsDispensed().has_value())
            j["coinsDispensed"] = obj.getCoinsDispensed().value();

        if (obj.getCashBoxReceived().has_value())
            j["cashBoxReceived"] = obj.getCashBoxReceived().value();

        if (obj.getCashBoxDispensed().has_value())
            j["cashBoxDispensed"] = obj.getCashBoxDispensed().value();
    }



	// ---------------------------------------------------------

    class TellerDetailsClass final
    {
    public:
        enum class AccuracyEnum
        {
            Accurate,
            AccurateSet,
            Inaccurate,
            Unknown
        };

        enum class ReplenishmentStatusEnum
        {
            Ok,
            Full,
            High,
            Low,
            Empty
        };

        enum class OperationStatusEnum
        {
            DispenseInoperative,
            DepositInoperative
        };

        explicit TellerDetailsClass(
            std::optional<int> tellerID = std::nullopt,
            std::optional<InputPositionEnum> inputPosition = std::nullopt,
            std::optional<OutputPositionEnum> outputPosition = std::nullopt,
            std::unordered_map<std::string, TellerTotalsClass> tellerTotals = {})
			: tellerID_(tellerID)
			, inputPosition_(inputPosition)
			, outputPosition_(outputPosition)
			, tellerTotals_(std::move(tellerTotals))
        {
        }

        /// <summary>
        /// Identification of the teller.
        /// <example>104</example>
        /// </summary>
        [[nodiscard]] const std::optional<int>& getTellerID() const noexcept { return tellerID_; }


		/// <summary>
        /// The input position of the teller.
		/// </summary>
		[[nodiscard]] const std::optional<InputPositionEnum>& getInputPosition() const noexcept { return inputPosition_; }

        /// <summary>
        /// The output position of the teller.
        /// </summary>
        [[nodiscard]] const std::optional<OutputPositionEnum>& getOutputPosition() const noexcept { return outputPosition_; }
        /// <summary>
        /// A map containing totals for the teller. The key is a string which identifies the type of total, e.g. "currentDay", "previousDay", etc.
        /// </summary>
		[[nodiscard]] const std::unordered_map<std::string, TellerTotalsClass>& getTellerTotals() const noexcept { return tellerTotals_; }

        

    private:
        std::optional<int> tellerID_;
		std::optional<InputPositionEnum> inputPosition_;
        std::optional<OutputPositionEnum> outputPosition_;
		std::unordered_map<std::string, TellerTotalsClass> tellerTotals_;
    };

    inline void to_json(nlohmann::json& j, const TellerDetailsClass& obj)
    {
        j = nlohmann::json::object();

        if (obj.getTellerID().has_value())
            j["tellerID"] = obj.getTellerID().value();

        if (obj.getInputPosition().has_value())
            j["inputPosition"] = obj.getInputPosition().value();

        if (obj.getOutputPosition().has_value())
            j["outputPosition"] = obj.getOutputPosition().value();

        if (!obj.getTellerTotals().empty())
        {
            j["tellerTotals"] = nlohmann::json::object();

            for (const auto& [key, value] : obj.getTellerTotals())
            {
                j["tellerTotals"][key] = value;
            }
        }
    }


    // Helper functions for ItemTargetEnumEnum serialization
    inline std::string itemTargetEnumToString(ItemTargetEnumEnum target)
    {
        switch (target)
        {
        case ItemTargetEnumEnum::SingleUnit: return "singleUnit";
        case ItemTargetEnumEnum::Retract: return "retract";
        case ItemTargetEnumEnum::Transport: return "transport";
        case ItemTargetEnumEnum::Stacker: return "stacker";
        case ItemTargetEnumEnum::Reject: return "reject";
        case ItemTargetEnumEnum::ItemCassette: return "itemCassette";
        case ItemTargetEnumEnum::CashIn: return "cashIn";
        case ItemTargetEnumEnum::OutDefault: return "outDefault";
        case ItemTargetEnumEnum::OutLeft: return "outLeft";
        case ItemTargetEnumEnum::OutRight: return "outRight";
        case ItemTargetEnumEnum::OutCenter: return "outCenter";
        case ItemTargetEnumEnum::OutTop: return "outTop";
        case ItemTargetEnumEnum::OutBottom: return "outBottom";
        case ItemTargetEnumEnum::OutFront: return "outFront";
        case ItemTargetEnumEnum::OutRear: return "outRear";
        default:
            throw std::invalid_argument("Unknown ItemTargetEnumEnum value");
        }
    }

    inline ItemTargetEnumEnum stringToItemTargetEnum(const std::string& str)
    {
        if (str == "singleUnit") return ItemTargetEnumEnum::SingleUnit;
        if (str == "retract") return ItemTargetEnumEnum::Retract;
        if (str == "transport") return ItemTargetEnumEnum::Transport;
        if (str == "stacker") return ItemTargetEnumEnum::Stacker;
        if (str == "reject") return ItemTargetEnumEnum::Reject;
        if (str == "itemCassette") return ItemTargetEnumEnum::ItemCassette;
        if (str == "cashIn") return ItemTargetEnumEnum::CashIn;
        if (str == "outDefault") return ItemTargetEnumEnum::OutDefault;
        if (str == "outLeft") return ItemTargetEnumEnum::OutLeft;
        if (str == "outRight") return ItemTargetEnumEnum::OutRight;
        if (str == "outCenter") return ItemTargetEnumEnum::OutCenter;
        if (str == "outTop") return ItemTargetEnumEnum::OutTop;
        if (str == "outBottom") return ItemTargetEnumEnum::OutBottom;
        if (str == "outFront") return ItemTargetEnumEnum::OutFront;
        if (str == "outRear") return ItemTargetEnumEnum::OutRear;

        throw std::invalid_argument("Unknown ItemTargetEnumEnum string: " + str);
    }

    
} // namespace XFS4IoT::CashManagement