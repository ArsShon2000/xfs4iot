// CashAcceptorServiceProvider.cpp

#include "CashAcceptorServiceProvider.hpp"
#include <stdexcept>
#include "../../server/CommandDispatcher.hpp"
#include "../../core/MessageDecoder.hpp"
#include "../../core/common/Commands/CapabilitiesCommand.hpp"
#include "../CommonServiceProvider/Handlers/CapabilitiesHandler.hpp"
#include "../CashManagementServiceProvider/Handlers/ResetHandler.hpp"
#include "../CashManagementServiceProvider/Handlers/GetBankNoteTypesHandler.hpp"
#include "../CashAcceptorServiceProvider/Handlers/ConfigureNoteTypesHandler.hpp"
#include "Handlers/CashInStartHandler.hpp"

namespace XFS4IoTServer
{
    CashAcceptorServiceProvider::CashAcceptorServiceProvider(
        const EndpointDetails& endpointDetails,
        const std::string& serviceName,
        std::shared_ptr<IDevice> device,
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<IPersistentData> persistentData)
        : ServiceProvider(
            endpointDetails,
            serviceName,
            {
                XFS4IoT::XFSConstants::ServiceClass::Common,
                XFS4IoT::XFSConstants::ServiceClass::CashAcceptor,
                XFS4IoT::XFSConstants::ServiceClass::CashManagement,
                XFS4IoT::XFSConstants::ServiceClass::Storage
            },
            device,
            logger)
        , commonService_(nullptr)
        , storageService_(nullptr)
        , cashManagementService_(nullptr)
        , cashAcceptor_(nullptr)
        //, cashDispenserService_(nullptr)
        //, ibnsService_(nullptr)
        //, checkService_(nullptr)
    {
        std::vector<XFS4IoT::XFSConstants::ServiceClass> services = XFS4IoTServer::CommandDispatcher::GetServiceClasses();

        //// Check optional services
        //if (auto cashDispenserDevice = std::dynamic_pointer_cast<ICashDispenserDevice>(device))
        //{
        //    cashDispenserService_ = std::make_shared<CashDispenserServiceClass>(
        //        shared_from_this(), logger, persistentData);
        //    services.push_back(XFSConstants::ServiceClass::CashDispenser);
        //}

        //if (auto ibnsDevice = std::dynamic_pointer_cast<IBanknoteNeutralizationDevice>(device))
        //{
        //    ibnsService_ = std::make_shared<BanknoteNeutralizationServiceClass>(
        //        shared_from_this(), logger, persistentData);
        //    services.push_back(XFSConstants::ServiceClass::BanknoteNeutralization);
        //}

        //if (auto checkDevice = std::dynamic_pointer_cast<ICheckDevice>(device))
        //{
        //    checkService_ = std::make_shared<CheckServiceClass>(
        //        shared_from_this(), logger, persistentData);
        //    services.push_back(XFSConstants::ServiceClass::Check);
        //}

        XFS4IoTServer::CommandDispatcher::SetServiceClasses(services);

        // Здесь не создаём дочерние объекты, т.к. shared_from_this() внутри конструктора — некорректно.
        (void)persistentData; // подавить предупреждение о неиспользуемом параметре
    }

