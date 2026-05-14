#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <stdexcept>
#include <nlohmann/json.hpp>

#include "../CashManagement/CashManagementSchemas.hpp"
#include "../JsonHelpers.hpp" // если используешь PutOptional / PutPtr

namespace XFS4IoT::CashAcceptor
{
    class PositionClass final
    {
    public:
        enum class ShutterEnum
        {
            Closed,
            Open,
            JammedOpen,
            JammedPartiallyOpen,
            JammedClosed,
            JammedUnknown,
            Unknown
        };

        enum class PositionStatusEnum
        {
            Empty,
            NotEmpty,
            Unknown,
            ForeignItems
        };

        enum class TransportEnum
        {
            Ok,
            Inoperative,
            Unknown
        };

        enum class TransportStatusEnum
        {
            Empty,
            NotEmpty,
            NotEmptyCustomer,
            Unknown
        };

        PositionClass(
            std::optional<CashManagement::PositionEnum> position = std::nullopt,
            std::optional<ShutterEnum> shutter = std::nullopt,
            std::optional<PositionStatusEnum> positionStatus = std::nullopt,
            std::optional<TransportEnum> transport = std::nullopt,
            std::optional<TransportStatusEnum> transportStatus = std::nullopt)
            : position_(position)
            , shutter_(shutter)
            , positionStatus_(positionStatus)
            , transport_(transport)
            , transportStatus_(transportStatus)
        {
        }

        const std::optional<CashManagement::PositionEnum>& GetPosition() const noexcept { return position_; }
        const std::optional<ShutterEnum>& GetShutter() const noexcept { return shutter_; }
        const std::optional<PositionStatusEnum>& GetPositionStatus() const noexcept { return positionStatus_; }
        const std::optional<TransportEnum>& GetTransport() const noexcept { return transport_; }
        const std::optional<TransportStatusEnum>& GetTransportStatus() const noexcept { return transportStatus_; }

    private:
        std::optional<CashManagement::PositionEnum> position_;
        std::optional<ShutterEnum> shutter_;
        std::optional<PositionStatusEnum> positionStatus_;
        std::optional<TransportEnum> transport_;
        std::optional<TransportStatusEnum> transportStatus_;
    };

    class StatusClass final
    {
    public:
        enum class IntermediateStackerEnum
        {
            Empty,
            NotEmpty,
            Full,
            Unknown
        };

        enum class StackerItemsEnum
        {
            CustomerAccess,
            NoCustomerAccess,
            AccessUnknown,
            NoItems
        };

        enum class BanknoteReaderEnum
        {
            Ok,
            Inoperable,
            Unknown
        };

        StatusClass(
            std::optional<IntermediateStackerEnum> intermediateStacker = std::nullopt,
            std::optional<StackerItemsEnum> stackerItems = std::nullopt,
            std::optional<BanknoteReaderEnum> banknoteReader = std::nullopt,
            std::optional<bool> dropBox = std::nullopt,
            std::vector<std::shared_ptr<PositionClass>> positions = {})
            : intermediateStacker_(intermediateStacker)
            , stackerItems_(stackerItems)
            , banknoteReader_(banknoteReader)
            , dropBox_(dropBox)
            , positions_(std::move(positions))
        {
        }

        const std::optional<IntermediateStackerEnum>& GetIntermediateStacker() const noexcept { return intermediateStacker_; }
        const std::optional<StackerItemsEnum>& GetStackerItems() const noexcept { return stackerItems_; }
        const std::optional<BanknoteReaderEnum>& GetBanknoteReader() const noexcept { return banknoteReader_; }
        const std::optional<bool>& GetDropBox() const noexcept { return dropBox_; }
        const std::vector<std::shared_ptr<PositionClass>>& GetPositions() const noexcept { return positions_; }

    private:
        std::optional<IntermediateStackerEnum> intermediateStacker_;
        std::optional<StackerItemsEnum> stackerItems_;
        std::optional<BanknoteReaderEnum> banknoteReader_;
        std::optional<bool> dropBox_;
        std::vector<std::shared_ptr<PositionClass>> positions_;
    };

    // ------------------------
    // enum -> json
    // ------------------------

