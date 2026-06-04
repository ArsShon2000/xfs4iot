#include "CommonServiceClass.hpp"

#include <format>
#include <utility>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

#include "../../core/Contracts.hpp"
#include "../../core/common/XFSConstants.hpp"
#include "../../core/Command.hpp"
#include "../HasFlagHelper.hpp"

#include "../CommonServiceProvider/Handlers/CapabilitiesHandler.hpp"
#include "../CashManagementServiceProvider/Handlers/ResetHandler.hpp"
#include "../CashManagementServiceProvider/Handlers/GetBankNoteTypesHandler.hpp"
#include "../CashAcceptorServiceProvider/Handlers/ConfigureNoteTypesHandler.hpp"
#include "../CashAcceptorServiceProvider/Handlers/CashInStartHandler.hpp"
#include "../CashAcceptorServiceProvider/Handlers/CashInHandler.hpp"
#include "../CommonServiceProvider/Handlers/SetVersionsHandler.hpp"
#include "../CommonServiceProvider/Handlers/UnsupportedCommandHandler.hpp"

namespace
{
    // Create a shared_ptr<T> that shares ownership with a shared_ptr<void>.
    // This uses the aliasing constructor. The caller must ensure the underlying
    // object actually is of type T (the device code provides sender as the real
    // object but typed as shared_ptr<void>).
    template<typename T>
    std::shared_ptr<T> alias_cast(const std::shared_ptr<void>& s) noexcept
    {
        if (!s) return nullptr;
        return std::shared_ptr<T>(s, reinterpret_cast<T*>(s.get()));
    }

}

namespace XFS4IoTServer
{
    CommonServiceClass::CommonServiceClass(
        std::shared_ptr<IServiceProvider> serviceProvider,
        std::shared_ptr<ILogger> logger,
        const std::string& serviceName,
        std::shared_ptr<XFS4IoT::MessageDecoder> decoder)
        : serviceProvider_(std::move(serviceProvider))
        , logger_(std::move(logger))
        , serviceName_(serviceName)
        , decoder_(decoder)
    {
        if (!serviceProvider_)
        {
            throw std::invalid_argument("CommonServiceClass: serviceProvider is null");
        }

        if (!logger_)
        {
            throw std::invalid_argument("CommonServiceClass: logger is null");
        }

        auto deviceBase = serviceProvider_->GetDevice();
        if (!deviceBase)
        {
            throw std::invalid_argument(
                "CommonServiceClass: ServiceProvider.Device is null");
        }

        device_ =
            std::dynamic_pointer_cast<XFS4IoTFramework::Common::ICommonDevice>(deviceBase);

        if (!device_)
        {
            throw std::runtime_error(
                "CommonServiceClass: ServiceProvider.Device is not ICommonDevice");
        }

        GetCapabilities();
        GetStatus();
    }

    std::shared_ptr<XFS4IoTFramework::Common::CommonCapabilitiesClass>
        CommonServiceClass::GetCommonCapabilities() const noexcept
    {
        return commonCapabilities_;
    }

    std::shared_ptr<XFS4IoTFramework::Common::CashManagementCapabilitiesClass>
        CommonServiceClass::GetCashManagementCapabilities() const noexcept
    {
        return cashManagementCapabilities_;
    }

    std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass>
        CommonServiceClass::GetCashAcceptorCapabilities() const noexcept
    {
        return cashAcceptorCapabilities_;
    }

    std::shared_ptr<XFS4IoTFramework::Common::CommonStatusClass>
        CommonServiceClass::GetCommonStatus() const noexcept
    {
        return commonStatus_;
    }

    std::shared_ptr<XFS4IoTFramework::Common::CashManagementStatusClass>
        CommonServiceClass::GetCashManagementStatus() const noexcept
    {
        return cashManagementStatus_;
    }

    std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorStatusClass>
        CommonServiceClass::GetCashAcceptorStatus() const noexcept
    {
        return cashAcceptorStatus_;
    }

    void CommonServiceClass::GetCapabilities()
    {
        if (commonCapabilities_)
        {
            return;
        }

        logger_->trace("CommonServiceClass::GetCapabilities - CommonDev.CommonCapabilities");

        commonCapabilities_ = device_->GetCommonCapabilities();

        logger_->trace("CommonServiceClass::GetCapabilities - CommonDev.CommonCapabilities=");

        if (!commonCapabilities_)
        {
            throw std::runtime_error(
                "Класс устройства установил свойство CommonCapabilities в значение null. "
                "Класс устройства должен сообщать о возможностях устройства.");
        }

        //cashManagementCapabilities_ = device_->GetCashManagementCapabilities();
        //cashAcceptorCapabilities_ = device_->GetCashAcceptorCapabilities();

        const auto supportedServiceMessages = BuildSupportedServiceMessages();
        if (!supportedServiceMessages.empty())
        {
            serviceProvider_->SetMessagesSupported(supportedServiceMessages);
        }
    }

    void CommonServiceClass::GetStatus()
    {
        if (commonStatus_)
        {
            return;
        }

        logger_->trace("CommonServiceClass::GetStatus - CommonDev.CommonStatus");

        commonStatus_ = device_->GetCommonStatus();

        logger_->trace("CommonServiceClass::GetStatus - CommonDev.CommonStatus = ");

        if (!commonStatus_)
        {
            throw std::runtime_error(
                "Класс устройства установил свойство CommonStatus в значение null. "
                "Класс устройства должен сообщать о состоянии устройства.");
        }

        //cashManagementStatus_ = device_->GetCashManagementStatus();
        //cashAcceptorStatus_ = device_->GetCashAcceptorStatus();

        // Здесь нужно будет подключить реальный callback-механизм, если он есть в CommonStatusClass.
        // Например:
        //
        // commonStatus_->SetPropertyChangedHandler(
        //     [this](std::shared_ptr<void> sender, std::shared_ptr<PropertyChangedEventArgs> args)
        //     {
        //         StatusChangedEventForwarder(std::move(sender), std::move(args));
        //     });
    }

