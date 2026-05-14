#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <stdexcept>

namespace XFS4IoTFramework::Common
{

    class InterfaceInfo
    {
    public:
        virtual ~InterfaceInfo() = default;

        virtual std::vector<std::string> GetCommandNames() const = 0;
        virtual std::vector<std::string> GetEventNames() const = 0;
    };

    class CommonCapabilitiesClass
    {
    public:

        /// <summary>
        /// Common Interface Class
        /// </summary>
        class CommonInterfaceClass : public XFS4IoTFramework::Common::InterfaceInfo
        {
        public:
            enum class CommandEnum
            {
                Capabilities,
                Status,
                ClearCommandNonce,
                GetCommandNonce,
                GetTransactionState,
                PowerSaveControl,
                SetTransactionState,
                SetVersions,
                Cancel
            };

            enum class EventEnum
            {
                ErrorEvent,
                NonceClearedEvent,
                StatusChangedEvent
            };

            CommonInterfaceClass(
                std::vector<CommandEnum> commands = {},
                std::vector<EventEnum> events = {})
                : commands_(std::move(commands))
                , events_(std::move(events))
            {
            }

            const std::vector<CommandEnum>& GetCommands() const noexcept
            {
                return commands_;
            }

            const std::vector<EventEnum>& GetEvents() const noexcept
            {
                return events_;
            }

            std::vector<std::string> GetCommandNames() const override
            {
                std::vector<std::string> result;
                result.reserve(commands_.size());

                for (const auto cmd : commands_)
                    result.push_back(ToString(cmd));

                return result;
            }

            std::vector<std::string> GetEventNames() const override
            {
                std::vector<std::string> result;
                result.reserve(events_.size());

                for (const auto evt : events_)
                    result.push_back(ToString(evt));

                return result;
            }

        private:
            static std::string ToString(CommandEnum value)
            {
                switch (value)
                {
                case CommandEnum::Capabilities: return "Capabilities";
                case CommandEnum::Status: return "Status";
                case CommandEnum::ClearCommandNonce: return "ClearCommandNonce";
                case CommandEnum::GetCommandNonce: return "GetCommandNonce";
                case CommandEnum::GetTransactionState: return "GetTransactionState";
                case CommandEnum::PowerSaveControl: return "PowerSaveControl";
                case CommandEnum::SetTransactionState: return "SetTransactionState";
                case CommandEnum::SetVersions: return "SetVersions";
                case CommandEnum::Cancel: return "Cancel";
                }
                throw std::invalid_argument("Unknown CommonInterfaceClass::CommandEnum");
            }

            static std::string ToString(EventEnum value)
            {
                switch (value)
                {
                case EventEnum::ErrorEvent: return "ErrorEvent";
                case EventEnum::NonceClearedEvent: return "NonceClearedEvent";
                case EventEnum::StatusChangedEvent: return "StatusChangedEvent";
                }
                throw std::invalid_argument("Unknown CommonInterfaceClass::EventEnum");
            }

        private:
            std::vector<CommandEnum> commands_;
            std::vector<EventEnum> events_;
        };

        /// <summary>
        /// CashAcceptor Interface Class
        /// </summary>
        class CashAcceptorInterfaceClass : public XFS4IoTFramework::Common::InterfaceInfo
        {
        public:
            enum class CommandEnum
            {
                CashIn,
                CashInEnd,
                CashInRollback,
                CashInStart,
                CashUnitCount,
                CompareSignature,
                ConfigureNoteReader,
                ConfigureNoteTypes,
                CreateSignature,
                Deplete,
                DeviceLockControl,
                GetCashInStatus,
                GetDepleteSource,
                GetDeviceLockStatus,
                GetPositionCapabilities,
                GetPresentStatus,
                GetReplenishTarget,
                PreparePresent,
                PresentMedia,
                Replenish
            };

            enum class EventEnum
            {
                IncompleteDepleteEvent,
                IncompleteReplenishEvent,
                InputRefuseEvent,
                InsertItemsEvent,
                SubCashInEvent
            };

            CashAcceptorInterfaceClass(
                std::vector<CommandEnum> commands = {},
                std::vector<EventEnum> events = {})
                : commands_(std::move(commands))
                , events_(std::move(events))
            {
            }

            const std::vector<CommandEnum>& GetCommands() const noexcept
            {
                return commands_;
            }

            const std::vector<EventEnum>& GetEvents() const noexcept
            {
                return events_;
            }

            std::vector<std::string> GetCommandNames() const override
            {
                std::vector<std::string> result;
                result.reserve(commands_.size());

                for (const auto cmd : commands_)
                    result.push_back(ToString(cmd));

                return result;
            }

            std::vector<std::string> GetEventNames() const override
            {
                std::vector<std::string> result;
                result.reserve(events_.size());

                for (const auto evt : events_)
                    result.push_back(ToString(evt));

                return result;
            }

