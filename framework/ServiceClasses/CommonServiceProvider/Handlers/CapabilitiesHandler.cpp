#include "CapabilitiesHandler.hpp"

#include <stdexcept>
#include <utility>
#include "../../../core/Exceptions.hpp"
#include "../../../../Devices/CashAcceptorSample/CashAcceptorSample.hpp"
#include "../../HasFlagHelper.hpp"

//#include "../Common/Events/CapabilitiesEvents.hpp"

namespace XFS4IoTServer::Common
{
    using XFS4IoT::Common::Commands::CapabilitiesCommand;
    using XFS4IoT::Common::Completions::CapabilitiesCompletion;
    using XFS4IoT::Common::InterfaceClass;

    CapabilitiesHandler::CapabilitiesHandler(
        std::shared_ptr<IConnection> connection,
        std::shared_ptr<ICommandDispatcher> dispatcher,
        std::shared_ptr<ILogger> logger)
        : connection_(std::move(connection))
        , logger_(std::move(logger))
    {
        if (!connection_) {
            throw std::invalid_argument("CapabilitiesHandler: соединение равно null");
        }
        if (!dispatcher) {
            throw std::invalid_argument("CapabilitiesHandler: диспетчер равен null");
        }
        if (!logger_) {
            throw std::invalid_argument("CapabilitiesHandler: логгер равен null");
        }

        provider_ = std::dynamic_pointer_cast<IServiceProvider>(dispatcher);
        if (!provider_) {
            throw std::runtime_error("CapabilitiesHandler: диспетчер не является IServiceProvider");
        }

        if (!provider_->GetDevice()) {
            throw std::runtime_error("CapabilitiesHandler: устройство провайдера равно null");
        }

        // Проверка, что устройство реализует ICommonDevice
        auto commonDevice = std::dynamic_pointer_cast<XFS4IoTFramework::Common::ICommonDevice>(provider_->GetDevice());
        if (!commonDevice) {
            throw std::runtime_error("CapabilitiesHandler: устройство провайдера не является ICommonDevice");
        }

        // Проверка, что сам provider / dispatcher реализует ICommonService
        common_ = std::dynamic_pointer_cast<XFS4IoTFramework::Common::ICommonService>(provider_);
        if (!common_) {
            throw std::runtime_error("CapabilitiesHandler: провайдер не является ICommonService");
        }
    }

    boost::asio::awaitable<void> CapabilitiesHandler::Handle(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::stop_token cancel)
    {
        logger_->trace("CapabilitiesHandler::Handle запущен");
        if (!command) {
            throw std::invalid_argument("CapabilitiesHandler::Handle: команда равна null");
        }

        auto capabilitiesCmd =
            std::dynamic_pointer_cast<CapabilitiesCommand>(command);

        if (!capabilitiesCmd) {
            throw std::runtime_error("CapabilitiesHandler::Handle: неожиданный тип команды");
        }

        const auto& header = capabilitiesCmd->Header();
        if (!header.RequestId().has_value()) {
            throw std::runtime_error("CapabilitiesHandler::Handle: requestId отсутствует");
        }

        auto result = co_await HandleCapabilities(capabilitiesCmd, cancel);

        auto response =
            std::make_shared<CapabilitiesCompletion>(
                header.RequestId().value(),
                result.payload,
                result.completionCode,
                result.errorDescription);

        logger_->trace(std::format("Capabilities выполнен json: {}", response->Serialise()));
        co_await connection_->SendMessageAsync(response);

        // Если у тебя есть постобработка как в C# ICommandHandler.CommandPostProcessing(result),
        // добавишь сюда.
        co_return;
    }