    inline void to_json(nlohmann::json& j, const PositionClass::ShutterEnum& p)
    {
        switch (p)
        {
        case PositionClass::ShutterEnum::Closed:              j = "closed"; break;
        case PositionClass::ShutterEnum::Open:                j = "open"; break;
        case PositionClass::ShutterEnum::JammedOpen:          j = "jammedOpen"; break;
        case PositionClass::ShutterEnum::JammedPartiallyOpen: j = "jammedPartiallyOpen"; break;
        case PositionClass::ShutterEnum::JammedClosed:        j = "jammedClosed"; break;
        case PositionClass::ShutterEnum::JammedUnknown:       j = "jammedUnknown"; break;
        case PositionClass::ShutterEnum::Unknown:             j = "unknown"; break;
        }
    }

    inline void to_json(nlohmann::json& j, const PositionClass::PositionStatusEnum& p)
    {
        switch (p)
        {
        case PositionClass::PositionStatusEnum::Empty:        j = "empty"; break;
        case PositionClass::PositionStatusEnum::NotEmpty:     j = "notEmpty"; break;
        case PositionClass::PositionStatusEnum::Unknown:      j = "unknown"; break;
        case PositionClass::PositionStatusEnum::ForeignItems: j = "foreignItems"; break;
        }
    }

    inline void to_json(nlohmann::json& j, const PositionClass::TransportEnum& p)
    {
        switch (p)
        {
        case PositionClass::TransportEnum::Ok:          j = "ok"; break;
        case PositionClass::TransportEnum::Inoperative: j = "inoperative"; break;
        case PositionClass::TransportEnum::Unknown:     j = "unknown"; break;
        }
    }

    inline void to_json(nlohmann::json& j, const PositionClass::TransportStatusEnum& p)
    {
        switch (p)
        {
        case PositionClass::TransportStatusEnum::Empty:            j = "empty"; break;
        case PositionClass::TransportStatusEnum::NotEmpty:         j = "notEmpty"; break;
        case PositionClass::TransportStatusEnum::NotEmptyCustomer: j = "notEmptyCustomer"; break;
        case PositionClass::TransportStatusEnum::Unknown:          j = "unknown"; break;
        }
    }

    inline void to_json(nlohmann::json& j, const StatusClass::IntermediateStackerEnum& p)
    {
        switch (p)
        {
        case StatusClass::IntermediateStackerEnum::Empty:    j = "empty"; break;
        case StatusClass::IntermediateStackerEnum::NotEmpty: j = "notEmpty"; break;
        case StatusClass::IntermediateStackerEnum::Full:     j = "full"; break;
        case StatusClass::IntermediateStackerEnum::Unknown:  j = "unknown"; break;
        }
    }

    inline void to_json(nlohmann::json& j, const StatusClass::StackerItemsEnum& p)
    {
        switch (p)
        {
        case StatusClass::StackerItemsEnum::CustomerAccess:   j = "customerAccess"; break;
        case StatusClass::StackerItemsEnum::NoCustomerAccess: j = "noCustomerAccess"; break;
        case StatusClass::StackerItemsEnum::AccessUnknown:    j = "accessUnknown"; break;
        case StatusClass::StackerItemsEnum::NoItems:          j = "noItems"; break;
        }
    }

    inline void to_json(nlohmann::json& j, const StatusClass::BanknoteReaderEnum& p)
    {
        switch (p)
        {
        case StatusClass::BanknoteReaderEnum::Ok:         j = "ok"; break;
        case StatusClass::BanknoteReaderEnum::Inoperable: j = "inoperable"; break;
        case StatusClass::BanknoteReaderEnum::Unknown:    j = "unknown"; break;
        }
    }

    // ------------------------
    // class -> json
    // ------------------------