    std::map<std::string, XFS4IoT::MessageTypeInfo>
        CommonServiceClass::BuildSupportedServiceMessages() const
    {
        std::map<std::string, XFS4IoT::MessageTypeInfo> supportedServiceMessages;

		const std::map<std::string, XFS4IoT::MessageTypeInfo> frameworkMessages = {
			{
				"CashManagement.MediaDetectedEvent",
				XFS4IoT::MessageTypeInfo(
					XFS4IoT::MessageTypeInfo::MessageTypeEnum::Event,
					{ "2.0" }
				)
			},
			{
				"CashAcceptor.InsertItemsEvent",
				XFS4IoT::MessageTypeInfo(
					XFS4IoT::MessageTypeInfo::MessageTypeEnum::Event,
					{ "2.0" }
				)
			},
			{
				"CashAcceptor.InputRefuseEvent",
				XFS4IoT::MessageTypeInfo(
					XFS4IoT::MessageTypeInfo::MessageTypeEnum::Event,
					{ "2.0" }
				)
			},
			{
				"CashManagement.ItemsInsertedEvent",
				XFS4IoT::MessageTypeInfo(
					XFS4IoT::MessageTypeInfo::MessageTypeEnum::Event,
					{ "2.0" }
				)
			},
			{
		        "Common.Capabilities",
		        XFS4IoT::MessageTypeInfo(
			        XFS4IoT::MessageTypeInfo::MessageTypeEnum::Command,
			        { "1.0" }
		        )
	        },
	        {
		        "CashManagement.Reset",
		        XFS4IoT::MessageTypeInfo(
			        XFS4IoT::MessageTypeInfo::MessageTypeEnum::Command,
			        { "1.0" }
		        )
	        },
	        {
		        "CashManagement.GetBankNoteTypes",
		        XFS4IoT::MessageTypeInfo(
			        XFS4IoT::MessageTypeInfo::MessageTypeEnum::Command,
			        { "1.0" }
		        )
	        },
	        {
		        "CashAcceptor.ConfigureNoteTypes",
		        XFS4IoT::MessageTypeInfo(
			        XFS4IoT::MessageTypeInfo::MessageTypeEnum::Command,
			        { "1.0" }
		        )
	        },
	        {
		        "CashAcceptor.CashInStart",
		        XFS4IoT::MessageTypeInfo(
			        XFS4IoT::MessageTypeInfo::MessageTypeEnum::Command,
			        { "1.0" }
		        )
	        },
	        {
		        "CashAcceptor.CashIn",
		        XFS4IoT::MessageTypeInfo(
			        XFS4IoT::MessageTypeInfo::MessageTypeEnum::Command,
			        { "1.0" }
		        )
	        },
            {
                "Common.SetVersions",
                XFS4IoT::MessageTypeInfo(
                    XFS4IoT::MessageTypeInfo::MessageTypeEnum::Command,
                    { "1.0" }
                )
            },
            {
                "Common.Unsupported",
                XFS4IoT::MessageTypeInfo(
                    XFS4IoT::MessageTypeInfo::MessageTypeEnum::Command,
                    { "1.0" }
                )
            }
		};

        if (!commonCapabilities_)
        {
            return supportedServiceMessages;
        }

        auto serviceProvider = std::dynamic_pointer_cast<XFS4IoTServer::ServiceProvider>(serviceProvider_);


        if (commonCapabilities_->GetCommonInterface())
        {
            AddInterfaceMessages(
                supportedServiceMessages,
                XFS4IoT::Common::InterfaceClass::NameEnum::Common,
                *commonCapabilities_->GetCommonInterface(),
                frameworkMessages,
                serviceProvider);
        }

        if (commonCapabilities_->GetCashManagementInterface())
        {
            AddInterfaceMessages(
                supportedServiceMessages,
                XFS4IoT::Common::InterfaceClass::NameEnum::CashManagement,
                *commonCapabilities_->GetCashManagementInterface(),
                frameworkMessages,
                serviceProvider);
        }

        if (commonCapabilities_->GetCashAcceptorInterface())
        {
            AddInterfaceMessages(
                supportedServiceMessages,
                XFS4IoT::Common::InterfaceClass::NameEnum::CashAcceptor,
                *commonCapabilities_->GetCashAcceptorInterface(),
                frameworkMessages,
                serviceProvider);
        }

        // Ниже добавляй остальные интерфейсы по мере появления их C++ классов:
        // Camera, CardReader, CashDispenser, Crypto, Keyboard, KeyManagement, PinPad,
        // BarcodeReader, Biometric, Auxiliaries, Printer, TextTerminal, Lights, Storage,
        // VendorApplication, VendorMode, Check, MixedMedia, PowerManagement,
        // BanknoteNeutralization, German, Deposit.

        return supportedServiceMessages;
    }

    std::map<std::string, XFS4IoT::MessageTypeInfo> CommonServiceClass::BuildSupportedServiceEvents() const
    {
        std::map<std::string, XFS4IoT::MessageTypeInfo> supportedServiceMessages;

        const auto frameworkMessages = serviceProvider_->GetMessagesSupported();

        return std::map<std::string, XFS4IoT::MessageTypeInfo>();
    }