        private:
            static std::string ToString(CommandEnum value)
            {
                switch (value)
                {
                case CommandEnum::CashIn: return "CashIn";
                case CommandEnum::CashInEnd: return "CashInEnd";
                case CommandEnum::CashInRollback: return "CashInRollback";
                case CommandEnum::CashInStart: return "CashInStart";
                case CommandEnum::CashUnitCount: return "CashUnitCount";
                case CommandEnum::CompareSignature: return "CompareSignature";
                case CommandEnum::ConfigureNoteReader: return "ConfigureNoteReader";
                case CommandEnum::ConfigureNoteTypes: return "ConfigureNoteTypes";
                case CommandEnum::CreateSignature: return "CreateSignature";
                case CommandEnum::Deplete: return "Deplete";
                case CommandEnum::DeviceLockControl: return "DeviceLockControl";
                case CommandEnum::GetCashInStatus: return "GetCashInStatus";
                case CommandEnum::GetDepleteSource: return "GetDepleteSource";
                case CommandEnum::GetDeviceLockStatus: return "GetDeviceLockStatus";
                case CommandEnum::GetPositionCapabilities: return "GetPositionCapabilities";
                case CommandEnum::GetPresentStatus: return "GetPresentStatus";
                case CommandEnum::GetReplenishTarget: return "GetReplenishTarget";
                case CommandEnum::PreparePresent: return "PreparePresent";
                case CommandEnum::PresentMedia: return "PresentMedia";
                case CommandEnum::Replenish: return "Replenish";
                }
                throw std::invalid_argument("Unknown CashAcceptorInterfaceClass::CommandEnum");
            }

            static std::string ToString(EventEnum value)
            {
                switch (value)
                {
                case EventEnum::IncompleteDepleteEvent: return "IncompleteDepleteEvent";
                case EventEnum::IncompleteReplenishEvent: return "IncompleteReplenishEvent";
                case EventEnum::InputRefuseEvent: return "InputRefuseEvent";
                case EventEnum::InsertItemsEvent: return "InsertItemsEvent";
                case EventEnum::SubCashInEvent: return "SubCashInEvent";
                }
                throw std::invalid_argument("Unknown CashAcceptorInterfaceClass::EventEnum");
            }

        private:
            std::vector<CommandEnum> commands_;
            std::vector<EventEnum> events_;
        };

        /// <summary>
        /// CashManagement Interface Class
        /// </summary>
        class CashManagementInterfaceClass : public XFS4IoTFramework::Common::InterfaceInfo
        {
        public:
            enum class CommandEnum
            {
                CalibrateCashUnit,
                CloseShutter,
                GetBankNoteTypes,
                GetClassificationList,
                GetItemInfo,
                GetTellerInfo,
                OpenShutter,
                Reset,
                Retract,
                SetClassificationList,
                SetTellerInfo
            };

            enum class EventEnum
            {
                IncompleteRetractEvent,
                InfoAvailableEvent,
                ItemsInsertedEvent,
                ItemsPresentedEvent,
                ItemsTakenEvent,
                MediaDetectedEvent,
                NoteErrorEvent,
                ShutterStatusChangedEvent,
                TellerInfoChangedEvent
            };

            CashManagementInterfaceClass(
                std::vector<CommandEnum> commands = {},
                std::vector<EventEnum> events = {})
                : commands_(std::move(commands))
                , events_(std::move(events))
            {
            }

            const std::vector<CommandEnum>& GetCommands() const noexcept
            {
                return commands_;
            }

            const std::vector<EventEnum>& GetEvents() const noexcept
            {
                return events_;
            }

            std::vector<std::string> GetCommandNames() const override
            {
                std::vector<std::string> result;
                result.reserve(commands_.size());

                for (const auto cmd : commands_)
                    result.push_back(ToString(cmd));

                return result;
            }

            std::vector<std::string> GetEventNames() const override
            {
                std::vector<std::string> result;
                result.reserve(events_.size());

                for (const auto evt : events_)
                    result.push_back(ToString(evt));

                return result;
            }

        private:
            static std::string ToString(CommandEnum value)
            {
                switch (value)
                {
                case CommandEnum::CalibrateCashUnit: return "CalibrateCashUnit";
                case CommandEnum::CloseShutter: return "CloseShutter";
                case CommandEnum::GetBankNoteTypes: return "GetBankNoteTypes";
                case CommandEnum::GetClassificationList: return "GetClassificationList";
                case CommandEnum::GetItemInfo: return "GetItemInfo";
                case CommandEnum::GetTellerInfo: return "GetTellerInfo";
                case CommandEnum::OpenShutter: return "OpenShutter";
                case CommandEnum::Reset: return "Reset";
                case CommandEnum::Retract: return "Retract";
                case CommandEnum::SetClassificationList: return "SetClassificationList";
                case CommandEnum::SetTellerInfo: return "SetTellerInfo";
                }
                throw std::invalid_argument("Unknown CashManagementInterfaceClass::CommandEnum");
            }