    boost::asio::awaitable<void> CapabilitiesHandler::HandleError(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::exception_ptr commandErrorException)
    {
        if (!command) {
            throw std::invalid_argument("CapabilitiesHandler::HandleError: команда равна null");
        }
        if (!commandErrorException) {
            throw std::invalid_argument("CapabilitiesHandler::HandleError: исключение равно null");
        }

        auto capabilitiesCmd =
            std::dynamic_pointer_cast<CapabilitiesCommand>(command);

        if (!capabilitiesCmd) {
            throw std::runtime_error("CapabilitiesHandler::HandleError: неожиданный тип команды");
        }

        const auto& header = capabilitiesCmd->Header();
        if (!header.RequestId().has_value()) {
            throw std::runtime_error("CapabilitiesHandler::HandleError: requestId отсутствует");
        }

        XFS4IoT::MessageHeader::CompletionCodeEnum errorCode =
            XFS4IoT::MessageHeader::CompletionCodeEnum::InternalError;
        std::string errorDescription = "Unknown error";

        try
        {
            std::rethrow_exception(commandErrorException);
        }
        catch (const std::exception& ex)
        {
            errorCode = MapExceptionToCompletionCode(ex);
            errorDescription = ex.what();
        }

        auto response =
            std::make_shared<CapabilitiesCompletion>(
                header.RequestId().value(),
                nullptr,
                errorCode,
                errorDescription);

        co_await connection_->SendMessageAsync(response);
    }