    void CommonServiceClass::AddInterfaceMessages(
        std::map<std::string, XFS4IoT::MessageTypeInfo>& target,
        XFS4IoT::Common::InterfaceClass::NameEnum interfaceName,
        const XFS4IoTFramework::Common::InterfaceInfo& interfaceInfo,
        const std::map<std::string, XFS4IoT::MessageTypeInfo>& frameworkMessages,
        std::shared_ptr<XFS4IoTServer::ServiceProvider> serviceProvider) const
    {

        const auto prefix = InterfaceNameToPrefix(interfaceName);

        const auto commandNames = interfaceInfo.GetCommandNames();
        if (!commandNames.empty())
        {
            for (const auto& commandName : commandNames)
            {
                const std::string fullName = prefix + commandName;
                auto it = frameworkMessages.find(fullName);
                if (it != frameworkMessages.end() &&
                    it->second.Type == XFS4IoT::MessageTypeInfo::MessageTypeEnum::Command)
                {
                    target.emplace(fullName, it->second);
                }
            }


			RegisterCommand<
                XFS4IoT::Common::Commands::CapabilitiesCommand,
                XFS4IoTServer::Common::CapabilitiesHandler
			>(
                XFS4IoT::Common::Commands::CapabilitiesCommand::CommandName,
				{ XFS4IoT::Common::Commands::CapabilitiesCommand::Version, "3.0" },
				XFS4IoT::XFSConstants::ServiceClass::Common,
				true,
				serviceProvider
			);

            RegisterCommand<
                XFS4IoT::Common::Commands::SetVersionsCommand,
                XFS4IoTFramework::Common::SetVersionsHandler
            >(
                XFS4IoT::Common::Commands::SetVersionsCommand::CommandName,
                { XFS4IoT::Common::Commands::SetVersionsCommand::Version, "3.0" },
                XFS4IoT::XFSConstants::ServiceClass::Common,
                true,
                serviceProvider
            );

            RegisterCommand<
                XFS4IoT::CashManagement::Commands::ResetCommand,
                XFS4IoTFramework::CashManagement::ResetHandler
            >(
                XFS4IoT::CashManagement::Commands::ResetCommand::CommandName,
                { XFS4IoT::CashManagement::Commands::ResetCommand::Version, "3.0" },
                XFS4IoT::XFSConstants::ServiceClass::CashManagement,
                false,
                serviceProvider
            );

            RegisterCommand<
                XFS4IoT::CashAcceptor::Commands::ConfigureNoteTypesCommand,
                XFS4IoTFramework::CashAcceptor::ConfigureNoteTypesHandler
            >(
                XFS4IoT::CashAcceptor::Commands::ConfigureNoteTypesCommand::CommandName,
                { XFS4IoT::CashAcceptor::Commands::ConfigureNoteTypesCommand::Version, "3.0" },
                XFS4IoT::XFSConstants::ServiceClass::CashAcceptor,
                false,
                serviceProvider
            );

            RegisterCommand<
                XFS4IoT::CashManagement::Commands::GetBankNoteTypesCommand,
                XFS4IoTFramework::CashManagement::GetBankNoteTypesHandler
            >(
                XFS4IoT::CashManagement::Commands::GetBankNoteTypesCommand::CommandName,
                { XFS4IoT::CashManagement::Commands::GetBankNoteTypesCommand::Version, "3.0" },
                XFS4IoT::XFSConstants::ServiceClass::CashManagement,
                true,
                serviceProvider
            );

            RegisterCommand<
                XFS4IoT::Common::Commands::UnsupportedCommand,
                XFS4IoTFramework::Common::UnsupportedCommandHandler
            >(
                "UnsupportedCommand",
                { "1.0" },
                XFS4IoT::XFSConstants::ServiceClass::Common,
                true,
                serviceProvider
            );

            RegisterCommand<
                XFS4IoT::CashAcceptor::Commands::CashInStartCommand,
                XFS4IoTFramework::CashAcceptor::CashInStartHandler
            >(
                XFS4IoT::CashAcceptor::Commands::CashInStartCommand::CommandName,
                { XFS4IoT::CashAcceptor::Commands::CashInStartCommand::Version },
                XFS4IoT::XFSConstants::ServiceClass::CashAcceptor,
                false,
                serviceProvider
            );

            RegisterCommand<
                XFS4IoT::CashAcceptor::Commands::CashInCommand,
                XFS4IoTFramework::CashAcceptor::CashInHandler
            >(
                XFS4IoT::CashAcceptor::Commands::CashInCommand::CommandName,
                { XFS4IoT::CashAcceptor::Commands::CashInCommand::Version, "1.0"},
                XFS4IoT::XFSConstants::ServiceClass::CashAcceptor,
                false,
                serviceProvider
            );

        }

        const auto eventNames = interfaceInfo.GetEventNames();
        if (!eventNames.empty())
        {
            for (const auto& eventName : eventNames)
            {
                const std::string fullName = prefix + eventName;
                auto it = frameworkMessages.find(fullName);
                if (it != frameworkMessages.end() &&
                    it->second.Type == XFS4IoT::MessageTypeInfo::MessageTypeEnum::Event)
                {
                    target.emplace(fullName, it->second);
                }
            }
        }
    }

    template<typename MessageType, typename HandlerType>
    void CommonServiceClass::RegisterCommand(
        const std::string& name
        , std::vector<std::string> versions
        , XFS4IoT::XFSConstants::ServiceClass serviceClass
        , bool isAsync
        , std::shared_ptr<XFS4IoTServer::ServiceProvider> serviceProvider) const
    {
        decoder_->RegisterMessageType<MessageType>(name, versions);

        serviceProvider->RegisterHandler<MessageType, HandlerType>(
            serviceClass,
            isAsync);
    }

    std::string CommonServiceClass::InterfaceNameToPrefix(
        XFS4IoT::Common::InterfaceClass::NameEnum interfaceName)
    {
        switch (interfaceName)
        {
        case XFS4IoT::Common::InterfaceClass::NameEnum::Common:                 return "Common.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::CardReader:             return "CardReader.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::CashAcceptor:           return "CashAcceptor.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::CashDispenser:          return "CashDispenser.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::CashManagement:         return "CashManagement.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::PinPad:                 return "PinPad.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::Crypto:                 return "Crypto.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::KeyManagement:          return "KeyManagement.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::Keyboard:               return "Keyboard.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::TextTerminal:           return "TextTerminal.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::Printer:                return "Printer.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::BarcodeReader:          return "BarcodeReader.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::Camera:                 return "Camera.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::Lights:                 return "Lights.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::Auxiliaries:            return "Auxiliaries.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::VendorMode:             return "VendorMode.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::VendorApplication:      return "VendorApplication.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::Storage:                return "Storage.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::Biometric:              return "Biometric.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::Check:                  return "Check.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::MixedMedia:             return "MixedMedia.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::Deposit:                return "Deposit.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::German:                 return "German.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::BanknoteNeutralization: return "BanknoteNeutralization.";
        case XFS4IoT::Common::InterfaceClass::NameEnum::PowerManagement:        return "PowerManagement.";
        }

        throw std::invalid_argument("Unknown InterfaceClass::NameEnum value");
    }