    void CashAcceptorServiceProvider::Initialize(
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<IPersistentData> persistentData)
    {
        logger->trace(std::format(
            "CashAcceptorServiceProvider::Initialize this = {}",
            static_cast<const void*>(this)));

        auto self = GetServiceProviderPtr();

        logger->trace(std::format(
            "CashAcceptorServiceProvider::Initialize self.get() = {}",
            static_cast<const void*>(self.get())));

        commonService_ = std::make_shared<CommonServiceClass>(self, logger, GetName());
        storageService_ = std::make_shared<StorageServiceClass>(
            self, logger, persistentData, XFS4IoTFramework::Storage::StorageTypeEnum::Cash);
        cashManagementService_ = std::make_shared<CashManagementServiceClass>(
            self, logger, persistentData);
        cashAcceptor_ = std::make_shared<CashAcceptorServiceClass>(self, logger);

        // 🔥 ВОТ ЭТО ВАЖНО
        auto decoder = std::make_shared<XFS4IoT::MessageDecoder>(logger);

        //decoder->RegisterMessageType<XFS4IoT::Common::Commands::CapabilitiesCommand>(
        //    "Common.Capabilities");

        decoder->RegisterMessageType<XFS4IoT::Common::Commands::CapabilitiesCommand>(
            XFS4IoT::Common::Commands::CapabilitiesCommand::CommandName);
        RegisterHandler<
            XFS4IoT::Common::Commands::CapabilitiesCommand,
            XFS4IoTServer::Common::CapabilitiesHandler
        >(
            XFS4IoT::XFSConstants::ServiceClass::Common,
            true
        );


        decoder->RegisterMessageType<XFS4IoT::CashManagement::Commands::ResetCommand>(
            XFS4IoT::CashManagement::Commands::ResetCommand::CommandName);
        RegisterHandler<
            XFS4IoT::CashManagement::Commands::ResetCommand,
            XFS4IoTFramework::CashManagement::ResetHandler
        >(
            XFS4IoT::XFSConstants::ServiceClass::CashManagement,
            false
        );


        decoder->RegisterMessageType<XFS4IoT::CashAcceptor::Commands::ConfigureNoteTypesCommand>(
            XFS4IoT::CashAcceptor::Commands::ConfigureNoteTypesCommand::CommandName);
        RegisterHandler<
            XFS4IoT::CashAcceptor::Commands::ConfigureNoteTypesCommand,
            XFS4IoTFramework::CashAcceptor::ConfigureNoteTypesHandler
        >(
            XFS4IoT::XFSConstants::ServiceClass::CashAcceptor,
            false
        );


        decoder->RegisterMessageType<XFS4IoT::CashManagement::Commands::GetBankNoteTypesCommand>(
            XFS4IoT::CashManagement::Commands::GetBankNoteTypesCommand::CommandName);
        RegisterHandler<
            XFS4IoT::CashManagement::Commands::GetBankNoteTypesCommand,
            XFS4IoTFramework::CashManagement::GetBankNoteTypesHandler
        >(
            XFS4IoT::XFSConstants::ServiceClass::CashManagement,
            false
        );

        decoder->RegisterMessageType<XFS4IoT::CashAcceptor::Commands::CashInStartCommand>(
            XFS4IoT::CashAcceptor::Commands::CashInStartCommand::CommandName);
        RegisterHandler<
            XFS4IoT::CashAcceptor::Commands::CashInStartCommand,
            XFS4IoTFramework::CashAcceptor::CashInStartHandler
        >(
            XFS4IoT::XFSConstants::ServiceClass::CashAcceptor,
            true
        );

        logger->trace(std::format(
            "CashAcceptorServiceProvider::Initialize зарегистрирован обработчик для this = {}",
            static_cast<const void*>(this)));

        //RegisterHandler<
        //    XFS4IoT::Common::Commands::StatusCommand,
        //    XFS4IoTServer::Common::StatusHandler
        //>(
        //    XFS4IoT::XFSConstants::ServiceClass::Common,
        //    true
        //);


        // TODO: нужно добавить сюда остальные команды сервиса

        decoder_ = decoder;

		SetMessagesSupported({
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
        }
		});
        logger->trace(std::format(
            "CashAcceptorServiceProvider::Initialize() количество зарегистрированных обработчиков count={}",
            m_messagesSupported.size()));
	}

    // ============================================================================
    // CashManagement unsolicited events
    // ============================================================================

    boost::asio::awaitable<void> CashAcceptorServiceProvider::ItemsTakenEvent(
        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position,
        std::optional<std::string> additionalBunches)
    {
        co_return co_await cashManagementService_->ItemsTakenEvent(position, additionalBunches);
    }

    boost::asio::awaitable<void> CashAcceptorServiceProvider::ItemsInsertedEvent(
        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position)
    {
        co_return co_await cashManagementService_->ItemsInsertedEvent(position);
    }

    boost::asio::awaitable<void> CashAcceptorServiceProvider::ItemsPresentedEvent(
        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position,
        std::optional<std::string> additionalBunches)
    {
        co_return co_await cashManagementService_->ItemsPresentedEvent(position, additionalBunches);
    }

    // ============================================================================
    // Storage Service
    // ============================================================================

    boost::asio::awaitable<void> CashAcceptorServiceProvider::UpdateCardStorageCount(
        const std::string& storageId,
        int countDelta)
    {
        throw std::runtime_error("Класс сервиса CashAcceptor не поддерживает хранение карт.");
        co_return;
    }

    boost::asio::awaitable<void> CashAcceptorServiceProvider::UpdateCashAccounting(
        const std::optional < std::unordered_map<std::string, std::shared_ptr<XFS4IoTFramework::Storage::CashUnitCountClass>>> countDelta)
    {
        co_return co_await storageService_->UpdateCashAccounting(countDelta);
    }

    //boost::asio::awaitable<void> CashAcceptorServiceProvider::UpdateCheckStorageCount(
    //    const std::map<std::string, StorageCheckCountClass>& countDelta)
    //{
    //    throw std::runtime_error("CashAcceptor service class doesn't support check storage.");
    //    co_return;
    //}

    //boost::asio::awaitable<void> CashAcceptorServiceProvider::UpdatePrinterStorageCount(
    //    const std::string& storageId,
    //    int countDelta)
    //{
    //    throw std::runtime_error("CashAcceptor service class doesn't support printer storage.");
    //    co_return;
    //}

    boost::asio::awaitable<void> CashAcceptorServiceProvider::UpdateDepositStorageCount(
        const std::string& storageId,
        int countDelta)
    {
        throw std::runtime_error("Класс сервиса CashAcceptor не поддерживает хранилище депозитов.");
        co_return;
    }

    void CashAcceptorServiceProvider::StorePersistent()
    {
        storageService_->StorePersistent();
    }

    XFS4IoTFramework::Storage::StorageTypeEnum CashAcceptorServiceProvider::GetStorageType() const
    {
        return storageService_->GetStorageType();
    }

    //const std::map<std::string, CardUnitStorage>& CashAcceptorServiceProvider::GetCardUnits() const
    //{
    //    return storageService_->GetCardUnits();
    //}

    const std::unordered_map<std::string, std::shared_ptr<XFS4IoTFramework::Storage::CashUnitStorage>>& CashAcceptorServiceProvider::GetCashUnits() const
    {
        return storageService_->GetCashUnits();
    }

    //const std::map<std::string, CheckUnitStorage>& CashAcceptorServiceProvider::GetCheckUnits() const
    //{
    //    return storageService_->GetCheckUnits();
    //}

    //const std::map<std::string, DepositUnitStorage>& CashAcceptorServiceProvider::GetDepositUnits() const
    //{
    //    return storageService_->GetDepositUnits();
    //}

    std::unordered_map<std::string, std::shared_ptr<XFS4IoT::Storage::StorageUnitClass>> CashAcceptorServiceProvider::GetStorages(
        const std::vector<std::string>& unitIds)
    {
        return storageService_->GetStorages(unitIds);
    }

    // ============================================================================
    // Storage Unsolicited events
    // ============================================================================

    boost::asio::awaitable<void> CashAcceptorServiceProvider::MediaDetectedEvent(
        std::optional<std::string> storageId,
        XFS4IoTFramework::CashManagement::ItemTargetEnum target,
        std::optional<int> index)
    {
        if (!cashManagementService_)
        {
            throw std::runtime_error("CashManagementServiceClass is not initialized");
        }

        co_await cashManagementService_->MediaDetectedEvent(
            std::move(storageId),
            target,
            index);
    }

    boost::asio::awaitable<void> CashAcceptorServiceProvider::StorageChangedEvent(
        std::shared_ptr<void> sender,
        std::shared_ptr<XFS4IoTServer::PropertyChangedEventArgs> propertyInfo)
    {
        co_return co_await storageService_->StorageChangedEvent(sender, propertyInfo);
    }

    // ============================================================================
    // Common unsolicited events
    // ============================================================================

    boost::asio::awaitable<void> CashAcceptorServiceProvider::StatusChangedEvent(
        std::shared_ptr<void> sender,
        std::shared_ptr<PropertyChangedEventArgs> propertyInfo)
    {
        co_return co_await commonService_->StatusChangedEvent(sender, propertyInfo);
    }

    boost::asio::awaitable<void> CashAcceptorServiceProvider::NonceClearedEvent(
        const std::string& reasonDescription)
    {
        throw std::runtime_error("Событие NonceClearedEvent не поддерживается в сервисе CashAcceptor.");
        co_return;
    }

    boost::asio::awaitable<void> CashAcceptorServiceProvider::ErrorEvent(
        XFS4IoTFramework::Common::CommonStatusClass::ErrorEventIdEnum eventId,
        XFS4IoTFramework::Common::CommonStatusClass::ErrorActionEnum action,
        const std::string& vendorDescription)
    {
        co_return co_await commonService_->ErrorEvent(eventId, action, vendorDescription);
    }

    // ============================================================================
    // Common Service
    // ============================================================================

    std::shared_ptr<XFS4IoTFramework::Common::CommonCapabilitiesClass> CashAcceptorServiceProvider::GetCommonCapabilities() const
    {
        return commonService_->GetCommonCapabilities();
    }

    void CashAcceptorServiceProvider::SetCommonCapabilities(std::shared_ptr<XFS4IoTFramework::Common::CommonCapabilitiesClass> capabilities)
    {
        commonService_->SetCommonCapabilities(capabilities);
    }

    std::shared_ptr<XFS4IoTFramework::Common::CommonStatusClass> CashAcceptorServiceProvider::GetCommonStatus() const
    {
        return commonService_->GetCommonStatus();
    }

    void CashAcceptorServiceProvider::SetCommonStatus(std::shared_ptr<XFS4IoTFramework::Common::CommonStatusClass> status)
    {
        commonService_->SetCommonStatus(status);
    }

    std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass> CashAcceptorServiceProvider::GetCashAcceptorCapabilities() const
    {
        return commonService_->GetCashAcceptorCapabilities();
    }

    void CashAcceptorServiceProvider::SetCashAcceptorCapabilities(std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass> capabilities)
    {
        commonService_->SetCashAcceptorCapabilities(capabilities);
    }

    std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorStatusClass> CashAcceptorServiceProvider::GetCashAcceptorStatus() const
    {
        return commonService_->GetCashAcceptorStatus();
    }

    void CashAcceptorServiceProvider::SetCashAcceptorStatus(std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorStatusClass> status)
    {
        commonService_->SetCashAcceptorStatus(status);
    }

    std::shared_ptr<XFS4IoTFramework::Common::CashManagementCapabilitiesClass> CashAcceptorServiceProvider::GetCashManagementCapabilities() const
    {
        return commonService_->GetCashManagementCapabilities();
    }

    void CashAcceptorServiceProvider::SetCashManagementCapabilities(std::shared_ptr<XFS4IoTFramework::Common::CashManagementCapabilitiesClass> capabilities)
    {
        commonService_->SetCashManagementCapabilities(capabilities);
    }

    std::shared_ptr<XFS4IoTFramework::Common::CashManagementStatusClass> CashAcceptorServiceProvider::GetCashManagementStatus() const
    {
        return commonService_->GetCashManagementStatus();
    }

    void CashAcceptorServiceProvider::SetCashManagementStatus(std::shared_ptr<XFS4IoTFramework::Common::CashManagementStatusClass> status)
    {
        commonService_->SetCashManagementStatus(status);
    }

    // ============================================================================
    // CashManagement Service
    // ============================================================================

    const std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> CashAcceptorServiceProvider::GetCashInStatusManaged() const
    {
        return cashManagementService_->GetCashInStatusManaged();
    }

    void CashAcceptorServiceProvider::StoreCashInStatus()
    {
        cashManagementService_->StoreCashInStatus();
    }

    const std::unordered_map<XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum,
        std::shared_ptr<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>>&
        CashAcceptorServiceProvider::GetLastCashManagementPresentStatus() const
    {
        return cashManagementService_->GetLastCashManagementPresentStatus();
    }

    void CashAcceptorServiceProvider::StoreCashManagementPresentStatus()
    {
        cashManagementService_->StoreCashManagementPresentStatus();
    }

    const std::shared_ptr<XFS4IoTFramework::CashManagement::ItemClassificationListClass>& CashAcceptorServiceProvider::GetItemClassificationList() const
    {
        return cashManagementService_->GetItemClassificationList();
    }

    void CashAcceptorServiceProvider::StoreItemClassificationList()
    {
        cashManagementService_->StoreItemClassificationList();
    }

    // ============================================================================
    // CashAcceptor Service
    // ============================================================================

    std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> CashAcceptorServiceProvider::GetCashInStatus() const
    {
        return cashAcceptor_->GetCashInStatus();
    }

    XFS4IoTFramework::CashAcceptor::DeviceLockStatusClass CashAcceptorServiceProvider::GetDeviceLockStatus() const
    {
        return cashAcceptor_->GetDeviceLockStatus();
    }

    const std::map<std::string, std::vector<std::string>>&
        CashAcceptorServiceProvider::GetDepleteCashUnitSources() const
    {
        return cashAcceptor_->GetDepleteCashUnitSources();
    }

    const std::vector<std::string>& CashAcceptorServiceProvider::GetReplenishTargets() const
    {
        return cashAcceptor_->GetReplenishTargets();
    }

} // namespace XFS4IoTServer