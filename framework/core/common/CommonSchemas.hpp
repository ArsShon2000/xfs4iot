#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace XFS4IoT::Common
{
    enum class DeviceEnum
    {
        Online,
        Offline,
        PowerOff,
        NoDevice,
        HardwareError,
        UserError,
        DeviceBusy,
        FraudAttempt,
        PotentialFraud,
        Starting
    };

    inline std::string toString(DeviceEnum value)
    {
        switch (value)
        {
        case DeviceEnum::Online:          return "online";
        case DeviceEnum::Offline:         return "offline";
        case DeviceEnum::PowerOff:        return "powerOff";
        case DeviceEnum::NoDevice:        return "noDevice";
        case DeviceEnum::HardwareError:   return "hardwareError";
        case DeviceEnum::UserError:       return "userError";
        case DeviceEnum::DeviceBusy:      return "deviceBusy";
        case DeviceEnum::FraudAttempt:    return "fraudAttempt";
        case DeviceEnum::PotentialFraud:  return "potentialFraud";
        case DeviceEnum::Starting:        return "starting";
        }

        throw std::invalid_argument("Unknown DeviceEnum value");
    }

    inline void to_json(nlohmann::json& j, const DeviceEnum& value)
    {
        j = toString(value);
    }

    enum class PositionStatusEnum
    {
        InPosition,
        NotInPosition,
        Unknown
    };

    inline std::string toString(PositionStatusEnum value)
    {
        switch (value)
        {
        case PositionStatusEnum::InPosition:    return "inPosition";
        case PositionStatusEnum::NotInPosition: return "notInPosition";
        case PositionStatusEnum::Unknown:       return "unknown";
        }

        throw std::invalid_argument("Unknown PositionStatusEnum value");
    }

    inline void to_json(nlohmann::json& j, const PositionStatusEnum& value)
    {
        j = toString(value);
    }

    enum class AntiFraudModuleEnum
    {
        Ok,
        Inoperable,
        DeviceDetected,
        Unknown
    };

    inline std::string toString(AntiFraudModuleEnum value)
    {
        switch (value)
        {
        case AntiFraudModuleEnum::Ok:             return "ok";
        case AntiFraudModuleEnum::Inoperable:     return "inoperable";
        case AntiFraudModuleEnum::DeviceDetected: return "deviceDetected";
        case AntiFraudModuleEnum::Unknown:        return "unknown";
        }

        throw std::invalid_argument("Unknown AntiFraudModuleEnum value");
    }

    inline void to_json(nlohmann::json& j, const AntiFraudModuleEnum& value)
    {
        j = toString(value);
    }

    enum class ExchangeEnum
    {
        Active,
        Inactive
    };

    inline std::string toString(ExchangeEnum value)
    {
        switch (value)
        {
        case ExchangeEnum::Active:   return "active";
        case ExchangeEnum::Inactive: return "inactive";
        }

        throw std::invalid_argument("Unknown ExchangeEnum value");
    }

    inline void to_json(nlohmann::json& j, const ExchangeEnum& value)
    {
        j = toString(value);
    }

    enum class EndToEndSecurityStatusEnum
    {
        NotEnforced,
        NotConfigured,
        Enforced
    };

    inline std::string toString(EndToEndSecurityStatusEnum value)
    {
        switch (value)
        {
        case EndToEndSecurityStatusEnum::NotEnforced:   return "notEnforced";
        case EndToEndSecurityStatusEnum::NotConfigured: return "notConfigured";
        case EndToEndSecurityStatusEnum::Enforced:      return "enforced";
        }

        throw std::invalid_argument("Unknown EndToEndSecurityStatusEnum value");
    }

    inline void to_json(nlohmann::json& j, const EndToEndSecurityStatusEnum& value)
    {
        j = toString(value);
    }

    class PersistentDataStoreStatusClass final
    {
    public:
        explicit PersistentDataStoreStatusClass(
            std::optional<int> remaining = std::nullopt)
            : remaining_(remaining)
        {
        }

        const std::optional<int>& GetRemaining() const noexcept
        {
            return remaining_;
        }

    private:
        std::optional<int> remaining_;

        friend void to_json(nlohmann::json& j, const PersistentDataStoreStatusClass& p)
        {
            j = nlohmann::json::object();

            if (p.remaining_.has_value())
            {
                j["remaining"] = p.remaining_.value();
            }
        }
    };

    class StatusPropertiesClass final
    {
    public:
        StatusPropertiesClass(
            std::optional<DeviceEnum> device = std::nullopt,
            std::optional<PositionStatusEnum> devicePosition = std::nullopt,
            std::optional<int> powerSaveRecoveryTime = std::nullopt,
            std::optional<AntiFraudModuleEnum> antiFraudModule = std::nullopt,
            std::optional<ExchangeEnum> exchange = std::nullopt,
            std::optional<EndToEndSecurityStatusEnum> endToEndSecurity = std::nullopt,
            std::shared_ptr<PersistentDataStoreStatusClass> persistentDataStore = nullptr)
            : device_(device)
            , devicePosition_(devicePosition)
            , powerSaveRecoveryTime_(powerSaveRecoveryTime)
            , antiFraudModule_(antiFraudModule)
            , exchange_(exchange)
            , endToEndSecurity_(endToEndSecurity)
            , persistentDataStore_(std::move(persistentDataStore))
        {
        }

        const std::optional<DeviceEnum>& GetDevice() const noexcept { return device_; }
        const std::optional<PositionStatusEnum>& GetDevicePosition() const noexcept { return devicePosition_; }
        const std::optional<int>& GetPowerSaveRecoveryTime() const noexcept { return powerSaveRecoveryTime_; }
        const std::optional<AntiFraudModuleEnum>& GetAntiFraudModule() const noexcept { return antiFraudModule_; }
        const std::optional<ExchangeEnum>& GetExchange() const noexcept { return exchange_; }
        const std::optional<EndToEndSecurityStatusEnum>& GetEndToEndSecurity() const noexcept { return endToEndSecurity_; }
        const std::shared_ptr<PersistentDataStoreStatusClass>& GetPersistentDataStore() const noexcept { return persistentDataStore_; }

    private:
        std::optional<DeviceEnum> device_;
        std::optional<PositionStatusEnum> devicePosition_;
        std::optional<int> powerSaveRecoveryTime_;
        std::optional<AntiFraudModuleEnum> antiFraudModule_;
        std::optional<ExchangeEnum> exchange_;
        std::optional<EndToEndSecurityStatusEnum> endToEndSecurity_;
        std::shared_ptr<PersistentDataStoreStatusClass> persistentDataStore_;

        friend void to_json(nlohmann::json& j, const StatusPropertiesClass& p)
        {
            j = nlohmann::json::object();

            if (p.device_.has_value())
                j["device"] = p.device_.value();

            if (p.devicePosition_.has_value())
                j["devicePosition"] = p.devicePosition_.value();

            if (p.powerSaveRecoveryTime_.has_value())
                j["powerSaveRecoveryTime"] = p.powerSaveRecoveryTime_.value();

            if (p.antiFraudModule_.has_value())
                j["antiFraudModule"] = p.antiFraudModule_.value();

            if (p.exchange_.has_value())
                j["exchange"] = p.exchange_.value();

            if (p.endToEndSecurity_.has_value())
                j["endToEndSecurity"] = p.endToEndSecurity_.value();

            if (p.persistentDataStore_)
                j["persistentDataStore"] = *p.persistentDataStore_;
        }
    };

    class InterfaceClass final
    {
    public:
        enum class NameEnum
        {
            Common,
            CardReader,
            CashAcceptor,
            CashDispenser,
            CashManagement,
            PinPad,
            Crypto,
            KeyManagement,
            Keyboard,
            TextTerminal,
            Printer,
            BarcodeReader,
            Camera,
            Lights,
            Auxiliaries,
            VendorMode,
            VendorApplication,
            Storage,
            Biometric,
            Check,
            MixedMedia,
            Deposit,
            German,
            BanknoteNeutralization,
            PowerManagement
        };

        class CommandsClass final
        {
        public:
            explicit CommandsClass(std::optional<std::vector<std::string>> versions = std::nullopt)
                : versions_(std::move(versions))
            {
            }

            const std::optional<std::vector<std::string>>& GetVersions() const noexcept
            {
                return versions_;
            }

        private:
            std::optional<std::vector<std::string>> versions_;
        };

        class EventsClass final
        {
        public:
            explicit EventsClass(std::optional<std::vector<std::string>> versions = std::nullopt)
                : versions_(std::move(versions))
            {
            }

            const std::optional<std::vector<std::string>>& GetVersions() const noexcept
            {
                return versions_;
            }

        private:
            std::optional<std::vector<std::string>> versions_;
        };

        InterfaceClass(
            std::optional<NameEnum> name = std::nullopt,
            std::optional<std::unordered_map<std::string, std::shared_ptr<CommandsClass>>> commands = std::nullopt,
            std::optional<std::unordered_map<std::string, std::shared_ptr<EventsClass>>> events = std::nullopt,
            std::optional<int32_t> maximumRequests = std::nullopt)
            : name_(name),
              commands_(std::move(commands)),
              events_(std::move(events)),
              maximumRequests_(maximumRequests)
        {
        }

        const std::optional<NameEnum>& GetName() const noexcept
        {
            return name_;
        }

        const std::optional<std::unordered_map<std::string, std::shared_ptr<CommandsClass>>>& GetCommands() const noexcept
        {
            return commands_;
        }

        const std::optional<std::unordered_map<std::string, std::shared_ptr<EventsClass>>>& GetEvents() const noexcept
        {
            return events_;
        }

        const std::optional<int32_t>& GetMaximumRequests() const noexcept
        {
            return maximumRequests_;
        }

    private:
        std::optional<NameEnum> name_;
        std::optional<std::unordered_map<std::string, std::shared_ptr<CommandsClass>>> commands_;
        std::optional<std::unordered_map<std::string, std::shared_ptr<EventsClass>>> events_;
        std::optional<int32_t> maximumRequests_;
    };

    inline std::string toString(InterfaceClass::NameEnum value)
    {
        switch (value)
        {
        case InterfaceClass::NameEnum::Common:                 return "common";
        case InterfaceClass::NameEnum::CardReader:             return "cardReader";
        case InterfaceClass::NameEnum::CashAcceptor:           return "cashAcceptor";
        case InterfaceClass::NameEnum::CashDispenser:          return "cashDispenser";
        case InterfaceClass::NameEnum::CashManagement:         return "cashManagement";
        case InterfaceClass::NameEnum::PinPad:                 return "pinPad";
        case InterfaceClass::NameEnum::Crypto:                 return "crypto";
        case InterfaceClass::NameEnum::KeyManagement:          return "keyManagement";
        case InterfaceClass::NameEnum::Keyboard:               return "keyboard";
        case InterfaceClass::NameEnum::TextTerminal:           return "textTerminal";
        case InterfaceClass::NameEnum::Printer:                return "printer";
        case InterfaceClass::NameEnum::BarcodeReader:          return "barcodeReader";
        case InterfaceClass::NameEnum::Camera:                 return "camera";
        case InterfaceClass::NameEnum::Lights:                 return "lights";
        case InterfaceClass::NameEnum::Auxiliaries:            return "auxiliaries";
        case InterfaceClass::NameEnum::VendorMode:             return "vendorMode";
        case InterfaceClass::NameEnum::VendorApplication:      return "vendorApplication";
        case InterfaceClass::NameEnum::Storage:                return "storage";
        case InterfaceClass::NameEnum::Biometric:              return "biometric";
        case InterfaceClass::NameEnum::Check:                  return "check";
        case InterfaceClass::NameEnum::MixedMedia:             return "mixedMedia";
        case InterfaceClass::NameEnum::Deposit:                return "deposit";
        case InterfaceClass::NameEnum::German:                 return "german";
        case InterfaceClass::NameEnum::BanknoteNeutralization: return "banknoteNeutralization";
        case InterfaceClass::NameEnum::PowerManagement:        return "powerManagement";
        }

        throw std::invalid_argument("Unknown InterfaceClass::NameEnum value");
    }

    inline void to_json(nlohmann::json& j, const InterfaceClass::NameEnum& value)
    {
        j = toString(value);
    }

    inline void to_json(nlohmann::json& j, const InterfaceClass::CommandsClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetVersions())
        {
            j["versions"] = *p.GetVersions();
        }
    }

    inline void to_json(nlohmann::json& j, const InterfaceClass::EventsClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetVersions())
        {
            j["versions"] = *p.GetVersions();
        }
    }

    inline void to_json(nlohmann::json& j, const InterfaceClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetName())
        {
            j["name"] = *p.GetName();
        }

        if (p.GetCommands())
        {
            nlohmann::json commandsJson = nlohmann::json::object();

            for (const auto& [key, value] : *p.GetCommands())
            {
                if (value)
                {
                    commandsJson[key] = *value;
                }
            }

            j["commands"] = std::move(commandsJson);
        }

        if (p.GetEvents())
        {
            nlohmann::json eventsJson = nlohmann::json::object();

            for (const auto& [key, value] : *p.GetEvents())
            {
                if (value)
                {
                    eventsJson[key] = *value;
                }
            }

            j["events"] = std::move(eventsJson);
        }

        if (p.GetMaximumRequests())
        {
            j["maximumRequests"] = *p.GetMaximumRequests();
        }
    }


    class FirmwareClass final
    {
    public:
        FirmwareClass(
            std::optional<std::string> firmwareName = std::nullopt,
            std::optional<std::string> firmwareVersion = std::nullopt,
            std::optional<std::string> hardwareRevision = std::nullopt)
            : firmwareName_(std::move(firmwareName))
            , firmwareVersion_(std::move(firmwareVersion))
            , hardwareRevision_(std::move(hardwareRevision))
        {
        }

        const std::optional<std::string>& GetFirmwareName() const noexcept
        {
            return firmwareName_;
        }

        const std::optional<std::string>& GetFirmwareVersion() const noexcept
        {
            return firmwareVersion_;
        }

        const std::optional<std::string>& GetHardwareRevision() const noexcept
        {
            return hardwareRevision_;
        }

    private:
        std::optional<std::string> firmwareName_;
        std::optional<std::string> firmwareVersion_;
        std::optional<std::string> hardwareRevision_;
    };

    inline void to_json(nlohmann::json& j, const FirmwareClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetFirmwareName())
        {
            j["firmwareName"] = *p.GetFirmwareName();
        }

        if (p.GetFirmwareVersion())
        {
            j["firmwareVersion"] = *p.GetFirmwareVersion();
        }

        if (p.GetHardwareRevision())
        {
            j["hardwareRevision"] = *p.GetHardwareRevision();
        }
    }


    class SoftwareClass final
    {
    public:
        SoftwareClass(
            std::optional<std::string> softwareName = std::nullopt,
            std::optional<std::string> softwareVersion = std::nullopt)
            : softwareName_(std::move(softwareName))
            , softwareVersion_(std::move(softwareVersion))
        {
        }

        const std::optional<std::string>& GetSoftwareName() const noexcept
        {
            return softwareName_;
        }

        const std::optional<std::string>& GetSoftwareVersion() const noexcept
        {
            return softwareVersion_;
        }

    private:
        std::optional<std::string> softwareName_;
        std::optional<std::string> softwareVersion_;
    };

    inline void to_json(nlohmann::json& j, const SoftwareClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetSoftwareName())
        {
            j["softwareName"] = *p.GetSoftwareName();
        }

        if (p.GetSoftwareVersion())
        {
            j["softwareVersion"] = *p.GetSoftwareVersion();
        }
    }


    class DeviceInformationClass final
    {
    public:
        DeviceInformationClass(
            std::optional<std::string> modelName = std::nullopt,
            std::optional<std::string> serialNumber = std::nullopt,
            std::optional<std::string> revisionNumber = std::nullopt,
            std::optional<std::string> modelDescription = std::nullopt,
            std::optional<std::vector<FirmwareClass>> firmware = std::nullopt,
            std::optional<std::vector<SoftwareClass>> software = std::nullopt)
            : modelName_(std::move(modelName))
            , serialNumber_(std::move(serialNumber))
            , revisionNumber_(std::move(revisionNumber))
            , modelDescription_(std::move(modelDescription))
            , firmware_(std::move(firmware))
            , software_(std::move(software))
        {
        }

        const std::optional<std::string>& GetModelName() const noexcept
        {
            return modelName_;
        }

        const std::optional<std::string>& GetSerialNumber() const noexcept
        {
            return serialNumber_;
        }

        const std::optional<std::string>& GetRevisionNumber() const noexcept
        {
            return revisionNumber_;
        }

        const std::optional<std::string>& GetModelDescription() const noexcept
        {
            return modelDescription_;
        }

        const std::optional<std::vector<FirmwareClass>>& GetFirmware() const noexcept
        {
            return firmware_;
        }

        const std::optional<std::vector<SoftwareClass>>& GetSoftware() const noexcept
        {
            return software_;
        }

    private:
        std::optional<std::string> modelName_;
        std::optional<std::string> serialNumber_;
        std::optional<std::string> revisionNumber_;
        std::optional<std::string> modelDescription_;
        std::optional<std::vector<FirmwareClass>> firmware_;
        std::optional<std::vector<SoftwareClass>> software_;
    };

    inline void to_json(nlohmann::json& j, const DeviceInformationClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetModelName())
        {
            j["modelName"] = *p.GetModelName();
        }

        if (p.GetSerialNumber())
        {
            j["serialNumber"] = *p.GetSerialNumber();
        }

        if (p.GetRevisionNumber())
        {
            j["revisionNumber"] = *p.GetRevisionNumber();
        }

        if (p.GetModelDescription())
        {
            j["modelDescription"] = *p.GetModelDescription();
        }

        if (p.GetFirmware())
        {
            j["firmware"] = *p.GetFirmware();
        }

        if (p.GetSoftware())
        {
            j["software"] = *p.GetSoftware();
        }
    }


    class EndToEndSecurityClass final
    {
    public:
        enum class RequiredEnum
        {
            IfConfigured,
            Always
        };

        enum class ResponseSecurityEnabledEnum
        {
            IfConfigured,
            Always
        };

        EndToEndSecurityClass(
            std::optional<RequiredEnum> required = std::nullopt,
            std::optional<bool> hardwareSecurityElement = std::nullopt,
            std::optional<ResponseSecurityEnabledEnum> responseSecurityEnabled = std::nullopt,
            std::optional<std::vector<std::string>> commands = std::nullopt,
            std::optional<int> commandNonceTimeout = std::nullopt)
            : required_(required)
            , hardwareSecurityElement_(hardwareSecurityElement)
            , responseSecurityEnabled_(responseSecurityEnabled)
            , commands_(std::move(commands))
            , commandNonceTimeout_(commandNonceTimeout)
        {
        }

        const std::optional<RequiredEnum>& GetRequired() const noexcept
        {
            return required_;
        }

        const std::optional<bool>& GetHardwareSecurityElement() const noexcept
        {
            return hardwareSecurityElement_;
        }

        const std::optional<ResponseSecurityEnabledEnum>& GetResponseSecurityEnabled() const noexcept
        {
            return responseSecurityEnabled_;
        }

        const std::optional<std::vector<std::string>>& GetCommands() const noexcept
        {
            return commands_;
        }

        const std::optional<int>& GetCommandNonceTimeout() const noexcept
        {
            return commandNonceTimeout_;
        }

    private:
        std::optional<RequiredEnum> required_;
        std::optional<bool> hardwareSecurityElement_;
        std::optional<ResponseSecurityEnabledEnum> responseSecurityEnabled_;
        std::optional<std::vector<std::string>> commands_;
        std::optional<int> commandNonceTimeout_;
    };

    inline std::string toString(EndToEndSecurityClass::RequiredEnum value)
    {
        switch (value)
        {
        case EndToEndSecurityClass::RequiredEnum::IfConfigured: return "ifConfigured";
        case EndToEndSecurityClass::RequiredEnum::Always:       return "always";
        }

        throw std::invalid_argument("Unknown EndToEndSecurityClass::RequiredEnum value");
    }

    inline void to_json(nlohmann::json& j, const EndToEndSecurityClass::RequiredEnum& value)
    {
        j = toString(value);
    }

    inline std::string toString(EndToEndSecurityClass::ResponseSecurityEnabledEnum value)
    {
        switch (value)
        {
        case EndToEndSecurityClass::ResponseSecurityEnabledEnum::IfConfigured: return "ifConfigured";
        case EndToEndSecurityClass::ResponseSecurityEnabledEnum::Always:       return "always";
        }

        throw std::invalid_argument("Unknown EndToEndSecurityClass::ResponseSecurityEnabledEnum value");
    }

    inline void to_json(nlohmann::json& j, const EndToEndSecurityClass::ResponseSecurityEnabledEnum& value)
    {
        j = toString(value);
    }

    inline void to_json(nlohmann::json& j, const EndToEndSecurityClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetRequired())
        {
            j["required"] = *p.GetRequired();
        }

        if (p.GetHardwareSecurityElement())
        {
            j["hardwareSecurityElement"] = *p.GetHardwareSecurityElement();
        }

        if (p.GetResponseSecurityEnabled())
        {
            j["responseSecurityEnabled"] = *p.GetResponseSecurityEnabled();
        }

        if (p.GetCommands())
        {
            j["commands"] = *p.GetCommands();
        }

        if (p.GetCommandNonceTimeout())
        {
            j["commandNonceTimeout"] = *p.GetCommandNonceTimeout();
        }
    }


    class PersistentDataStoreCapabilityClass final
    {
    public:
        explicit PersistentDataStoreCapabilityClass(
            std::optional<int> capacity = std::nullopt)
            : capacity_(capacity)
        {
        }

        const std::optional<int>& GetCapacity() const noexcept
        {
            return capacity_;
        }

    private:
        std::optional<int> capacity_;
    };

    inline void to_json(nlohmann::json& j, const PersistentDataStoreCapabilityClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetCapacity())
        {
            j["capacity"] = *p.GetCapacity();
        }
    }


    class CapabilityPropertiesClass final
    {
    public:
        CapabilityPropertiesClass(
            std::optional<std::string> serviceVersion = std::nullopt,
            std::optional<std::vector<DeviceInformationClass>> deviceInformation = std::nullopt,
            std::optional<bool> powerSaveControl = std::nullopt,
            std::optional<bool> antiFraudModule = std::nullopt,
            std::shared_ptr<EndToEndSecurityClass> endToEndSecurity = nullptr,
            std::shared_ptr<PersistentDataStoreCapabilityClass> persistentDataStore = nullptr)
            : serviceVersion_(std::move(serviceVersion))
            , deviceInformation_(std::move(deviceInformation))
            , powerSaveControl_(powerSaveControl)
            , antiFraudModule_(antiFraudModule)
            , endToEndSecurity_(std::move(endToEndSecurity))
            , persistentDataStore_(std::move(persistentDataStore))
        {
        }

        const std::optional<std::string>& GetServiceVersion() const noexcept
        {
            return serviceVersion_;
        }

        const std::optional<std::vector<DeviceInformationClass>>& GetDeviceInformation() const noexcept
        {
            return deviceInformation_;
        }

        const std::optional<bool>& GetPowerSaveControl() const noexcept
        {
            return powerSaveControl_;
        }

        const std::optional<bool>& GetAntiFraudModule() const noexcept
        {
            return antiFraudModule_;
        }

        const std::shared_ptr<EndToEndSecurityClass>& GetEndToEndSecurity() const noexcept
        {
            return endToEndSecurity_;
        }

        const std::shared_ptr<PersistentDataStoreCapabilityClass>& GetPersistentDataStore() const noexcept
        {
            return persistentDataStore_;
        }

    private:
        std::optional<std::string> serviceVersion_;
        std::optional<std::vector<DeviceInformationClass>> deviceInformation_;
        std::optional<bool> powerSaveControl_;
        std::optional<bool> antiFraudModule_;
        std::shared_ptr<EndToEndSecurityClass> endToEndSecurity_;
        std::shared_ptr<PersistentDataStoreCapabilityClass> persistentDataStore_;
    };

    inline void to_json(nlohmann::json& j, const CapabilityPropertiesClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetServiceVersion())
        {
            j["serviceVersion"] = *p.GetServiceVersion();
        }

        if (p.GetDeviceInformation())
        {
            j["deviceInformation"] = *p.GetDeviceInformation();
        }

        if (p.GetPowerSaveControl())
        {
            j["powerSaveControl"] = *p.GetPowerSaveControl();
        }

        if (p.GetAntiFraudModule())
        {
            j["antiFraudModule"] = *p.GetAntiFraudModule();
        }

        if (p.GetEndToEndSecurity())
        {
            j["endToEndSecurity"] = *p.GetEndToEndSecurity();
        }

        if (p.GetPersistentDataStore())
        {
            j["persistentDataStore"] = *p.GetPersistentDataStore();
        }
    }


    class StatusPropertiesChangedClass final
    {
    public:
        StatusPropertiesChangedClass(
            std::optional<DeviceEnum> device = std::nullopt,
            std::optional<PositionStatusEnum> devicePosition = std::nullopt,
            std::optional<int> powerSaveRecoveryTime = std::nullopt,
            std::optional<AntiFraudModuleEnum> antiFraudModule = std::nullopt,
            std::optional<ExchangeEnum> exchange = std::nullopt,
            std::optional<EndToEndSecurityStatusEnum> endToEndSecurity = std::nullopt,
            std::shared_ptr<PersistentDataStoreStatusClass> persistentDataStore = nullptr)
            : device_(device)
            , devicePosition_(devicePosition)
            , powerSaveRecoveryTime_(powerSaveRecoveryTime)
            , antiFraudModule_(antiFraudModule)
            , exchange_(exchange)
            , endToEndSecurity_(endToEndSecurity)
            , persistentDataStore_(std::move(persistentDataStore))
        {
        }

        const std::optional<DeviceEnum>& GetDevice() const noexcept
        {
            return device_;
        }

        const std::optional<PositionStatusEnum>& GetDevicePosition() const noexcept
        {
            return devicePosition_;
        }

        const std::optional<int>& GetPowerSaveRecoveryTime() const noexcept
        {
            return powerSaveRecoveryTime_;
        }

        const std::optional<AntiFraudModuleEnum>& GetAntiFraudModule() const noexcept
        {
            return antiFraudModule_;
        }

        const std::optional<ExchangeEnum>& GetExchange() const noexcept
        {
            return exchange_;
        }

        const std::optional<EndToEndSecurityStatusEnum>& GetEndToEndSecurity() const noexcept
        {
            return endToEndSecurity_;
        }

        const std::shared_ptr<PersistentDataStoreStatusClass>& GetPersistentDataStore() const noexcept
        {
            return persistentDataStore_;
        }

    private:
        std::optional<DeviceEnum> device_;
        std::optional<PositionStatusEnum> devicePosition_;
        std::optional<int> powerSaveRecoveryTime_;
        std::optional<AntiFraudModuleEnum> antiFraudModule_;
        std::optional<ExchangeEnum> exchange_;
        std::optional<EndToEndSecurityStatusEnum> endToEndSecurity_;
        std::shared_ptr<PersistentDataStoreStatusClass> persistentDataStore_;
    };

    inline void to_json(nlohmann::json& j, const StatusPropertiesChangedClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetDevice())
        {
            j["device"] = *p.GetDevice();
        }

        if (p.GetDevicePosition())
        {
            j["devicePosition"] = *p.GetDevicePosition();
        }

        if (p.GetPowerSaveRecoveryTime())
        {
            j["powerSaveRecoveryTime"] = *p.GetPowerSaveRecoveryTime();
        }

        if (p.GetAntiFraudModule())
        {
            j["antiFraudModule"] = *p.GetAntiFraudModule();
        }

        if (p.GetExchange())
        {
            j["exchange"] = *p.GetExchange();
        }

        if (p.GetEndToEndSecurity())
        {
            j["endToEndSecurity"] = *p.GetEndToEndSecurity();
        }

        if (p.GetPersistentDataStore())
        {
            j["persistentDataStore"] = *p.GetPersistentDataStore();
        }
    }


} // namespace XFS4IoT::Common