    boost::asio::awaitable<CapabilitiesHandler::CapabilitiesResult>
        CapabilitiesHandler::HandleCapabilities(
            std::shared_ptr<CapabilitiesCommand> /*capabilities*/,
            std::stop_token /*cancel*/)
    {
        // ВАЖНО:
        // Ниже я перевожу логику 1-в-1.
        // Но имена геттеров Common/CommonCapabilities/CashManagementCapabilities/CashAcceptorCapabilities
        // тебе нужно подставить под свои реальные C++ классы.

        // Примерно предполагается:
        // auto commonCaps = common_->GetCommonCapabilities();
        // auto cashMgmtCaps = common_->GetCashManagementCapabilities();
        // auto cashAcceptorCaps = common_->GetCashAcceptorCapabilities();

        auto commonCaps = common_->GetCommonCapabilities();
        if (!commonCaps) {
            co_return CapabilitiesResult{
                nullptr,
                XFS4IoT::MessageHeader::CompletionCodeEnum::InternalError,
                "Класс устройства не сообщает об общих возможностях."
            };
        }

        std::vector<InterfaceClass::NameEnum> supportedInterfaces;
        supportedInterfaces.emplace_back(InterfaceClass::NameEnum::Common);

        if (commonCaps->GetCashManagementInterface()) {
            supportedInterfaces.emplace_back(InterfaceClass::NameEnum::CashManagement);
        }

        if (commonCaps->GetCashAcceptorInterface()) {
            supportedInterfaces.emplace_back(InterfaceClass::NameEnum::CashAcceptor);
        }

        std::vector<std::shared_ptr<InterfaceClass>> interfaces;
        for (const auto iface : supportedInterfaces)
        {
            auto devInterface = GetDeviceInterface(iface);
            if (devInterface) {
                interfaces.emplace_back(std::move(devInterface));
            }
        }

        std::optional<std::vector<XFS4IoT::Common::DeviceInformationClass>> deviceInformation;
        if (!commonCaps->GetDeviceInformation().empty())
        {
            std::vector<XFS4IoT::Common::DeviceInformationClass> infoList;

            for (const auto& device : commonCaps->GetDeviceInformation())
            {
                std::optional<std::vector<XFS4IoT::Common::FirmwareClass>> firmware;
                if (!device.GetFirmware().empty())
                {
                    std::vector<XFS4IoT::Common::FirmwareClass> firmwareList;

                    for (const auto& firm : device.GetFirmware())
                    {
                        firmwareList.emplace_back(
                            firm.GetFirmwareName(),
                            firm.GetFirmwareVersion(),
                            firm.GetHardwareRevision());
                    }

                    firmware = std::move(firmwareList);
                }

                std::optional<std::vector<XFS4IoT::Common::SoftwareClass>> software;
                if (!device.GetSoftware().empty())
                {
                    std::vector<XFS4IoT::Common::SoftwareClass> softwareList;

                    for (const auto& soft : device.GetSoftware())
                    {
                        softwareList.emplace_back(
                            soft.GetSoftwareName(),
                            soft.GetSoftwareVersion());
                    }

                    software = std::move(softwareList);
                }

                infoList.emplace_back(
                    device.GetModelName(),
                    device.GetSerialNumber(),
                    device.GetRevisionNumber(),
                    device.GetModelDescription(),
                    firmware,
                    software);
            }

            deviceInformation = std::move(infoList);
        }

        std::shared_ptr<XFS4IoT::Common::EndToEndSecurityClass> endToEndSecurity = nullptr;
		if (false) // Пока оставим закомментированным, так как в нашем примере нет поддержки EndToEndSecurity. 
        //if (commonCaps->GetEndToEndSecurity())
        {
            std::optional<XFS4IoT::Common::EndToEndSecurityClass::RequiredEnum> required = std::nullopt;
            switch (commonCaps->GetEndToEndSecurity()->GetRequired().value_or(
                decltype(commonCaps->GetEndToEndSecurity()->GetRequired())::value_type{}))
            {
            case XFS4IoTFramework::Common::CommonCapabilitiesClass::EndToEndSecurityClass::RequiredEnum::Always:
                required = XFS4IoT::Common::EndToEndSecurityClass::RequiredEnum::Always;
                break;
            case XFS4IoTFramework::Common::CommonCapabilitiesClass::EndToEndSecurityClass::RequiredEnum::IfConfigured:
                required = XFS4IoT::Common::EndToEndSecurityClass::RequiredEnum::IfConfigured;
                break;
            default:
                break;
            }

            std::optional<XFS4IoT::Common::EndToEndSecurityClass::ResponseSecurityEnabledEnum> responseSecurity = std::nullopt;
            switch (commonCaps->GetEndToEndSecurity()->GetResponseSecurityEnabled().value_or(
                decltype(commonCaps->GetEndToEndSecurity()->GetResponseSecurityEnabled())::value_type{}))
            {
            case XFS4IoTFramework::Common::CommonCapabilitiesClass::EndToEndSecurityClass::ResponseSecurityEnabledEnum::Always:
                responseSecurity = XFS4IoT::Common::EndToEndSecurityClass::ResponseSecurityEnabledEnum::Always;
                break;
            case XFS4IoTFramework::Common::CommonCapabilitiesClass::EndToEndSecurityClass::ResponseSecurityEnabledEnum::IfConfigured:
                responseSecurity = XFS4IoT::Common::EndToEndSecurityClass::ResponseSecurityEnabledEnum::IfConfigured;
                break;
            default:
                break;
            }

            endToEndSecurity = std::make_shared<XFS4IoT::Common::EndToEndSecurityClass>(
                required,
                commonCaps->GetEndToEndSecurity()->GetHardwareSecurityElement(),
                responseSecurity,
                std::nullopt,
                commonCaps->GetEndToEndSecurity()->GetCommandNonceTimeout());
        }

        std::shared_ptr<XFS4IoT::Common::PersistentDataStoreCapabilityClass> persistentDataStore = 
            std::make_shared<XFS4IoT::Common::PersistentDataStoreCapabilityClass>(UINT32_MAX);

        auto commonCapabilities =
            std::make_shared<XFS4IoT::Common::CapabilityPropertiesClass>(
                std::string(XFS4IoT::XFSConstants::ServiceVersion),
                deviceInformation,
                commonCaps->GetPowerSaveControl(),
                commonCaps->GetAntiFraudModule(),
                endToEndSecurity,
                persistentDataStore);

        std::shared_ptr<XFS4IoT::CashManagement::CapabilitiesClass> cashManagement = nullptr;
        auto cashMgmtCaps = common_->GetCashManagementCapabilities();
        if (cashMgmtCaps)
        {
            std::shared_ptr<XFS4IoT::CashManagement::CapabilitiesClass::ExchangeTypeClass> exchangeType = nullptr;
            if (cashMgmtCaps->GetExchangeTypes() !=
                XFS4IoTFramework::Common::CashManagementCapabilitiesClass::ExchangeTypesEnum::NotSupported)
            {
                using ItemEnum = XFS4IoTFramework::Common::CashManagementCapabilitiesClass::ExchangeTypesEnum;
                exchangeType =
                    std::make_shared<XFS4IoT::CashManagement::CapabilitiesClass::ExchangeTypeClass>(
                        XFS4IoT::HasFlag(
                                cashMgmtCaps->GetExchangeTypes(),
                            std::make_optional<ItemEnum>(ItemEnum::ByHand)));
            }

            std::shared_ptr<XFS4IoT::CashManagement::CapabilitiesClass::ItemInfoTypesClass> itemInfoTypes = nullptr;
            if (cashMgmtCaps->GetItemInfoTypes() !=
                XFS4IoTFramework::Common::CashManagementCapabilitiesClass::ItemInfoTypesEnum::NotSupported)
            {
                using ItemEnum = XFS4IoTFramework::Common::CashManagementCapabilitiesClass::ItemInfoTypesEnum;
                itemInfoTypes =
                    std::make_shared<XFS4IoT::CashManagement::CapabilitiesClass::ItemInfoTypesClass>(
                        XFS4IoT::HasFlag(
                            cashMgmtCaps->GetItemInfoTypes(),
                            std::make_optional<ItemEnum>(ItemEnum::SerialNumber)),
                        XFS4IoT::HasFlag(
                            cashMgmtCaps->GetItemInfoTypes(),
                            std::make_optional<ItemEnum>(ItemEnum::Signature)),
                        XFS4IoT::HasFlag(
                            cashMgmtCaps->GetItemInfoTypes(),
                            std::make_optional<ItemEnum>(ItemEnum::ImageFile)));
            }

            cashManagement =
                std::make_shared<XFS4IoT::CashManagement::CapabilitiesClass>(
                    cashMgmtCaps->GetCashBox(),
                    exchangeType,
                    itemInfoTypes,
                    cashMgmtCaps->GetClassificationList(),
                    std::make_shared<XFS4IoT::CashManagement::CapabilitiesClass::ClassificationsClass>(
						true, // unrecognized
						false, // counterfeit
						false, // suspect
						false, // inked
                        true, // fit
						false // unfit
                    ));
        }

        std::shared_ptr<XFS4IoT::CashAcceptor::CapabilitiesClass> cashAcceptor = nullptr;
        auto cashAcceptorCaps = common_->GetCashAcceptorCapabilities();
        if (cashAcceptorCaps)
        {
            std::optional<std::vector<XFS4IoT::CashAcceptor::PosCapsClass>> positions;

            if (cashAcceptorCaps->GetPositions().has_value())
            {
                std::vector<XFS4IoT::CashAcceptor::PosCapsClass> posList;

                for (const auto& [key, value] : *cashAcceptorCaps->GetPositions())
                {
                    std::optional<XFS4IoT::CashManagement::PositionEnum> convertedPosition;
                    switch (key)
                    {
                    case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::OutLeft:
                        convertedPosition = XFS4IoT::CashManagement::PositionEnum::OutLeft; break;
                    case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::OutRight:
                        convertedPosition = XFS4IoT::CashManagement::PositionEnum::OutRight; break;
                    case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::OutCenter:
                        convertedPosition = XFS4IoT::CashManagement::PositionEnum::OutCenter; break;
                    case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::OutTop:
                        convertedPosition = XFS4IoT::CashManagement::PositionEnum::OutTop; break;
                    case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::OutBottom:
                        convertedPosition = XFS4IoT::CashManagement::PositionEnum::OutBottom; break;
                    case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::OutFront:
                        convertedPosition = XFS4IoT::CashManagement::PositionEnum::OutFront; break;
                    case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::OutRear:
                        convertedPosition = XFS4IoT::CashManagement::PositionEnum::OutRear; break;
                    case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::InDefault:
                        convertedPosition = XFS4IoT::CashManagement::PositionEnum::InDefault; break;
                    case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::InLeft:
                        convertedPosition = XFS4IoT::CashManagement::PositionEnum::InLeft; break;
                    case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::InRight:
                        convertedPosition = XFS4IoT::CashManagement::PositionEnum::InRight; break;
                    case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::InCenter:
                        convertedPosition = XFS4IoT::CashManagement::PositionEnum::InCenter; break;
                    case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::InTop:
                        convertedPosition = XFS4IoT::CashManagement::PositionEnum::InTop; break;
                    case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::InBottom:
                        convertedPosition = XFS4IoT::CashManagement::PositionEnum::InBottom; break;
                    case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::InFront:
                        convertedPosition = XFS4IoT::CashManagement::PositionEnum::InFront; break;
                    case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::InRear:
                        convertedPosition = XFS4IoT::CashManagement::PositionEnum::InRear; break;
                    default:
                        throw XFS4IoT::InvalidDataException(
                            std::format("Сообщено о неожиданной позиции для класса устройства приёмника наличных. {}", static_cast<int>(key)));
                    }

                    std::shared_ptr<XFS4IoT::CashAcceptor::PosCapsClass::UsageClass> usage = nullptr;
                    if (value.GetUsage() !=
                        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::NotSupported)
                    {
                        usage =
                            std::make_shared<XFS4IoT::CashAcceptor::PosCapsClass::UsageClass>(
                                XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::HasFlag(
                                    value.GetUsage(),
                                    XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::In),
                                XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::HasFlag(
                                    value.GetUsage(),
                                    XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::Refuse),
                                XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::HasFlag(
                                    value.GetUsage(),
                                    XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::Rollback));
                    }

                    std::shared_ptr<XFS4IoT::CashAcceptor::PosCapsClass::RetractAreasClass> retractAreas = nullptr;
                    if (value.GetRetractArea() !=
                        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::NotSupported)
                    {
                        retractAreas =
                            std::make_shared<XFS4IoT::CashAcceptor::PosCapsClass::RetractAreasClass>(
                                XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::HasFlag(
                                    value.GetRetractArea(),
                                    XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::Retract),
                                XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::HasFlag(
                                    value.GetRetractArea(),
                                    XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::Reject),
                                XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::HasFlag(
                                    value.GetRetractArea(),
                                    XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::Transport),
                                XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::HasFlag(
                                    value.GetRetractArea(),
                                    XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::Stacker),
                                XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::HasFlag(
                                    value.GetRetractArea(),
                                    XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::BillCassettes),
                                XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::HasFlag(
                                    value.GetRetractArea(),
                                    XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::CashIn),
                                XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::HasFlag(
                                    value.GetRetractArea(),
                                    XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::ItemCassette)
                            );
                    }

                    posList.emplace_back(
                        convertedPosition.value(),
                        usage,
                        value.GetShutterControl(),
                        value.GetItemsTakenSensor(),
                        value.GetItemsInsertedSensor(),
                        retractAreas,
                        value.GetPresentControl(),
                        value.GetPresentControl());
                }

                positions = std::move(posList);
            }

            XFS4IoT::CashAcceptor::CapabilitiesClass::TypeEnum type;
            switch (*cashAcceptorCaps->GetType())
            {
            case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::TypeEnum::SelfServiceBill:
                type = XFS4IoT::CashAcceptor::CapabilitiesClass::TypeEnum::SelfServiceBill;
                break;
            case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::TypeEnum::SelfServiceCoin:
                type = XFS4IoT::CashAcceptor::CapabilitiesClass::TypeEnum::SelfServiceCoin;
                break;
            case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::TypeEnum::TellerBill:
                type = XFS4IoT::CashAcceptor::CapabilitiesClass::TypeEnum::TellerBill;
                break;
            case XFS4IoTFramework::Common::CashManagementCapabilitiesClass::TypeEnum::TellerCoin:
                type = XFS4IoT::CashAcceptor::CapabilitiesClass::TypeEnum::TellerCoin;
                break;
            default:
                throw XFS4IoT::InternalErrorException(
                    std::format("Указан неожиданный тип приёмника наличных. {}", static_cast<int>(*cashAcceptorCaps->GetType())));
            }

            std::shared_ptr<XFS4IoT::CashAcceptor::CapabilitiesClass::RetractAreasClass> retractAreas = nullptr;
            if (cashAcceptorCaps->GetRetractAreas() !=
                XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractAreaEnum::NotSupported)
            {
                //using RetractArea = XFS4IoTFramework::Common::CashManagementCapabilitiesClass::ItemInfoTypesEnum;
                retractAreas =
					std::make_shared<XFS4IoT::CashAcceptor::CapabilitiesClass::RetractAreasClass>(
						XFS4IoT::HasFlag(
							*cashAcceptorCaps->GetRetractAreas(),
							XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractAreaEnum::Retract),
                        XFS4IoT::HasFlag(
                            *cashAcceptorCaps->GetRetractAreas(),
                            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractAreaEnum::Transport),
                        XFS4IoT::HasFlag(
                            *cashAcceptorCaps->GetRetractAreas(),
                            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractAreaEnum::Stacker),
                        XFS4IoT::HasFlag(
                            *cashAcceptorCaps->GetRetractAreas(),
                            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractAreaEnum::Reject),
                        XFS4IoT::HasFlag(
                            *cashAcceptorCaps->GetRetractAreas(),
                            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractAreaEnum::ItemCassette),
                        XFS4IoT::HasFlag(
                            *cashAcceptorCaps->GetRetractAreas(),
                            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractAreaEnum::CashIn));
            }

            std::shared_ptr<XFS4IoT::CashAcceptor::CapabilitiesClass::RetractTransportActionsClass> retractTransportActions = nullptr;
            if (cashAcceptorCaps->GetRetractTransportActions() !=
                XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractTransportActionEnum::NotSupported)
            {
                retractTransportActions =
                    std::make_shared<XFS4IoT::CashAcceptor::CapabilitiesClass::RetractTransportActionsClass>(
                        XFS4IoT::HasFlag(
                            *cashAcceptorCaps->GetRetractTransportActions(),
                            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractTransportActionEnum::Present),
                        XFS4IoT::HasFlag(
                            *cashAcceptorCaps->GetRetractTransportActions(),
                            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractTransportActionEnum::Retract),
                        XFS4IoT::HasFlag(
                            *cashAcceptorCaps->GetRetractTransportActions(),
                            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractTransportActionEnum::Reject),
                        XFS4IoT::HasFlag(
                            *cashAcceptorCaps->GetRetractTransportActions(),
                            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractTransportActionEnum::BillCassette),
                        XFS4IoT::HasFlag(
                            *cashAcceptorCaps->GetRetractTransportActions(),
                            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractTransportActionEnum::CashIn));
            }

            std::shared_ptr<XFS4IoT::CashAcceptor::CapabilitiesClass::RetractStackerActionsClass> retractStackerActions = nullptr;
            if (cashAcceptorCaps->GetRetractStackerActions() !=
                XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractStackerActionEnum::NotSupported)
            {
                retractStackerActions =
                    std::make_shared<XFS4IoT::CashAcceptor::CapabilitiesClass::RetractStackerActionsClass>(
                        XFS4IoT::HasFlag(
                            *cashAcceptorCaps->GetRetractStackerActions(),
                            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractStackerActionEnum::Present),
                        XFS4IoT::HasFlag(
                            *cashAcceptorCaps->GetRetractStackerActions(),
                            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractStackerActionEnum::Retract),
                        XFS4IoT::HasFlag(
                            *cashAcceptorCaps->GetRetractStackerActions(),
                            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractStackerActionEnum::Reject),
                        XFS4IoT::HasFlag(
                            *cashAcceptorCaps->GetRetractStackerActions(),
                            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractStackerActionEnum::BillCassette),
                        XFS4IoT::HasFlag(
                            *cashAcceptorCaps->GetRetractStackerActions(),
                            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractStackerActionEnum::CashIn));
            }

            std::shared_ptr<XFS4IoT::CashAcceptor::CapabilitiesClass::CashInLimitClass> cashInLimit = nullptr;
            if (cashAcceptorCaps->GetCashInLimit() !=
                XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::CashInLimitEnum::NotSupported)
            {
                cashInLimit =
                    std::make_shared<XFS4IoT::CashAcceptor::CapabilitiesClass::CashInLimitClass>(
                        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::HasFlag(
                            *cashAcceptorCaps->GetCashInLimit(),
                            XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::CashInLimitEnum::ByTotalItems),
                        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::HasFlag(
                            *cashAcceptorCaps->GetCashInLimit(),
                            XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::CashInLimitEnum::ByAmount));
            }

            std::shared_ptr<XFS4IoT::CashAcceptor::CapabilitiesClass::CountActionsClass> countActions = nullptr;
            if (cashAcceptorCaps->GetCountActions() !=
                XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::CountActionEnum::NotSupported)
            {
                countActions =
                    std::make_shared<XFS4IoT::CashAcceptor::CapabilitiesClass::CountActionsClass>(
                        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::HasFlag(
                            *cashAcceptorCaps->GetCountActions(),
                            XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::CountActionEnum::Individual),
                        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::HasFlag(
                            *cashAcceptorCaps->GetCountActions(),
                            XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::CountActionEnum::All));
            }

            std::shared_ptr<XFS4IoT::CashAcceptor::CapabilitiesClass::RetainActionClass> retainAction = nullptr;
            if (cashAcceptorCaps->GetRetainCounterfeitAction() !=
                XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum::NotSupported)
            {
                retainAction =
                    std::make_shared<XFS4IoT::CashAcceptor::CapabilitiesClass::RetainActionClass>(
                        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::HasFlag(
                            *cashAcceptorCaps->GetRetainCounterfeitAction(),
                            XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum::Level2),
                        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::HasFlag(
                            *cashAcceptorCaps->GetRetainCounterfeitAction(),
                            XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum::Level3),
                        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::HasFlag(
                            *cashAcceptorCaps->GetRetainCounterfeitAction(),
                            XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum::Inked));
            }

            cashAcceptor =
                std::make_shared<XFS4IoT::CashAcceptor::CapabilitiesClass>(
                    type,
                    cashAcceptorCaps->GetMaxCashInItems(),
                    cashAcceptorCaps->GetShutter(),
                    cashAcceptorCaps->GetShutterControl(),
                    cashAcceptorCaps->GetIntermediateStacker(),
                    cashAcceptorCaps->GetItemsTakenSensor(), // оставил как у твоем C# источнике
					true, // itemsInsertedSensor  
                    positions,
                    retractAreas,
                    retractTransportActions,
                    retractStackerActions,
                    cashInLimit,
                    countActions,
                    retainAction);
        }

        auto payload =
            std::make_shared<XFS4IoT::Common::Completions::CapabilitiesCompletionPayloadData>(
                interfaces,
                commonCapabilities,
                cashAcceptor,
                cashManagement);

        co_return CapabilitiesResult{
            payload,
            XFS4IoT::MessageHeader::CompletionCodeEnum::Success,
            ""
        };
    }