            static std::string ToString(EventEnum value)
            {
                switch (value)
                {
                case EventEnum::IncompleteRetractEvent: return "IncompleteRetractEvent";
                case EventEnum::InfoAvailableEvent: return "InfoAvailableEvent";
                case EventEnum::ItemsInsertedEvent: return "ItemsInsertedEvent";
                case EventEnum::ItemsPresentedEvent: return "ItemsPresentedEvent";
                case EventEnum::ItemsTakenEvent: return "ItemsTakenEvent";
                case EventEnum::MediaDetectedEvent: return "MediaDetectedEvent";
                case EventEnum::NoteErrorEvent: return "NoteErrorEvent";
                case EventEnum::ShutterStatusChangedEvent: return "ShutterStatusChangedEvent";
                case EventEnum::TellerInfoChangedEvent: return "TellerInfoChangedEvent";
                }
                throw std::invalid_argument("Unknown CashManagementInterfaceClass::EventEnum");
            }

        private:
            std::vector<CommandEnum> commands_;
            std::vector<EventEnum> events_;
        };

        /// <summary>
        /// Deposit Class
        /// </summary>
        class DepositClass : public XFS4IoTFramework::Common::InterfaceInfo
        {
        public:
            enum class CommandEnum
            {
                SetProtection,
                TriggerNeutralization
            };

            enum class EventEnum
            {
            };

            DepositClass(
                std::vector<CommandEnum> commands = {},
                std::vector<EventEnum> events = {})
                : commands_(std::move(commands))
                , events_(std::move(events))
            {
            }

            const std::vector<CommandEnum>& GetCommands() const noexcept
            {
                return commands_;
            }

            const std::vector<EventEnum>& GetEvents() const noexcept
            {
                return events_;
            }

            std::vector<std::string> GetCommandNames() const override
            {
                std::vector<std::string> result;
                result.reserve(commands_.size());

                for (const auto cmd : commands_)
                    result.push_back(ToString(cmd));

                return result;
            }

            std::vector<std::string> GetEventNames() const override
            {
                return {};
            }

        private:
            static std::string ToString(CommandEnum value)
            {
                switch (value)
                {
                case CommandEnum::SetProtection: return "SetProtection";
                case CommandEnum::TriggerNeutralization: return "TriggerNeutralization";
                }
                throw std::invalid_argument("Unknown DepositClass::CommandEnum");
            }

        private:
            std::vector<CommandEnum> commands_;
            std::vector<EventEnum> events_;
        };

        /// <summary>
        /// Firmware Class
        /// </summary>
        class FirmwareClass
        {
        public:
            FirmwareClass(
                std::string firmwareName = "",
                std::string firmwareVersion = "",
                std::string hardwareRevision = "")
                : firmwareName_(std::move(firmwareName))
                , firmwareVersion_(std::move(firmwareVersion))
                , hardwareRevision_(std::move(hardwareRevision))
            {
            }

            const std::string& GetFirmwareName() const noexcept
            {
                return firmwareName_;
            }

            const std::string& GetFirmwareVersion() const noexcept
            {
                return firmwareVersion_;
            }

            const std::string& GetHardwareRevision() const noexcept
            {
                return hardwareRevision_;
            }

        private:
            std::string firmwareName_;
            std::string firmwareVersion_;
            std::string hardwareRevision_;
        };

        /// <summary>
        /// Software Class
        /// </summary>
        class SoftwareClass
        {
        public:
            SoftwareClass(
                std::string softwareName = "",
                std::string softwareVersion = "")
                : softwareName_(std::move(softwareName))
                , softwareVersion_(std::move(softwareVersion))
            {
            }

            const std::string& GetSoftwareName() const noexcept
            {
                return softwareName_;
            }

            const std::string& GetSoftwareVersion() const noexcept
            {
                return softwareVersion_;
            }

        private:
            std::string softwareName_;
            std::string softwareVersion_;
        };

        /// <summary>
        /// Device Information Class
        /// </summary>
        class DeviceInformationClass
        {
        public:
            DeviceInformationClass(
                std::string modelName = "",
                std::string serialNumber = "",
                std::string revisionNumber = "",
                std::string modelDescription = "",
                std::vector<FirmwareClass> firmware = {},
                std::vector<SoftwareClass> software = {})
                : modelName_(std::move(modelName))
                , serialNumber_(std::move(serialNumber))
                , revisionNumber_(std::move(revisionNumber))
                , modelDescription_(std::move(modelDescription))
                , firmware_(std::move(firmware))
                , software_(std::move(software))
            {
            }