    boost::asio::awaitable<void> CommonServiceClass::StatusChangedEvent(
        std::shared_ptr<XFS4IoT::Common::Events::StatusChangedEventPayloadData> payload)
    {
        if (!payload)
        {
            throw std::invalid_argument("CommonServiceClass::StatusChangedEvent: payload равен null");
        }

        auto eventMessage =
            std::make_shared<XFS4IoT::Common::Events::StatusChangedEvent>(payload);

        co_await serviceProvider_->BroadcastEvent(eventMessage);
    }

    boost::asio::awaitable<void> CommonServiceClass::ErrorEvent(
        std::shared_ptr<XFS4IoT::Common::Events::ErrorEventPayloadData> payload)
    {
        if (!payload)
        {
            throw std::invalid_argument("CommonServiceClass::ErrorEvent: payload равен null");
        }

        auto eventMessage =
            std::make_shared<XFS4IoT::Common::Events::ErrorEvent>(payload);

        co_await serviceProvider_->BroadcastEvent(eventMessage);
    }

    boost::asio::awaitable<void> CommonServiceClass::NonceClearedEvent(
        std::shared_ptr<XFS4IoT::Common::Events::NonceClearedEventPayloadData> payload)
    {
        if (!payload)
        {
            throw std::invalid_argument("CommonServiceClass::NonceClearedEvent: payload равен null");
        }

        auto eventMessage =
            std::make_shared<XFS4IoT::Common::Events::NonceClearedEvent>(payload);

        co_await serviceProvider_->BroadcastEvent(eventMessage);
    }

    boost::asio::awaitable<void> CommonServiceClass::NonceClearedEvent(
        const std::string& reasonDescription)
    {
        auto payload =
            std::make_shared<XFS4IoT::Common::Events::NonceClearedEventPayloadData>(
                reasonDescription);

        co_await NonceClearedEvent(payload);
    }

    XFS4IoT::Common::Events::ErrorEventPayloadData::EventIdEnum
        CommonServiceClass::MapErrorEventId(
            XFS4IoTFramework::Common::CommonStatusClass::ErrorEventIdEnum value)
    {
        using Source = XFS4IoTFramework::Common::CommonStatusClass::ErrorEventIdEnum;
        using Target = XFS4IoT::Common::Events::ErrorEventPayloadData::EventIdEnum;

        switch (value)
        {
        case Source::FraudAttempt: return Target::FraudAttempt;
        case Source::Hardware:     return Target::Hardware;
        case Source::Software:     return Target::Software;
        default:                   return Target::User;
        }
    }

    XFS4IoT::Common::Events::ErrorEventPayloadData::ActionEnum
        CommonServiceClass::MapErrorAction(
            XFS4IoTFramework::Common::CommonStatusClass::ErrorActionEnum value)
    {
        using Source = XFS4IoTFramework::Common::CommonStatusClass::ErrorActionEnum;
        using Target = XFS4IoT::Common::Events::ErrorEventPayloadData::ActionEnum;

        switch (value)
        {
        case Source::Clear:          return Target::Clear;
        case Source::Configuration:  return Target::Configuration;
        case Source::Maintenance:    return Target::Maintenance;
        case Source::Reset:          return Target::Reset;
        case Source::SoftwareError:  return Target::SoftwareError;
        default:                     return Target::Suspend;
        }
    }

    boost::asio::awaitable<void> CommonServiceClass::ErrorEvent(
        XFS4IoTFramework::Common::CommonStatusClass::ErrorEventIdEnum eventId,
        XFS4IoTFramework::Common::CommonStatusClass::ErrorActionEnum action,
        const std::string& vendorDescription)
    {
        auto payload =
            std::make_shared<XFS4IoT::Common::Events::ErrorEventPayloadData>(
                MapErrorEventId(eventId),
                MapErrorAction(action),
                vendorDescription);

        co_await ErrorEvent(payload);
    }