    std::shared_ptr<InterfaceClass> CapabilitiesHandler::GetDeviceInterface(
        InterfaceClass::NameEnum interfaceName) const
    {
        std::optional<std::unordered_map<std::string, std::shared_ptr<InterfaceClass::CommandsClass>>> commands;
        std::optional<std::unordered_map<std::string, std::shared_ptr<InterfaceClass::EventsClass>>> events;

        const auto messagesSupported = provider_->GetMessagesSupported();

        //for (const auto& [name, info] : messagesSupported)
        //{
        //    const std::string prefix = std::format("{}.", XFS4IoT::Common::toString(interfaceName));

        //    // Там InterfaceName = "Common", "CashManagement", ...
        //    // Если твой toString(NameEnum) возвращает lowerCamelCase, то здесь это не подойдет.
        //    // Тогда используй отдельный helper для exact XFS interface name.
        //}

        // Правильнее так:
        auto interfaceNameToPrefix = [](InterfaceClass::NameEnum v) -> std::string
            {
                switch (v)
                {
                case InterfaceClass::NameEnum::Common: return "Common.";
                case InterfaceClass::NameEnum::CardReader: return "CardReader.";
                case InterfaceClass::NameEnum::CashAcceptor: return "CashAcceptor.";
                case InterfaceClass::NameEnum::CashDispenser: return "CashDispenser.";
                case InterfaceClass::NameEnum::CashManagement: return "CashManagement.";
                case InterfaceClass::NameEnum::PinPad: return "PinPad.";
                case InterfaceClass::NameEnum::Crypto: return "Crypto.";
                case InterfaceClass::NameEnum::KeyManagement: return "KeyManagement.";
                case InterfaceClass::NameEnum::Keyboard: return "Keyboard.";
                case InterfaceClass::NameEnum::TextTerminal: return "TextTerminal.";
                case InterfaceClass::NameEnum::Printer: return "Printer.";
                case InterfaceClass::NameEnum::BarcodeReader: return "BarcodeReader.";
                case InterfaceClass::NameEnum::Camera: return "Camera.";
                case InterfaceClass::NameEnum::Lights: return "Lights.";
                case InterfaceClass::NameEnum::Auxiliaries: return "Auxiliaries.";
                case InterfaceClass::NameEnum::VendorMode: return "VendorMode.";
                case InterfaceClass::NameEnum::VendorApplication: return "VendorApplication.";
                case InterfaceClass::NameEnum::Storage: return "Storage.";
                case InterfaceClass::NameEnum::Biometric: return "Biometric.";
                case InterfaceClass::NameEnum::Check: return "Check.";
                case InterfaceClass::NameEnum::MixedMedia: return "MixedMedia.";
                case InterfaceClass::NameEnum::Deposit: return "Deposit.";
                case InterfaceClass::NameEnum::German: return "German.";
                case InterfaceClass::NameEnum::BanknoteNeutralization: return "BanknoteNeutralization.";
                case InterfaceClass::NameEnum::PowerManagement: return "PowerManagement.";
                }

                throw std::invalid_argument("Unknown interface name");
            };

        const std::string prefix = interfaceNameToPrefix(interfaceName);

        for (const auto& [name, info] : messagesSupported)
        {
            if (!name.starts_with(prefix)) {
                continue;
            }

            if (info.Type == XFS4IoT::MessageTypeInfo::MessageTypeEnum::Command)
            {
                if (!commands.has_value()) {
                    commands = std::unordered_map<std::string, std::shared_ptr<InterfaceClass::CommandsClass>>{};
                }

                commands->emplace(
                    name,
                    std::make_shared<InterfaceClass::CommandsClass>(info.Versions));
            }
            else if (info.Type == XFS4IoT::MessageTypeInfo::MessageTypeEnum::Event)
            {
                if (!events.has_value()) {
                    events = std::unordered_map<std::string, std::shared_ptr<InterfaceClass::EventsClass>>{};
                }

                events->emplace(
                    name,
                    std::make_shared<InterfaceClass::EventsClass>(info.Versions));
            }
        }

        if (!commands.has_value() && !events.has_value()) {
            return nullptr;
        }

        return std::make_shared<InterfaceClass>(
            interfaceName,
            commands,
            events,
            XFS4IoT::XFSConstants::MaximumRequests);
    }