            const std::string& GetModelName() const noexcept
            {
                return modelName_;
            }

            const std::string& GetSerialNumber() const noexcept
            {
                return serialNumber_;
            }

            const std::string& GetRevisionNumber() const noexcept
            {
                return revisionNumber_;
            }

            const std::string& GetModelDescription() const noexcept
            {
                return modelDescription_;
            }

            const std::vector<FirmwareClass>& GetFirmware() const noexcept
            {
                return firmware_;
            }

            const std::vector<SoftwareClass>& GetSoftware() const noexcept
            {
                return software_;
            }

        private:
            std::string modelName_;
            std::string serialNumber_;
            std::string revisionNumber_;
            std::string modelDescription_;
            std::vector<FirmwareClass> firmware_;
            std::vector<SoftwareClass> software_;
        };

        /// <summary>
        /// End to End Security Class
        /// </summary>
        class EndToEndSecurityClass
        {
        public:
            enum class RequiredEnum
            {
                IfConfigured,
                Always
            };

            enum class ResponseSecurityEnabledEnum
            {
                NotSupported,
                IfConfigured,
                Always
            };

            EndToEndSecurityClass(
                std::optional<RequiredEnum> required = std::nullopt,
                std::optional<bool> hardwareSecurityElement = std::nullopt,
                std::optional<ResponseSecurityEnabledEnum> responseSecurityEnabled = std::nullopt,
                std::optional<int> commandNonceTimeout = std::nullopt)
                : required_(required)
                , hardwareSecurityElement_(hardwareSecurityElement)
                , responseSecurityEnabled_(responseSecurityEnabled)
                , commandNonceTimeout_(commandNonceTimeout)
            {
            }

            std::optional<RequiredEnum> GetRequired() const { return required_; }
            std::optional<bool> GetHardwareSecurityElement() const { return hardwareSecurityElement_; }
            std::optional<ResponseSecurityEnabledEnum> GetResponseSecurityEnabled() const { return responseSecurityEnabled_; }
            std::optional<int> GetCommandNonceTimeout() const { return commandNonceTimeout_; }

        private:
            std::optional<RequiredEnum> required_;
            std::optional<bool> hardwareSecurityElement_;
            std::optional<ResponseSecurityEnabledEnum> responseSecurityEnabled_;
            std::optional<int> commandNonceTimeout_;
        };

        CommonCapabilitiesClass(
            CommonInterfaceClass commonInterface,
            std::optional<CashAcceptorInterfaceClass> cashAcceptorInterface = std::nullopt,
            std::optional<CashManagementInterfaceClass> cashManagementInterface = std::nullopt,
            std::optional<DepositClass> depositInterface = std::nullopt,
            std::vector<DeviceInformationClass> deviceInformation = {},
            std::optional<bool> powerSaveControl = std::nullopt,
            std::optional<bool> antiFraudModule = std::nullopt,
            std::optional<EndToEndSecurityClass> endToEndSecurity = std::nullopt)
            : commonInterface_(std::move(commonInterface))
            , cashAcceptorInterface_(cashAcceptorInterface)
            , cashManagementInterface_(cashManagementInterface)
            , depositInterface_(depositInterface)
            , deviceInformation_(std::move(deviceInformation))
            , powerSaveControl_(powerSaveControl)
            , antiFraudModule_(antiFraudModule)
            , endToEndSecurity_(endToEndSecurity)
        {
        }

        const std::optional<CommonInterfaceClass>& GetCommonInterface() const { return commonInterface_; }
        std::optional<CashAcceptorInterfaceClass> GetCashAcceptorInterface() const { return cashAcceptorInterface_; }
        std::optional<CashManagementInterfaceClass> GetCashManagementInterface() const { return cashManagementInterface_; }
        std::optional<DepositClass> GetDepositInterface() const { return depositInterface_; }
        const std::vector<DeviceInformationClass>& GetDeviceInformation() const { return deviceInformation_; }

        [[deprecated("This property is obsolete. Migrate to PowerManagement interface.")]]
        std::optional<bool> GetPowerSaveControl() const { return powerSaveControl_; }

        std::optional<bool> GetAntiFraudModule() const { return antiFraudModule_; }
        std::optional<EndToEndSecurityClass> GetEndToEndSecurity() const { return endToEndSecurity_; }

    private:
        std::optional<CommonInterfaceClass> commonInterface_;
        std::optional<CashAcceptorInterfaceClass> cashAcceptorInterface_;
        std::optional<CashManagementInterfaceClass> cashManagementInterface_;
        std::optional<DepositClass> depositInterface_;
        std::vector<DeviceInformationClass> deviceInformation_;
        std::optional<bool> powerSaveControl_;
        std::optional<bool> antiFraudModule_;
        std::optional<EndToEndSecurityClass> endToEndSecurity_;
    };

} // namespace XFS4IoTFramework::Common