    boost::asio::awaitable<void> CommonServiceClass::StatusChangedEvent(
    std::shared_ptr<void> sender,
    std::shared_ptr<XFS4IoTServer::PropertyChangedEventArgs> propertyInfo)
{
    if (!sender)
    {
        throw std::invalid_argument(
            "CommonServiceClass::StatusChangedEvent: sender равен null");
    }

    if (!propertyInfo)
    {
        throw std::invalid_argument(
            "CommonServiceClass::StatusChangedEvent: propertyInfo равен null");
    }

    const std::string& propertyName = propertyInfo->getPropertyName().value_or("");

    // ------------------------------------------------------------------------
    // CommonStatusClass
    // ------------------------------------------------------------------------
    if (auto commonStatus =
        alias_cast<XFS4IoTFramework::Common::CommonStatusClass>(sender))
    {
        std::optional<XFS4IoT::Common::DeviceEnum> device = std::nullopt;
        if (propertyName == "Device")
        {
            switch (commonStatus->GetDevice())
            {
            case XFS4IoTFramework::Common::CommonStatusClass::DeviceEnum::DeviceBusy:
                device = XFS4IoT::Common::DeviceEnum::DeviceBusy; break;
            case XFS4IoTFramework::Common::CommonStatusClass::DeviceEnum::FraudAttempt:
                device = XFS4IoT::Common::DeviceEnum::FraudAttempt; break;
            case XFS4IoTFramework::Common::CommonStatusClass::DeviceEnum::HardwareError:
                device = XFS4IoT::Common::DeviceEnum::HardwareError; break;
            case XFS4IoTFramework::Common::CommonStatusClass::DeviceEnum::NoDevice:
                device = XFS4IoT::Common::DeviceEnum::NoDevice; break;
            case XFS4IoTFramework::Common::CommonStatusClass::DeviceEnum::Offline:
                device = XFS4IoT::Common::DeviceEnum::Offline; break;
            case XFS4IoTFramework::Common::CommonStatusClass::DeviceEnum::Online:
                device = XFS4IoT::Common::DeviceEnum::Online; break;
            case XFS4IoTFramework::Common::CommonStatusClass::DeviceEnum::PotentialFraud:
                device = XFS4IoT::Common::DeviceEnum::PotentialFraud; break;
            case XFS4IoTFramework::Common::CommonStatusClass::DeviceEnum::PowerOff:
                device = XFS4IoT::Common::DeviceEnum::PowerOff; break;
            case XFS4IoTFramework::Common::CommonStatusClass::DeviceEnum::UserError:
                device = XFS4IoT::Common::DeviceEnum::UserError; break;
            default:
                throw std::runtime_error(
                    std::format("Unexpected Device property value specified. {}",
                        static_cast<int>(commonStatus->GetDevice())));
            }
        }

        std::optional<XFS4IoT::Common::PositionStatusEnum> devicePosition = std::nullopt;
        if (propertyName == "DevicePosition")
        {
            switch (commonStatus->GetDevicePosition())
            {
            case XFS4IoTFramework::Common::CommonStatusClass::PositionStatusEnum::InPosition:
                devicePosition = XFS4IoT::Common::PositionStatusEnum::InPosition; break;
            case XFS4IoTFramework::Common::CommonStatusClass::PositionStatusEnum::NotInPosition:
                devicePosition = XFS4IoT::Common::PositionStatusEnum::NotInPosition; break;
            case XFS4IoTFramework::Common::CommonStatusClass::PositionStatusEnum::Unknown:
                devicePosition = XFS4IoT::Common::PositionStatusEnum::Unknown; break;
            default:
                throw std::runtime_error(
                    std::format("Unexpected DevicePosition property value specified. {}",
                        static_cast<int>(commonStatus->GetDevicePosition())));
            }
        }

        std::optional<int> powerSaveRecoveryTime = std::nullopt;
        if (propertyName == "PowerSaveRecoveryTime")
        {
            powerSaveRecoveryTime = commonStatus->GetPowerSaveRecoveryTime();
        }

        std::optional<XFS4IoT::Common::AntiFraudModuleEnum> antiFraudModule = std::nullopt;
        if (propertyName == "AntiFraudModule")
        {
            switch (commonStatus->GetAntiFraudModule())
            {
            case XFS4IoTFramework::Common::CommonStatusClass::AntiFraudModuleEnum::DeviceDetected:
                antiFraudModule = XFS4IoT::Common::AntiFraudModuleEnum::DeviceDetected; break;
            case XFS4IoTFramework::Common::CommonStatusClass::AntiFraudModuleEnum::Inoperable:
                antiFraudModule = XFS4IoT::Common::AntiFraudModuleEnum::Inoperable; break;
            case XFS4IoTFramework::Common::CommonStatusClass::AntiFraudModuleEnum::Ok:
                antiFraudModule = XFS4IoT::Common::AntiFraudModuleEnum::Ok; break;
            case XFS4IoTFramework::Common::CommonStatusClass::AntiFraudModuleEnum::Unknown:
                antiFraudModule = XFS4IoT::Common::AntiFraudModuleEnum::Unknown; break;
            case XFS4IoTFramework::Common::CommonStatusClass::AntiFraudModuleEnum::NotSupported:
                antiFraudModule = std::nullopt; break;
            default:
                throw std::runtime_error(
                    std::format("Unexpected AntiFraudModule property value specified. {}",
                        static_cast<int>(commonStatus->GetAntiFraudModule())));
            }
        }

        std::optional<XFS4IoT::Common::ExchangeEnum> exchange = std::nullopt;
        if (propertyName == "Exchange")
        {
            switch (commonStatus->GetExchange())
            {
            case XFS4IoTFramework::Common::CommonStatusClass::ExchangeEnum::Active:
                exchange = XFS4IoT::Common::ExchangeEnum::Active; break;
            case XFS4IoTFramework::Common::CommonStatusClass::ExchangeEnum::Inactive:
                exchange = XFS4IoT::Common::ExchangeEnum::Inactive; break;
            case XFS4IoTFramework::Common::CommonStatusClass::ExchangeEnum::NotSupported:
                exchange = std::nullopt; break;
            default:
                throw std::runtime_error(
                    std::format("Unexpected Exchange property value specified. {}",
                        static_cast<int>(commonStatus->GetExchange())));
            }
        }

        std::optional<XFS4IoT::Common::EndToEndSecurityStatusEnum> endToEndSecurity = std::nullopt;
        if (propertyName == "EndToEndSecurity")
        {
            switch (commonStatus->GetEndToEndSecurity())
            {
            case XFS4IoTFramework::Common::CommonStatusClass::EndToEndSecurityEnum::NotEnforced:
                endToEndSecurity = XFS4IoT::Common::EndToEndSecurityStatusEnum::NotEnforced; break;
            case XFS4IoTFramework::Common::CommonStatusClass::EndToEndSecurityEnum::NotConfigured:
                endToEndSecurity = XFS4IoT::Common::EndToEndSecurityStatusEnum::NotConfigured; break;
            case XFS4IoTFramework::Common::CommonStatusClass::EndToEndSecurityEnum::Enforced:
                endToEndSecurity = XFS4IoT::Common::EndToEndSecurityStatusEnum::Enforced; break;
            case XFS4IoTFramework::Common::CommonStatusClass::EndToEndSecurityEnum::NotSupported:
                endToEndSecurity = std::nullopt; break;
            default:
                throw std::runtime_error(
                    std::format("Unexpected EndToEndSecurity property value specified. {}",
                        static_cast<int>(commonStatus->GetEndToEndSecurity())));
            }
        }

        auto common =
            std::make_shared<XFS4IoT::Common::StatusPropertiesChangedClass>(
                device,
                devicePosition,
                powerSaveRecoveryTime,
                antiFraudModule,
                exchange,
                endToEndSecurity,
                nullptr);

        auto payload =
            std::make_shared<XFS4IoT::Common::Events::StatusChangedEventPayloadData>(
                common,
                nullptr,
                nullptr);

        co_await StatusChangedEvent(payload);
        co_return;
    }

    // ------------------------------------------------------------------------
    // CashManagementStatusClass
    // ------------------------------------------------------------------------
    if (auto cashManagementStatus =
        alias_cast<XFS4IoTFramework::Common::CashManagementStatusClass>(sender))
    {
        std::optional<XFS4IoT::CashManagement::StatusClass::DispenserEnum> dispenser = std::nullopt;
        if (propertyName == "Dispenser")
        {
            switch (cashManagementStatus->GetDispenser())
            {
            case XFS4IoTFramework::Common::CashManagementStatusClass::DispenserEnum::Ok:
                dispenser = XFS4IoT::CashManagement::StatusClass::DispenserEnum::Ok; break;
            case XFS4IoTFramework::Common::CashManagementStatusClass::DispenserEnum::Unknown:
                dispenser = XFS4IoT::CashManagement::StatusClass::DispenserEnum::Unknown; break;
            case XFS4IoTFramework::Common::CashManagementStatusClass::DispenserEnum::Attention:
                dispenser = XFS4IoT::CashManagement::StatusClass::DispenserEnum::Attention; break;
            case XFS4IoTFramework::Common::CashManagementStatusClass::DispenserEnum::Stop:
                dispenser = XFS4IoT::CashManagement::StatusClass::DispenserEnum::Stop; break;
            case XFS4IoTFramework::Common::CashManagementStatusClass::DispenserEnum::NotSupported:
                dispenser = std::nullopt; break;
            default:
                dispenser = std::nullopt; break;
            }
        }

        std::optional<XFS4IoT::CashManagement::StatusClass::AcceptorEnum> acceptor = std::nullopt;
        if (propertyName == "Acceptor")
        {
            switch (cashManagementStatus->GetAcceptor())
            {
            case XFS4IoTFramework::Common::CashManagementStatusClass::AcceptorEnum::Ok:
                acceptor = XFS4IoT::CashManagement::StatusClass::AcceptorEnum::Ok; break;
            case XFS4IoTFramework::Common::CashManagementStatusClass::AcceptorEnum::Unknown:
                acceptor = XFS4IoT::CashManagement::StatusClass::AcceptorEnum::Unknown; break;
            case XFS4IoTFramework::Common::CashManagementStatusClass::AcceptorEnum::Attention:
                acceptor = XFS4IoT::CashManagement::StatusClass::AcceptorEnum::Attention; break;
            case XFS4IoTFramework::Common::CashManagementStatusClass::AcceptorEnum::Stop:
                acceptor = XFS4IoT::CashManagement::StatusClass::AcceptorEnum::Stop; break;
            case XFS4IoTFramework::Common::CashManagementStatusClass::AcceptorEnum::NotSupported:
                acceptor = std::nullopt; break;
            default:
                acceptor = std::nullopt; break;
            }
        }

        auto cashManagement =
            std::make_shared<XFS4IoT::CashManagement::StatusClass>(
                dispenser,
                acceptor);

        auto payload =
            std::make_shared<XFS4IoT::Common::Events::StatusChangedEventPayloadData>(
                nullptr,
                nullptr,
                cashManagement);

        co_await StatusChangedEvent(payload);
        co_return;
    }

    // ------------------------------------------------------------------------
    // CashAcceptorStatusClass
    // ------------------------------------------------------------------------
    if (auto cashAcceptor =
        alias_cast<XFS4IoTFramework::Common::CashAcceptorStatusClass>(sender))
    {
        std::optional<XFS4IoT::CashAcceptor::StatusClass::IntermediateStackerEnum> intermediateStacker = std::nullopt;
        if (propertyName == "IntermediateStacker")
        {
            switch (cashAcceptor->GetIntermediateStacker())
            {
            case XFS4IoTFramework::Common::CashAcceptorStatusClass::IntermediateStackerEnum::Full:
                intermediateStacker = XFS4IoT::CashAcceptor::StatusClass::IntermediateStackerEnum::Full; break;
            case XFS4IoTFramework::Common::CashAcceptorStatusClass::IntermediateStackerEnum::Empty:
                intermediateStacker = XFS4IoT::CashAcceptor::StatusClass::IntermediateStackerEnum::Empty; break;
            case XFS4IoTFramework::Common::CashAcceptorStatusClass::IntermediateStackerEnum::NotEmpty:
                intermediateStacker = XFS4IoT::CashAcceptor::StatusClass::IntermediateStackerEnum::NotEmpty; break;
            case XFS4IoTFramework::Common::CashAcceptorStatusClass::IntermediateStackerEnum::Unknown:
                intermediateStacker = XFS4IoT::CashAcceptor::StatusClass::IntermediateStackerEnum::Unknown; break;
            case XFS4IoTFramework::Common::CashAcceptorStatusClass::IntermediateStackerEnum::NotSupported:
                intermediateStacker = std::nullopt; break;
            default:
                throw std::runtime_error(
                    std::format("Unsupported intermediate stacker status specified. {}",
                        static_cast<int>(cashAcceptor->GetIntermediateStacker())));
            }
        }

        std::optional<XFS4IoT::CashAcceptor::StatusClass::StackerItemsEnum> stackerItems = std::nullopt;
        if (propertyName == "StackerItems")
        {
            switch (cashAcceptor->GetStackerItems())
            {
            case XFS4IoTFramework::Common::CashAcceptorStatusClass::StackerItemsEnum::AccessUnknown:
                stackerItems = XFS4IoT::CashAcceptor::StatusClass::StackerItemsEnum::AccessUnknown; break;
            case XFS4IoTFramework::Common::CashAcceptorStatusClass::StackerItemsEnum::NoCustomerAccess:
                stackerItems = XFS4IoT::CashAcceptor::StatusClass::StackerItemsEnum::NoCustomerAccess; break;
            case XFS4IoTFramework::Common::CashAcceptorStatusClass::StackerItemsEnum::CustomerAccess:
                stackerItems = XFS4IoT::CashAcceptor::StatusClass::StackerItemsEnum::CustomerAccess; break;
            case XFS4IoTFramework::Common::CashAcceptorStatusClass::StackerItemsEnum::NoItems:
                stackerItems = XFS4IoT::CashAcceptor::StatusClass::StackerItemsEnum::NoItems; break;
            case XFS4IoTFramework::Common::CashAcceptorStatusClass::StackerItemsEnum::NotSupported:
                stackerItems = std::nullopt; break;
            default:
                throw std::runtime_error(
                    std::format("Unsupported stacker item status specified. {}",
                        static_cast<int>(cashAcceptor->GetStackerItems())));
            }
        }

        std::optional<XFS4IoT::CashAcceptor::StatusClass::BanknoteReaderEnum> banknoteReader = std::nullopt;
        if (propertyName == "BanknoteReader")
        {
            switch (cashAcceptor->GetBanknoteReader())
            {
            case XFS4IoTFramework::Common::CashAcceptorStatusClass::BanknoteReaderEnum::Ok:
                banknoteReader = XFS4IoT::CashAcceptor::StatusClass::BanknoteReaderEnum::Ok; break;
            case XFS4IoTFramework::Common::CashAcceptorStatusClass::BanknoteReaderEnum::Inoperable:
                banknoteReader = XFS4IoT::CashAcceptor::StatusClass::BanknoteReaderEnum::Inoperable; break;
            case XFS4IoTFramework::Common::CashAcceptorStatusClass::BanknoteReaderEnum::Unknown:
                banknoteReader = XFS4IoT::CashAcceptor::StatusClass::BanknoteReaderEnum::Unknown; break;
            case XFS4IoTFramework::Common::CashAcceptorStatusClass::BanknoteReaderEnum::NotSupported:
                banknoteReader = std::nullopt; break;
            default:
                throw std::runtime_error(
                    std::format("Unsupported banknote reader status specified. {}",
                        static_cast<int>(cashAcceptor->GetBanknoteReader())));
            }
        }

        std::optional<bool> dropBox = std::nullopt;
        if (propertyName == "DropBox")
        {
            dropBox = cashAcceptor->GetDropBox();
        }

        auto cashAcceptorStatus =
            std::make_shared<XFS4IoT::CashAcceptor::StatusClass>(
                intermediateStacker,
                stackerItems,
                banknoteReader,
                dropBox);

        auto payload =
            std::make_shared<XFS4IoT::Common::Events::StatusChangedEventPayloadData>(
                nullptr,
                cashAcceptorStatus,
                nullptr);

        co_await StatusChangedEvent(payload);
        co_return;
    }

    // ------------------------------------------------------------------------
    // CashManagementStatusClass::PositionStatusClass
    // ------------------------------------------------------------------------
    if (auto positionStatus =
        alias_cast<XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusClass>(sender))
    {
        if (positionStatus->GetCashAcceptorPosition().has_value())
        {
            std::optional<XFS4IoT::CashAcceptor::PositionClass::ShutterEnum> shutter = std::nullopt;
            if (propertyName == "Shutter")
            {
                switch (positionStatus->GetShutter())
                {
                case XFS4IoTFramework::Common::CashManagementStatusClass::ShutterEnum::Open:
                    shutter = XFS4IoT::CashAcceptor::PositionClass::ShutterEnum::Open; break;
                case XFS4IoTFramework::Common::CashManagementStatusClass::ShutterEnum::Closed:
                    shutter = XFS4IoT::CashAcceptor::PositionClass::ShutterEnum::Closed; break;
                case XFS4IoTFramework::Common::CashManagementStatusClass::ShutterEnum::JammedClosed:
                    shutter = XFS4IoT::CashAcceptor::PositionClass::ShutterEnum::JammedClosed; break;
                case XFS4IoTFramework::Common::CashManagementStatusClass::ShutterEnum::JammedPartiallyOpen:
                    shutter = XFS4IoT::CashAcceptor::PositionClass::ShutterEnum::JammedPartiallyOpen; break;
                case XFS4IoTFramework::Common::CashManagementStatusClass::ShutterEnum::JammedUnknown:
                    shutter = XFS4IoT::CashAcceptor::PositionClass::ShutterEnum::JammedUnknown; break;
                case XFS4IoTFramework::Common::CashManagementStatusClass::ShutterEnum::Unknown:
                    shutter = XFS4IoT::CashAcceptor::PositionClass::ShutterEnum::Unknown; break;
                case XFS4IoTFramework::Common::CashManagementStatusClass::ShutterEnum::NotSupported:
                    shutter = std::nullopt; break;
                case XFS4IoTFramework::Common::CashManagementStatusClass::ShutterEnum::JammedOpen:
                    shutter = std::nullopt; break; // если в XFS4IoT::CashAcceptor::PositionClass нет JammedOpen
                default:
                    throw std::runtime_error(
                        std::format("Unsupported shutter status specified. {}",
                            static_cast<int>(positionStatus->GetShutter())));
                }
            }

            std::optional<XFS4IoT::CashAcceptor::PositionClass::PositionStatusEnum> positionState = std::nullopt;
            if (propertyName == "PositionStatus")
            {
                switch (positionStatus->GetPositionStatus())
                {
                case XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::Empty:
                    positionState = XFS4IoT::CashAcceptor::PositionClass::PositionStatusEnum::Empty; break;
                case XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::NotEmpty:
                    positionState = XFS4IoT::CashAcceptor::PositionClass::PositionStatusEnum::NotEmpty; break;
                case XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::Unknown:
                    positionState = XFS4IoT::CashAcceptor::PositionClass::PositionStatusEnum::Unknown; break;
                case XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::NotSupported:
                    positionState = std::nullopt; break;
                default:
                    throw std::runtime_error(
                        std::format("Unsupported position status specified. {}",
                            static_cast<int>(positionStatus->GetPositionStatus())));
                }
            }

            std::optional<XFS4IoT::CashAcceptor::PositionClass::TransportEnum> transport = std::nullopt;
            if (propertyName == "Transport")
            {
                switch (positionStatus->GetTransport())
                {
                case XFS4IoTFramework::Common::CashManagementStatusClass::TransportEnum::Ok:
                    transport = XFS4IoT::CashAcceptor::PositionClass::TransportEnum::Ok; break;
                case XFS4IoTFramework::Common::CashManagementStatusClass::TransportEnum::Inoperative:
                    transport = XFS4IoT::CashAcceptor::PositionClass::TransportEnum::Inoperative; break;
                case XFS4IoTFramework::Common::CashManagementStatusClass::TransportEnum::Unknown:
                    transport = XFS4IoT::CashAcceptor::PositionClass::TransportEnum::Unknown; break;
                case XFS4IoTFramework::Common::CashManagementStatusClass::TransportEnum::NotSupported:
                    transport = std::nullopt; break;
                default:
                    throw std::runtime_error(
                        std::format("Unsupported transport specified. {}",
                            static_cast<int>(positionStatus->GetTransport())));
                }
            }

            std::optional<XFS4IoT::CashAcceptor::PositionClass::TransportStatusEnum> transportStatus = std::nullopt;
            if (propertyName == "TransportStatus")
            {
                switch (positionStatus->GetTransportStatus())
                {
                case XFS4IoTFramework::Common::CashManagementStatusClass::TransportStatusEnum::Empty:
                    transportStatus = XFS4IoT::CashAcceptor::PositionClass::TransportStatusEnum::Empty; break;
                case XFS4IoTFramework::Common::CashManagementStatusClass::TransportStatusEnum::NotEmpty:
                    transportStatus = XFS4IoT::CashAcceptor::PositionClass::TransportStatusEnum::NotEmpty; break;
                case XFS4IoTFramework::Common::CashManagementStatusClass::TransportStatusEnum::NotEmptyCustomer:
                    transportStatus = XFS4IoT::CashAcceptor::PositionClass::TransportStatusEnum::NotEmptyCustomer; break;
                case XFS4IoTFramework::Common::CashManagementStatusClass::TransportStatusEnum::Unknown:
                    transportStatus = XFS4IoT::CashAcceptor::PositionClass::TransportStatusEnum::Unknown; break;
                case XFS4IoTFramework::Common::CashManagementStatusClass::TransportStatusEnum::NotSupported:
                    transportStatus = std::nullopt; break;
                default:
                    throw std::runtime_error(
                        std::format("Unsupported transport status specified. {}",
                            static_cast<int>(positionStatus->GetTransportStatus())));
                }
            }

            using PositionEnum = XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum;
            const auto pos = positionStatus->GetCashAcceptorPosition().value();

            const std::vector<PositionEnum> allPositions =
            {
                PositionEnum::InDefault,
                PositionEnum::InLeft,
                PositionEnum::InRight,
                PositionEnum::InCenter,
                PositionEnum::InTop,
                PositionEnum::InBottom,
                PositionEnum::InFront,
                PositionEnum::InRear,
                PositionEnum::OutDefault,
                PositionEnum::OutLeft,
                PositionEnum::OutRight,
                PositionEnum::OutCenter,
                PositionEnum::OutTop,
                PositionEnum::OutBottom,
                PositionEnum::OutFront,
                PositionEnum::OutRear
            };

            std::vector<PositionEnum> positions;
            for (const auto& e : allPositions)
            {
                if (XFS4IoT::HasFlag(pos, e))
                {
                    positions.push_back(e);
                }
            }

            std::vector< std::shared_ptr<XFS4IoT::CashAcceptor::PositionClass>> positionsStatus;
            if (!positions.empty())
            {

                for (const auto& position : positions)
                {
                    std::optional <XFS4IoT::CashManagement::PositionEnum> mappedPosition;

                    switch (position)
                    {
                    case PositionEnum::InBottom:   mappedPosition = XFS4IoT::CashManagement::PositionEnum::InBottom; break;
                    case PositionEnum::InRight:    mappedPosition = XFS4IoT::CashManagement::PositionEnum::InRight; break;
                    case PositionEnum::InFront:    mappedPosition = XFS4IoT::CashManagement::PositionEnum::InFront; break;
                    case PositionEnum::InRear:     mappedPosition = XFS4IoT::CashManagement::PositionEnum::InRear; break;
                    case PositionEnum::InLeft:     mappedPosition = XFS4IoT::CashManagement::PositionEnum::InLeft; break;
                    case PositionEnum::InCenter:   mappedPosition = XFS4IoT::CashManagement::PositionEnum::InCenter; break;
                    case PositionEnum::InTop:      mappedPosition = XFS4IoT::CashManagement::PositionEnum::InTop; break;
                    case PositionEnum::InDefault:  mappedPosition = XFS4IoT::CashManagement::PositionEnum::InDefault; break;
                    case PositionEnum::OutBottom:  mappedPosition = XFS4IoT::CashManagement::PositionEnum::OutBottom; break;
                    case PositionEnum::OutRight:   mappedPosition = XFS4IoT::CashManagement::PositionEnum::OutRight; break;
                    case PositionEnum::OutFront:   mappedPosition = XFS4IoT::CashManagement::PositionEnum::OutFront; break;
                    case PositionEnum::OutRear:    mappedPosition = XFS4IoT::CashManagement::PositionEnum::OutRear; break;
                    case PositionEnum::OutLeft:    mappedPosition = XFS4IoT::CashManagement::PositionEnum::OutLeft; break;
                    case PositionEnum::OutTop:     mappedPosition = XFS4IoT::CashManagement::PositionEnum::OutTop; break;
                    case PositionEnum::OutCenter:  mappedPosition = XFS4IoT::CashManagement::PositionEnum::OutCenter; break;
                    case PositionEnum::OutDefault: mappedPosition = XFS4IoT::CashManagement::PositionEnum::OutDefault; break;
                    default:
                        throw std::runtime_error(
                            std::format("Unsupported cash management position specified. {}",
                                static_cast<int>(position)));
                    }

					positionsStatus.emplace_back(std::make_shared<XFS4IoT::CashAcceptor::PositionClass>(
                        mappedPosition,
                        shutter,
                        positionState,
                        transport,
                        transportStatus));
                }
            }

            auto cashAcceptorStatus =
                std::make_shared<XFS4IoT::CashAcceptor::StatusClass>(
                    std::nullopt,
                    std::nullopt,
                    std::nullopt,
                    std::nullopt,
                    positionsStatus);

            auto payload =
                std::make_shared<XFS4IoT::Common::Events::StatusChangedEventPayloadData>(
                    nullptr,
                    cashAcceptorStatus,
                    nullptr);

            co_await StatusChangedEvent(payload);
            co_return;
        }
    }

    logger_->warn(
        std::format("CommonServiceClass::StatusChangedEvent: неожиданный тип отправителя для свойства '{}'",
            propertyName));

    co_return;
}

    void CommonServiceClass::StatusChangedEventForwarder(
        std::shared_ptr<void> sender,
        std::shared_ptr<XFS4IoTServer::PropertyChangedEventArgs> propertyInfo)
    {
        if (!serviceProvider_)
        {
            return;
        }

        auto& ioContext = serviceProvider_->getIoContext();

        boost::asio::co_spawn(
            ioContext,
            StatusChangedEvent(std::move(sender), std::move(propertyInfo)),
            boost::asio::detached);
    }
}