    inline void to_json(nlohmann::json& j, const PositionClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetPosition())        j["position"] = *p.GetPosition();
        if (p.GetShutter())         j["shutter"] = *p.GetShutter();
        if (p.GetPositionStatus())  j["positionStatus"] = *p.GetPositionStatus();
        if (p.GetTransport())       j["transport"] = *p.GetTransport();
        if (p.GetTransportStatus()) j["transportStatus"] = *p.GetTransportStatus();
    }

    inline void to_json(nlohmann::json& j, const StatusClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetIntermediateStacker()) j["intermediateStacker"] = *p.GetIntermediateStacker();
        if (p.GetStackerItems())        j["stackerItems"] = *p.GetStackerItems();
        if (p.GetBanknoteReader())      j["banknoteReader"] = *p.GetBanknoteReader();
        if (p.GetDropBox())             j["dropBox"] = *p.GetDropBox();

        if (!p.GetPositions().empty())
        {
            j["positions"] = nlohmann::json::array();
            for (const auto& pos : p.GetPositions())
            {
                if (pos)
                    j["positions"].push_back(*pos);
            }
        }
    }

    class PosCapsClass final
    {
    public:
        class UsageClass final
        {
        public:
            UsageClass(
                std::optional<bool> in = std::nullopt,
                std::optional<bool> refuse = std::nullopt,
                std::optional<bool> rollback = std::nullopt)
                : in_(in), refuse_(refuse), rollback_(rollback)
            {
            }

            const std::optional<bool>& GetIn() const noexcept { return in_; }
            const std::optional<bool>& GetRefuse() const noexcept { return refuse_; }
            const std::optional<bool>& GetRollback() const noexcept { return rollback_; }

        private:
            std::optional<bool> in_;
            std::optional<bool> refuse_;
            std::optional<bool> rollback_;
        };

        class RetractAreasClass final
        {
        public:
            RetractAreasClass(
                std::optional<bool> retract = std::nullopt,
                std::optional<bool> reject = std::nullopt,
                std::optional<bool> transport = std::nullopt,
                std::optional<bool> stacker = std::nullopt,
                std::optional<bool> billCassettes = std::nullopt,
                std::optional<bool> cashIn = std::nullopt,
                std::optional<bool> itemCassette = std::nullopt)
                : retract_(retract), reject_(reject), transport_(transport), stacker_(stacker),
                billCassettes_(billCassettes), cashIn_(cashIn), itemCassette_(itemCassette)
            {
            }

            const std::optional<bool>& GetRetract() const noexcept { return retract_; }
            const std::optional<bool>& GetReject() const noexcept { return reject_; }
            const std::optional<bool>& GetTransport() const noexcept { return transport_; }
            const std::optional<bool>& GetStacker() const noexcept { return stacker_; }
            const std::optional<bool>& GetBillCassettes() const noexcept { return billCassettes_; }
            const std::optional<bool>& GetCashIn() const noexcept { return cashIn_; }
            const std::optional<bool>& GetItemCassette() const noexcept { return itemCassette_; }

        private:
            std::optional<bool> retract_;
            std::optional<bool> reject_;
            std::optional<bool> transport_;
            std::optional<bool> stacker_;
            std::optional<bool> billCassettes_;
            std::optional<bool> cashIn_;
            std::optional<bool> itemCassette_;
        };

        PosCapsClass(
            std::optional<CashManagement::PositionEnum> position = std::nullopt,
            std::shared_ptr<UsageClass> usage = nullptr,
            std::optional<bool> shutterControl = std::nullopt,
            std::optional<bool> itemsTakenSensor = std::nullopt,
            std::optional<bool> itemsInsertedSensor = std::nullopt,
            std::shared_ptr<RetractAreasClass> retractAreas = nullptr,
            std::optional<bool> presentControl = std::nullopt,
            std::optional<bool> preparePresent = std::nullopt)
            : position_(position), usage_(std::move(usage)), shutterControl_(shutterControl),
            itemsTakenSensor_(itemsTakenSensor), itemsInsertedSensor_(itemsInsertedSensor),
            retractAreas_(std::move(retractAreas)), presentControl_(presentControl),
            preparePresent_(preparePresent)
        {
        }

        const std::optional<CashManagement::PositionEnum>& GetPosition() const noexcept { return position_; }
        const std::shared_ptr<UsageClass>& GetUsage() const noexcept { return usage_; }
        const std::optional<bool>& GetShutterControl() const noexcept { return shutterControl_; }
        const std::optional<bool>& GetItemsTakenSensor() const noexcept { return itemsTakenSensor_; }
        const std::optional<bool>& GetItemsInsertedSensor() const noexcept { return itemsInsertedSensor_; }
        const std::shared_ptr<RetractAreasClass>& GetRetractAreas() const noexcept { return retractAreas_; }
        const std::optional<bool>& GetPresentControl() const noexcept { return presentControl_; }
        const std::optional<bool>& GetPreparePresent() const noexcept { return preparePresent_; }

    private:
        std::optional<CashManagement::PositionEnum> position_;
        std::shared_ptr<UsageClass> usage_;
        std::optional<bool> shutterControl_;
        std::optional<bool> itemsTakenSensor_;
        std::optional<bool> itemsInsertedSensor_;
        std::shared_ptr<RetractAreasClass> retractAreas_;
        std::optional<bool> presentControl_;
        std::optional<bool> preparePresent_;
    };

    class CapabilitiesClass final
    {
    public:
        enum class TypeEnum
        {
            TellerBill,
            SelfServiceBill,
            TellerCoin,
            SelfServiceCoin
        };

        class RetractAreasClass final
        {
        public:
            RetractAreasClass(
                std::optional<bool> retract = std::nullopt,
                std::optional<bool> transport = std::nullopt,
                std::optional<bool> stacker = std::nullopt,
                std::optional<bool> reject = std::nullopt,
                std::optional<bool> billCassette = std::nullopt,
                std::optional<bool> cashIn = std::nullopt,
                std::optional<bool> itemCassette = std::nullopt)
                : retract_(retract), transport_(transport), stacker_(stacker), reject_(reject),
                billCassette_(billCassette), cashIn_(cashIn), itemCassette_(itemCassette)
            {
            }

            const std::optional<bool>& GetRetract() const noexcept { return retract_; }
            const std::optional<bool>& GetTransport() const noexcept { return transport_; }
            const std::optional<bool>& GetStacker() const noexcept { return stacker_; }
            const std::optional<bool>& GetReject() const noexcept { return reject_; }
            const std::optional<bool>& GetBillCassette() const noexcept { return billCassette_; }
            const std::optional<bool>& GetCashIn() const noexcept { return cashIn_; }
            const std::optional<bool>& GetItemCassette() const noexcept { return itemCassette_; }

        private:
            std::optional<bool> retract_;
            std::optional<bool> transport_;
            std::optional<bool> stacker_;
            std::optional<bool> reject_;
            std::optional<bool> billCassette_;
            std::optional<bool> cashIn_;
            std::optional<bool> itemCassette_;
        };

        class RetractTransportActionsClass final
        {
        public:
            RetractTransportActionsClass(
                std::optional<bool> present = std::nullopt,
                std::optional<bool> retract = std::nullopt,
                std::optional<bool> reject = std::nullopt,
                std::optional<bool> billCassette = std::nullopt,
                std::optional<bool> cashIn = std::nullopt,
                std::optional<bool> itemCassette = std::nullopt)
                : present_(present), retract_(retract), reject_(reject),
                billCassette_(billCassette), cashIn_(cashIn), itemCassette_(itemCassette)
            {
            }

            const std::optional<bool>& GetPresent() const noexcept { return present_; }
            const std::optional<bool>& GetRetract() const noexcept { return retract_; }
            const std::optional<bool>& GetReject() const noexcept { return reject_; }
            const std::optional<bool>& GetBillCassette() const noexcept { return billCassette_; }
            const std::optional<bool>& GetCashIn() const noexcept { return cashIn_; }
            const std::optional<bool>& GetItemCassette() const noexcept { return itemCassette_; }

        private:
            std::optional<bool> present_;
            std::optional<bool> retract_;
            std::optional<bool> reject_;
            std::optional<bool> billCassette_;
            std::optional<bool> cashIn_;
            std::optional<bool> itemCassette_;
        };

        class RetractStackerActionsClass final
        {
        public:
            RetractStackerActionsClass(
                std::optional<bool> present = std::nullopt,
                std::optional<bool> retract = std::nullopt,
                std::optional<bool> reject = std::nullopt,
                std::optional<bool> billCassette = std::nullopt,
                std::optional<bool> cashIn = std::nullopt,
                std::optional<bool> itemCassette = std::nullopt)
                : present_(present), retract_(retract), reject_(reject),
                billCassette_(billCassette), cashIn_(cashIn), itemCassette_(itemCassette)
            {
            }

            const std::optional<bool>& GetPresent() const noexcept { return present_; }
            const std::optional<bool>& GetRetract() const noexcept { return retract_; }
            const std::optional<bool>& GetReject() const noexcept { return reject_; }
            const std::optional<bool>& GetBillCassette() const noexcept { return billCassette_; }
            const std::optional<bool>& GetCashIn() const noexcept { return cashIn_; }
            const std::optional<bool>& GetItemCassette() const noexcept { return itemCassette_; }

        private:
            std::optional<bool> present_;
            std::optional<bool> retract_;
            std::optional<bool> reject_;
            std::optional<bool> billCassette_;
            std::optional<bool> cashIn_;
            std::optional<bool> itemCassette_;
        };

        class CashInLimitClass final
        {
        public:
            CashInLimitClass(
                std::optional<bool> byTotalItems = std::nullopt,
                std::optional<bool> byAmount = std::nullopt)
                : byTotalItems_(byTotalItems), byAmount_(byAmount)
            {
            }

            const std::optional<bool>& GetByTotalItems() const noexcept { return byTotalItems_; }
            const std::optional<bool>& GetByAmount() const noexcept { return byAmount_; }

        private:
            std::optional<bool> byTotalItems_;
            std::optional<bool> byAmount_;
        };

        class CountActionsClass final
        {
        public:
            CountActionsClass(
                std::optional<bool> individual = std::nullopt,
                std::optional<bool> all = std::nullopt)
                : individual_(individual), all_(all)
            {
            }

            const std::optional<bool>& GetIndividual() const noexcept { return individual_; }
            const std::optional<bool>& GetAll() const noexcept { return all_; }

        private:
            std::optional<bool> individual_;
            std::optional<bool> all_;
        };

        class RetainActionClass final
        {
        public:
            RetainActionClass(
                std::optional<bool> counterfeit = std::nullopt,
                std::optional<bool> suspect = std::nullopt,
                std::optional<bool> inked = std::nullopt)
                : counterfeit_(counterfeit), suspect_(suspect), inked_(inked)
            {
            }

            const std::optional<bool>& GetCounterfeit() const noexcept { return counterfeit_; }
            const std::optional<bool>& GetSuspect() const noexcept { return suspect_; }
            const std::optional<bool>& GetInked() const noexcept { return inked_; }

        private:
            std::optional<bool> counterfeit_;
            std::optional<bool> suspect_;
            std::optional<bool> inked_;
        };

        CapabilitiesClass(
            std::optional<TypeEnum> type = std::nullopt,
            std::optional<int> maxCashInItems = std::nullopt,
            std::optional<bool> shutter = std::nullopt,
            std::optional<bool> shutterControl = std::nullopt,
            std::optional<int> intermediateStacker = std::nullopt,
            std::optional<bool> itemsTakenSensor = std::nullopt,
            std::optional<bool> itemsInsertedSensor = std::nullopt,
            std::optional<std::vector<PosCapsClass>> positions = std::nullopt,
            std::shared_ptr<RetractAreasClass> retractAreas = nullptr,
            std::shared_ptr<RetractTransportActionsClass> retractTransportActions = nullptr,
            std::shared_ptr<RetractStackerActionsClass> retractStackerActions = nullptr,
            std::shared_ptr<CashInLimitClass> cashInLimit = nullptr,
            std::shared_ptr<CountActionsClass> countActions = nullptr,
            std::shared_ptr<RetainActionClass> retainAction = nullptr)
            : type_(type), maxCashInItems_(maxCashInItems), shutter_(shutter),
            shutterControl_(shutterControl), intermediateStacker_(intermediateStacker),
            itemsTakenSensor_(itemsTakenSensor), itemsInsertedSensor_(itemsInsertedSensor),
            positions_(std::move(positions)), retractAreas_(std::move(retractAreas)),
            retractTransportActions_(std::move(retractTransportActions)),
            retractStackerActions_(std::move(retractStackerActions)),
            cashInLimit_(std::move(cashInLimit)), countActions_(std::move(countActions)),
            retainAction_(std::move(retainAction))
        {
        }

        const std::optional<TypeEnum>& GetType() const noexcept { return type_; }
        const std::optional<int>& GetMaxCashInItems() const noexcept { return maxCashInItems_; }
        const std::optional<bool>& GetShutter() const noexcept { return shutter_; }
        const std::optional<bool>& GetShutterControl() const noexcept { return shutterControl_; }
        const std::optional<int>& GetIntermediateStacker() const noexcept { return intermediateStacker_; }
        const std::optional<bool>& GetItemsTakenSensor() const noexcept { return itemsTakenSensor_; }
        const std::optional<bool>& GetItemsInsertedSensor() const noexcept { return itemsInsertedSensor_; }
        const std::optional<std::vector<PosCapsClass>>& GetPositions() const noexcept { return positions_; }
        const std::shared_ptr<RetractAreasClass>& GetRetractAreas() const noexcept { return retractAreas_; }
        const std::shared_ptr<RetractTransportActionsClass>& GetRetractTransportActions() const noexcept { return retractTransportActions_; }
        const std::shared_ptr<RetractStackerActionsClass>& GetRetractStackerActions() const noexcept { return retractStackerActions_; }
        const std::shared_ptr<CashInLimitClass>& GetCashInLimit() const noexcept { return cashInLimit_; }
        const std::shared_ptr<CountActionsClass>& GetCountActions() const noexcept { return countActions_; }
        const std::shared_ptr<RetainActionClass>& GetRetainAction() const noexcept { return retainAction_; }

    private:
        std::optional<TypeEnum> type_;
        std::optional<int> maxCashInItems_;
        std::optional<bool> shutter_;
        std::optional<bool> shutterControl_;
        std::optional<int> intermediateStacker_;
        std::optional<bool> itemsTakenSensor_;
        std::optional<bool> itemsInsertedSensor_;
        std::optional<std::vector<PosCapsClass>> positions_;
        std::shared_ptr<RetractAreasClass> retractAreas_;
        std::shared_ptr<RetractTransportActionsClass> retractTransportActions_;
        std::shared_ptr<RetractStackerActionsClass> retractStackerActions_;
        std::shared_ptr<CashInLimitClass> cashInLimit_;
        std::shared_ptr<CountActionsClass> countActions_;
        std::shared_ptr<RetainActionClass> retainAction_;
    };

    inline std::string ToString(CapabilitiesClass::TypeEnum value)
    {
        switch (value)
        {
        case CapabilitiesClass::TypeEnum::TellerBill: return "tellerBill";
        case CapabilitiesClass::TypeEnum::SelfServiceBill: return "selfServiceBill";
        case CapabilitiesClass::TypeEnum::TellerCoin: return "tellerCoin";
        case CapabilitiesClass::TypeEnum::SelfServiceCoin: return "selfServiceCoin";
        default: throw std::invalid_argument("Unknown TypeEnum value");
        }
    }

    inline void to_json(nlohmann::json& j, const CapabilitiesClass::TypeEnum& value)
    {
        j = ToString(value);
    }

    inline void to_json(nlohmann::json& j, const PosCapsClass::UsageClass& p)
    {
        j = nlohmann::json::object();
        if (p.GetIn()) j["in"] = *p.GetIn();
        if (p.GetRefuse()) j["refuse"] = *p.GetRefuse();
        if (p.GetRollback()) j["rollback"] = *p.GetRollback();
    }

    inline void to_json(nlohmann::json& j, const PosCapsClass::RetractAreasClass& p)
    {
        j = nlohmann::json::object();
        if (p.GetRetract()) j["retract"] = *p.GetRetract();
        if (p.GetReject()) j["reject"] = *p.GetReject();
        if (p.GetTransport()) j["transport"] = *p.GetTransport();
        if (p.GetStacker()) j["stacker"] = *p.GetStacker();
        if (p.GetBillCassettes()) j["billCassettes"] = *p.GetBillCassettes();
        if (p.GetCashIn()) j["cashIn"] = *p.GetCashIn();
        if (p.GetItemCassette()) j["itemCassette"] = *p.GetItemCassette();
    }

    inline void to_json(nlohmann::json& j, const PosCapsClass& p)
    {
        j = nlohmann::json::object();
        if (p.GetPosition()) j["position"] = *p.GetPosition();
        if (p.GetUsage()) j["usage"] = *p.GetUsage();
        if (p.GetShutterControl()) j["shutterControl"] = *p.GetShutterControl();
        if (p.GetItemsTakenSensor()) j["itemsTakenSensor"] = *p.GetItemsTakenSensor();
        if (p.GetItemsInsertedSensor()) j["itemsInsertedSensor"] = *p.GetItemsInsertedSensor();
        if (p.GetRetractAreas()) j["retractAreas"] = *p.GetRetractAreas();
        if (p.GetPresentControl()) j["presentControl"] = *p.GetPresentControl();
        if (p.GetPreparePresent()) j["preparePresent"] = *p.GetPreparePresent();
    }

    inline void to_json(nlohmann::json& j, const CapabilitiesClass::RetractAreasClass& p)
    {
        j = nlohmann::json::object();
        if (p.GetRetract()) j["retract"] = *p.GetRetract();
        if (p.GetTransport()) j["transport"] = *p.GetTransport();
        if (p.GetStacker()) j["stacker"] = *p.GetStacker();
        if (p.GetReject()) j["reject"] = *p.GetReject();
        if (p.GetBillCassette()) j["billCassette"] = *p.GetBillCassette();
        if (p.GetCashIn()) j["cashIn"] = *p.GetCashIn();
        if (p.GetItemCassette()) j["itemCassette"] = *p.GetItemCassette();
    }

    inline void to_json(nlohmann::json& j, const CapabilitiesClass::RetractTransportActionsClass& p)
    {
        j = nlohmann::json::object();
        if (p.GetPresent()) j["present"] = *p.GetPresent();
        if (p.GetRetract()) j["retract"] = *p.GetRetract();
        if (p.GetReject()) j["reject"] = *p.GetReject();
        if (p.GetBillCassette()) j["billCassette"] = *p.GetBillCassette();
        if (p.GetCashIn()) j["cashIn"] = *p.GetCashIn();
        if (p.GetItemCassette()) j["itemCassette"] = *p.GetItemCassette();
    }

    inline void to_json(nlohmann::json& j, const CapabilitiesClass::RetractStackerActionsClass& p)
    {
        j = nlohmann::json::object();
        if (p.GetPresent()) j["present"] = *p.GetPresent();
        if (p.GetRetract()) j["retract"] = *p.GetRetract();
        if (p.GetReject()) j["reject"] = *p.GetReject();
        if (p.GetBillCassette()) j["billCassette"] = *p.GetBillCassette();
        if (p.GetCashIn()) j["cashIn"] = *p.GetCashIn();
        if (p.GetItemCassette()) j["itemCassette"] = *p.GetItemCassette();
    }

    inline void to_json(nlohmann::json& j, const CapabilitiesClass::CashInLimitClass& p)
    {
        j = nlohmann::json::object();
        if (p.GetByTotalItems()) j["byTotalItems"] = *p.GetByTotalItems();
        if (p.GetByAmount()) j["byAmount"] = *p.GetByAmount();
    }

    inline void to_json(nlohmann::json& j, const CapabilitiesClass::CountActionsClass& p)
    {
        j = nlohmann::json::object();
        if (p.GetIndividual()) j["individual"] = *p.GetIndividual();
        if (p.GetAll()) j["all"] = *p.GetAll();
    }

    inline void to_json(nlohmann::json& j, const CapabilitiesClass::RetainActionClass& p)
    {
        j = nlohmann::json::object();
        if (p.GetCounterfeit()) j["counterfeit"] = *p.GetCounterfeit();
        if (p.GetSuspect()) j["suspect"] = *p.GetSuspect();
        if (p.GetInked()) j["inked"] = *p.GetInked();
    }

    inline void to_json(nlohmann::json& j, const CapabilitiesClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetType()) j["type"] = *p.GetType();
        if (p.GetMaxCashInItems()) j["maxCashInItems"] = *p.GetMaxCashInItems();
        if (p.GetShutter()) j["shutter"] = *p.GetShutter();
        if (p.GetShutterControl()) j["shutterControl"] = *p.GetShutterControl();
        if (p.GetIntermediateStacker()) j["intermediateStacker"] = *p.GetIntermediateStacker();
        if (p.GetItemsTakenSensor()) j["itemsTakenSensor"] = *p.GetItemsTakenSensor();
        if (p.GetItemsInsertedSensor()) j["itemsInsertedSensor"] = *p.GetItemsInsertedSensor();
        if (p.GetPositions()) j["positions"] = *p.GetPositions();
        if (p.GetRetractAreas()) j["retractAreas"] = *p.GetRetractAreas();
        if (p.GetRetractTransportActions()) j["retractTransportActions"] = *p.GetRetractTransportActions();
        if (p.GetRetractStackerActions()) j["retractStackerActions"] = *p.GetRetractStackerActions();
        if (p.GetCashInLimit()) j["cashInLimit"] = *p.GetCashInLimit();
        if (p.GetCountActions()) j["countActions"] = *p.GetCountActions();
        if (p.GetRetainAction()) j["retainAction"] = *p.GetRetainAction();
    }
}