    XFS4IoT::MessageHeader::CompletionCodeEnum
        CapabilitiesHandler::MapExceptionToCompletionCode(const std::exception& ex) const
    {
        if (dynamic_cast<const XFS4IoT::InvalidDataException*>(&ex)) {
            return XFS4IoT::MessageHeader::CompletionCodeEnum::InvalidData;
        }
        if (dynamic_cast<const XFS4IoT::InternalErrorException*>(&ex)) {
            return XFS4IoT::MessageHeader::CompletionCodeEnum::InternalError;
        }
        if (dynamic_cast<const XFS4IoT::UnsupportedDataException*>(&ex)) {
            return XFS4IoT::MessageHeader::CompletionCodeEnum::UnsupportedData;
        }
        if (dynamic_cast<const XFS4IoT::SequenceErrorException*>(&ex)) {
            return XFS4IoT::MessageHeader::CompletionCodeEnum::SequenceError;
        }
        if (dynamic_cast<const XFS4IoT::AuthorisationRequiredException*>(&ex)) {
            return XFS4IoT::MessageHeader::CompletionCodeEnum::AuthorisationRequired;
        }
        if (dynamic_cast<const XFS4IoT::HardwareErrorException*>(&ex)) {
            return XFS4IoT::MessageHeader::CompletionCodeEnum::HardwareError;
        }
        if (dynamic_cast<const XFS4IoT::UserErrorException*>(&ex)) {
            return XFS4IoT::MessageHeader::CompletionCodeEnum::UserError;
        }
        if (dynamic_cast<const XFS4IoT::FraudAttemptException*>(&ex)) {
            return XFS4IoT::MessageHeader::CompletionCodeEnum::FraudAttempt;
        }
        if (dynamic_cast<const XFS4IoT::DeviceNotReadyException*>(&ex)) {
            return XFS4IoT::MessageHeader::CompletionCodeEnum::DeviceNotReady;
        }
        if (dynamic_cast<const XFS4IoT::InvalidCommandException*>(&ex)) {
            return XFS4IoT::MessageHeader::CompletionCodeEnum::InvalidCommand;
        }
        if (dynamic_cast<const XFS4IoT::NotEnoughSpaceException*>(&ex)) {
            return XFS4IoT::MessageHeader::CompletionCodeEnum::NotEnoughSpace;
        }
        if (dynamic_cast<const XFS4IoT::NotImplementedException*>(&ex) ||
            dynamic_cast<const XFS4IoT::NotSupportedException*>(&ex)) {
            return XFS4IoT::MessageHeader::CompletionCodeEnum::UnsupportedCommand;
        }
        if (auto timeoutEx = dynamic_cast<const TimeoutCanceledException*>(&ex))
        {
            return timeoutEx->IsCancelRequested()
                ? XFS4IoT::MessageHeader::CompletionCodeEnum::Canceled
                : XFS4IoT::MessageHeader::CompletionCodeEnum::TimeOut;
        }

        return XFS4IoT::MessageHeader::CompletionCodeEnum::InternalError;
    }
}