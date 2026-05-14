#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <thread>
#include <semaphore>
#include <atomic>
#include "../../framework/ServiceClasses/CommonServiceProvider/CommonCapabilitiesClass.hpp"
#include "../../framework/core/Logger/ILogger.hpp"
#include "../../framework/server/DeviceResult.hpp"
#include "../../framework/ServiceClasses/CashManagementServiceProvider/ICashManagementDevice.hpp"
#include "../../framework/ServiceClasses/CashAcceptorServiceProvider/ICashAcceptorDevice.hpp"
#include "../../framework/ServiceClasses/CommonServiceProvider/ICommonDevice.hpp"
//#include "CashAcceptorServiceProvider.hpp"
#include "../../framework/ServiceClasses/CommonServiceProvider/CommonStatusClass.hpp"
#include "../../framework/ServiceClasses/CommonServiceProvider/CashAcceptorStatusClass.hpp"
#include "../../framework/ServiceClasses/CommonServiceProvider/CashManagementStatusClass.hpp"
#include "../../framework/ServiceClasses/CommonServiceProvider/CashAcceptorCapabilitiesClass.hpp"
#include "../../framework/ServiceClasses/CommonServiceProvider/CashManagementCapabilitiesClass.hpp"
#include "../../framework/ServiceClasses/StorageServiceProvider/IStorageDevice.hpp"
#include "../../framework/ServiceClasses/CashManagementServiceProvider/CashInStatusClass.hpp"
#include "../../framework/ServiceClasses/CashAcceptorServiceProvider/CashAcceptorServiceProvider.hpp"
#include "../../framework/ServiceClasses/StorageServiceProvider/CashUnit.hpp"
#include "../../framework/ServiceClasses/CashAcceptorServiceProvider/Events/CashInCommandEvents.hpp"
#include "../../framework/ServiceClasses/CommonServiceProvider/DeviceParameters.hpp"
#include "../../framework/ServiceClasses/CashAcceptorServiceProvider/Events/CashInRollbackCommandEvents.hpp"
#include "../../framework/ServiceClasses/CashAcceptorServiceProvider/Events/CashUnitCountCommandEvents.hpp"
#include "../../framework/ServiceClasses/CashAcceptorServiceProvider/Events/PreparePresentCommandEvents.hpp"
#include "../HW/Dors_210BA.h"
#include "../../framework/core/common/StateMachine/StateMachine.hpp"
//#include "CashUnitStorageConfiguration.hpp"
//#include "CashStatusClass.hpp"

    class PowerUpManager;

namespace XFS4IoTSP::CashAcceptor::Sample
{

    using StateMachine = XfsCommon::StateMachine< FS365::HW::Dors::DorsHW::POLL_RES >;

    inline constexpr std::chrono::milliseconds POLLING_INTERVAL{ 100 }; // интервал между опросами устройства, по спецификации - 100мс

    inline constexpr std::chrono::milliseconds OPERATION_INTERVAL{ 5000 }; // время, необходимое на завершение операций Stacking, Returning, Accepting, Initialize



    //using namespace XFS4IoT;
    //using namespace XFS4IoTFramework::Storage;
    //using namespace XFS4IoTFramework::Common::;
    //using namespace XFS4IoTFramework::CashAcceptor;
    //using namespace XFS4IoTFramework::CashManagement::;
    /// <summary>
    /// Sample CashAcceptor device class to implement
    /// </summary>
    class CashAcceptorSample : public XFS4IoTFramework::CashManagement::ICashManagementDevice,
        public XFS4IoTFramework::CashAcceptor::ICashAcceptorDevice,
        public XFS4IoTFramework::Common::ICommonDevice,
        public XFS4IoTFramework::Storage::IStorageDevice,
        public std::enable_shared_from_this<CashAcceptorSample>
    {


    public:
        /// <summary>
        /// Constructor
        /// </summary>
        explicit CashAcceptorSample(
            std::unique_ptr<FS365::HW::Dors::DorsHW> device
            , FS365::HW::Dors::CIdentification& idn
            , std::shared_ptr<ILogger> logger
            , std::map<std::string, XFS4IoTFramework::Common::CashManagementCapabilitiesClass::BanknoteItem> &allBanknoteIDs
        );

        /// <summary>
        /// Destructor
        /// </summary>
        ~CashAcceptorSample();

        /// <summary>
        /// RunAsync
        /// Handle unsolicited events
        /// Here is an example of handling ItemsTakenEvent after cash is returned and taken by customer.
        /// </summary>
        boost::asio::awaitable<void> RunAsync(std::stop_token cancel);

        void Initialize();


        // ============================================================================
        // Купюры
        // ============================================================================
        /// Флаг наличия кэша таблицы номиналов
        bool getHasBillTableCache() { return m_bHasBillTableCache; }

        /// Номинал обрабатываемой банкноты
        uint16_t m_usCurrentNoteID;

        // ============================================================================
        // CashAcceptor Interface
        // ============================================================================

        /// <summary>
        /// Before initiating a cash-in operation, an application must issue the CashInStart command to begin a cash-in transaction.
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashInStartResult> CashInStart(
            const XFS4IoTFramework::CashAcceptor::CashInStartRequest& request,
            std::stop_token cancellation) override;

        /// <summary>
        /// This command moves items into the cash device from an input position.
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashInResult> CashIn(
            std::shared_ptr<XFS4IoTFramework::CashAcceptor::CashInCommandEvents> events,
            const XFS4IoTFramework::CashAcceptor::CashInRequest& request,
            std::stop_token cancellation) override;

        /// <summary>
        /// This command ends a cash-in transaction.
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashInEndResult> CashInEnd(
            std::shared_ptr<XFS4IoTFramework::CashAcceptor::CashInEndCommandEvents> events,
            std::stop_token cancellation) override;

        /// <summary>
        /// This command is used to roll back a cash-in transaction.
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashInRollbackResult> CashInRollback(
            std::shared_ptr<XFS4IoTFramework::CashAcceptor::CashInRollbackCommandEvents> events,
            std::stop_token cancellation) override;

        /// <summary>
        /// This command is used to change the note types the banknote reader should accept during cash-in.
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::ConfigureNoteTypesResult> ConfigureNoteTypes(
            const XFS4IoTFramework::CashAcceptor::ConfigureNoteTypesRequest& request,
            std::stop_token cancellation) override;


        /// <summary>
        /// This command is used to configure the currency description configuration data.
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::ConfigureNoteReaderResult> ConfigureNoteReader(
            const XFS4IoTFramework::CashAcceptor::ConfigureNoteReaderRequest& request,
            std::stop_token cancellation) override;

        /// <summary>
        /// This command counts the items in the storage unit(s).
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashUnitCountResult> CashUnitCount(
            std::shared_ptr<XFS4IoTFramework::CashAcceptor::CashUnitCountCommandEvents> events,
            const XFS4IoTFramework::CashAcceptor::CashUnitCountRequest& request,
            std::stop_token cancellation) override;

        /// <summary>
        /// This command can be used to lock or unlock a CashAcceptor device or one or more storage units.
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::DeviceLockResult> DeviceLockControl(
            const XFS4IoTFramework::CashAcceptor::DeviceLockRequest& request,
            std::stop_token cancellation) override;

        /// <summary>
        /// This command opens the shutter and presents items to be taken by the customer.
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::PresentMediaResult> PresentMedia(
            const XFS4IoTFramework::CashAcceptor::PresentMediaRequest& request,
            std::stop_token cancellation) override;

        /// <summary>
        /// Move a remaining bunch to the output position explicitly.
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::PreparePresentResult> PreparePresent(
            std::shared_ptr<XFS4IoTFramework::CashAcceptor::PreparePresentCommandEvents> events,
            const XFS4IoTFramework::CashAcceptor::PreparePresentRequest& request,
            std::stop_token cancellation) override;

        /// <summary>
        /// The deplete target and destination information
        /// </summary>
        std::map<std::string, std::vector<std::string>> GetDepleteCashUnitSources() override;

        /// <summary>
        /// Which storage units can be specified as targets for a given source storage unit
        /// </summary>
        std::vector<std::string> GetReplenishTargets() override;

        /// <summary>
        /// CashAcceptor Status
        /// </summary>
        const XFS4IoTFramework::Common::CashAcceptorStatusClass& GetCashAcceptorStatus() const override { return *cashAcceptorStatus_; }
        void SetCashAcceptorStatus(std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorStatusClass> status) override {
            cashAcceptorStatus_ = std::move(status);
        }
        
        /// <summary>
        /// CashAcceptor Capabilities
        /// </summary>
        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass GetCashAcceptorCapabilities() const override { return cashAcceptorCapabilities_; }
        void SetCashAcceptorCapabilities(XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass capabilities) override {
            cashAcceptorCapabilities_ = std::move(capabilities);
        }

        /// <summary>
        /// Status of current cash-in operation
        /// </summary>
        std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> GetCashInStatus() const override { return cashInStatus_; }
        void SetCashInStatus(std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> status) override { cashInStatus_ = status; }

        /// <summary>
        /// The physical lock/unlock status of the CashAcceptor device and storages
        /// </summary>
        XFS4IoTFramework::CashAcceptor::DeviceLockStatusClass GetDeviceLockStatus() const override { return deviceLockStatus_; }
        void SetDeviceLockStatus(XFS4IoTFramework::CashAcceptor::DeviceLockStatusClass status) override {
            deviceLockStatus_ = std::move(status);
        }

        // ============================================================================
        // CashManagement Interface
        // ============================================================================

        /// <summary>
        /// This method will retract items which may have been in customer access.
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::CashManagement::RetractResult> RetractAsync(
            std::shared_ptr<XFS4IoTFramework::CashManagement::RetractCommandEvents> events,
            const XFS4IoTFramework::CashManagement::RetractRequest& retractInfo,
            std::stop_token cancellation) override;

        /// <summary>
        /// Perform shutter operation to open or close.
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::CashManagement::OpenCloseShutterResult> OpenCloseShutterAsync(
            const XFS4IoTFramework::CashManagement::OpenCloseShutterRequest& shutterInfo,
            std::stop_token cancellation) override;

        /// <summary>
        /// Perform a hardware reset.
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::CashManagement::ResetDeviceResult> ResetDeviceAsync(
            std::shared_ptr<XFS4IoTFramework::CashManagement::ResetCommandEvents> events,
            const XFS4IoTFramework::CashManagement::ResetDeviceRequest& resetDeviceInfo,
            std::stop_token cancellation) override;

        /// <summary>
        /// Calibrate cash unit.
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::CashManagement::CalibrateCashUnitResult> CalibrateCashUnitAsync(
            std::shared_ptr<XFS4IoTFramework::CashManagement::CalibrateCashUnitCommandEvents> events,
            const XFS4IoTFramework::CashManagement::CalibrateCashUnitRequest& calibrationInfo,
            std::stop_token cancellation) override;

        /// <summary>
        /// Get teller info (Teller devices only).
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::CashManagement::GetTellerInfoResult> GetTellerInfoAsync(
            const XFS4IoTFramework::CashManagement::GetTellerInfoRequest& request,
            std::stop_token cancellation) override;

        /// <summary>
        /// Set teller info (Teller devices only).
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::CashManagement::SetTellerInfoResult> SetTellerInfoAsync(
            const XFS4IoTFramework::CashManagement::SetTellerInfoRequest& request,
            std::stop_token cancellation) override;

        /// <summary>
        /// Get information about detected items.
        /// </summary>
        XFS4IoTFramework::CashManagement::GetItemInfoResult GetItemInfoInfo(const XFS4IoTFramework::CashManagement::GetItemInfoRequest& request) override;

        /// <summary>
        /// CashManagement Status
        /// </summary>
        std::shared_ptr < XFS4IoTFramework::Common::CashManagementStatusClass> GetCashManagementStatus() const override { return cashManagementStatus_; }
        void SetCashManagementStatus(std::shared_ptr<XFS4IoTFramework::Common::CashManagementStatusClass> status) override {
            cashManagementStatus_ = std::move(status);
        }

        /// <summary>
        /// CashManagement Capabilities
        /// </summary>
        std::shared_ptr < XFS4IoTFramework::Common::CashManagementCapabilitiesClass> GetCashManagementCapabilities() const override { return cashManagementCapabilities_; }
        void SetCashManagementCapabilities(std::shared_ptr<XFS4IoTFramework::Common::CashManagementCapabilitiesClass> capabilities) override {
            cashManagementCapabilities_ = std::move(capabilities);
        }


        // ============================================================================
        // EVENTS
        // ============================================================================

        void SendMediaDetected(
            const std::string& storageId,
            XFS4IoTFramework::CashManagement::ItemTargetEnum target,
            std::optional<int> index = std::nullopt);


        // ============================================================================
        // Storage Interface
        // ============================================================================

        /// <summary>
        /// Return storage information for current configuration and capabilities on the startup.
        /// </summary>
        bool GetCashStorageConfiguration(
            std::map<std::string, XFS4IoTFramework::Storage::CashUnitStorageConfiguration>& newCashUnits) override;

        /// <summary>
        /// Return cash unit counts maintained by the device
        /// </summary>
        bool GetCashUnitCounts(
            std::map<std::string, XFS4IoTFramework::Storage::CashUnitCountClass>& unitCounts) override;

        /// <summary>
        /// Return cash unit initial counts maintained by the device class
        /// </summary>
        bool GetCashUnitInitialCounts(
            std::map<std::string, XFS4IoTFramework::Storage::StorageCashCountClass>& initialCounts) override;

        /// <summary>
        /// Return cash storage status
        /// </summary>
        bool GetCashStorageStatus(
            std::map<std::string, XFS4IoTFramework::Storage::CashUnitStorage::StatusEnum>& storageStatus) override;

        /// <summary>
        /// Return cash unit status maintained by the device class
        /// </summary>
        bool GetCashUnitStatus(
            std::map<std::string, XFS4IoTFramework::Storage::CashStatusClass::ReplenishmentStatusEnum>& unitStatus) override;

        /// <summary>
        /// Return accuracy of counts
        /// </summary>
        void GetCashUnitAccuracy(
            const std::string& storageId,
            XFS4IoTFramework::Storage::CashStatusClass::AccuracyEnum& unitAccuracy) override;

        /// <summary>
        /// Set new configuration and counters
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::Storage::SetCashStorageResult> SetCashStorageAsync(
            const XFS4IoTFramework::Storage::SetCashStorageRequest& request,
            std::stop_token cancellation) override;

        /// <summary>
        /// Start cash unit exchange operation
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::Storage::StartExchangeResult> StartExchangeAsync(std::stop_token cancellation) override;

        /// <summary>
        /// Complete cash unit exchange operation
        /// </summary>
        boost::asio::awaitable<XFS4IoTFramework::Storage::EndExchangeResult> EndExchangeAsync(std::stop_token cancellation) override;

        //// Card storage operations - not supported
        //bool GetCardStorageConfiguration(
        //    std::map<std::string, CardUnitStorageConfiguration>& newCardUnits) override;
        //bool GetCardUnitCounts(
        //    std::map<std::string, CardUnitCount>& unitCounts) override;
        //bool GetCardUnitStatus(
        //    std::map<std::string, CardStatusClass::ReplenishmentStatusEnum>& unitStatus) override;
        //bool GetCardStorageStatus(
        //    std::map<std::string, CardUnitStorage::StatusEnum>& storageStatus) override;
        //std::future<SetCardStorageResult> SetCardStorageAsync(
        //    const SetCardStorageRequest& request,
        //    std::stop_token cancellation) override;

        //// Check storage operations - not supported
        //bool GetCheckStorageConfiguration(
        //    std::map<std::string, CheckUnitStorageConfiguration>& newCheckUnits) override;
        //bool GetCheckUnitCounts(
        //    std::map<std::string, StorageCheckCountClass>& unitCounts) override;
        //bool GetCheckUnitInitialCounts(
        //    std::map<std::string, StorageCheckCountClass>& initialCounts) override;
        //bool GetCheckStorageStatus(
        //    std::map<std::string, CheckUnitStorage::StatusEnum>& storageStatus) override;
        //bool GetCheckUnitStatus(
        //    std::map<std::string, CheckStatusClass::ReplenishmentStatusEnum>& unitStatus) override;
        //std::future<SetCheckStorageResult> SetCheckStorageAsync(
        //    const SetCheckStorageRequest& request,
        //    std::stop_token cancellation) override;

        //// Printer storage operations - not supported
        //bool GetPrinterStorageConfiguration(
        //    std::map<std::string, PrinterUnitStorageConfiguration>& newPrinterUnits) override;
        //bool GetPrinterUnitCounts(
        //    std::map<std::string, PrinterUnitCount>& unitCounts) override;
        //bool GetPrinterStorageStatus(
        //    std::map<std::string, PrinterUnitStorage::StatusEnum>& storageStatus) override;
        //bool GetPrinterUnitStatus(
        //    std::map<std::string, XFS4IoTFramework::Storage::PrinterStatusClass::ReplenishmentStatusEnum>& unitStatus) override;
        //std::future<SetPrinterStorageResult> SetPrinterStorageAsync(
        //    const SetPrinterStorageRequest& request,
        //    std::stop_token cancellation) override;

        //// IBNS storage operations - not supported
        //bool GetIBNSStorageInfo(
        //    std::map<std::string, IBNSStorageInfo>& newIBNSUnits) override;

        //// Deposit storage operations - not supported
        //bool GetDepositStorageConfiguration(
        //    std::map<std::string, DepositUnitStorageConfiguration>& newDepositUnits) override;
        //bool GetDepositUnitInfo(
        //    std::map<std::string, DepositUnitInfo>& unitCounts) override;
        //boost::asio::awaitable<SetPrinterStorageResult> SetDepositStorageAsync(
        //    const SetDepositStorageRequest& request,
        //    std::stop_token cancellation) override;

        // ============================================================================
        // Common Interface
        // ============================================================================

        /// <summary>
        /// Common Status
        /// </summary>
        std::shared_ptr<XFS4IoTFramework::Common::CommonStatusClass> GetCommonStatus() const override { return commonStatus_; }
        void SetCommonStatus(std::shared_ptr < XFS4IoTFramework::Common::CommonStatusClass> status) override {
            commonStatus_ = std::move(status);
            }

        /// <summary>
        /// Common Capabilities
        /// </summary>
        std::shared_ptr < XFS4IoTFramework::Common::CommonCapabilitiesClass> GetCommonCapabilities() const override { return commonCapabilities_; }
        void SetCommonCapabilities(std::shared_ptr <XFS4IoTFramework::Common::CommonCapabilitiesClass> capabilities) override {
            commonCapabilities_ = std::move(capabilities);
            
        }
        
        boost::asio::awaitable<XFS4IoTServer::DeviceResult> PowerSaveControl(
            int maxPowerSaveRecoveryTime,
            std::stop_token cancel) override;

        boost::asio::awaitable<XFS4IoTServer::DeviceResult> SetTransactionState(
            const XFS4IoTFramework::Common::SetTransactionStateRequest& request) override;

        boost::asio::awaitable<XFS4IoTFramework::Common::GetTransactionStateResult> GetTransactionState() override;

        boost::asio::awaitable<XFS4IoTFramework::Common::GetCommandNonceResult> GetCommandNonce() override;

        boost::asio::awaitable<XFS4IoTServer::DeviceResult> ClearCommandNonce() override;

        std::shared_ptr<XFS4IoTServer::IServiceProvider> GetServiceProvider() const override;
        void SetServiceProvider(std::shared_ptr<XFS4IoTServer::IServiceProvider> serviceProvider) override;

        void loggingChangeStatus(FS365::HW::Dors::DorsHW::POLL_RES currentState);

        /// Указатель на устройство
        std::unique_ptr<FS365::HW::Dors::DorsHW> m_pDevice;
        /// Идентификационные данные устройства
        FS365::HW::Dors::CIdentification m_idn;
        /// Текущее состояние устройства
        FS365::HW::Dors::DorsHW::POLL_RES m_State;
        /// Предыдущее состояние устройства
        FS365::HW::Dors::DorsHW::POLL_RES m_PreviousState;
        /// Дополнительный параметр состояния
        uint8_t m_bAdditionalRes;
        /// Состояние до начала операции сброса
        FS365::HW::Dors::DorsHW::POLL_RES m_stateBeforeResetStarted;
        /// Флаг сброса состояния устройства
        bool m_bResetOperationInProgress;
        /// Флаг режима приёма наличных
        bool bConjointCashInIsActive{ false };
        /// Флаг некорректной конфигурации ПО
        bool m_bSoftwareConfigurationFault;
        /// Флаг замены кассеты
        bool m_bCassetteHasBeenReplaced;
        /// Попытки восстановить порт
        int m_AttemptsToRestorePort = 0;
        /// Флаг, что устройство готово к обслуживанию
        bool m_SetAsReadyToServe = false;

        // Синхронизация доступа к внутренним полям
        std::mutex m_mtx;

        /// Обработчик переходов по состояниям стейт-машины
        StateMachine m_stateMachine;

        std::shared_ptr<ILogger> logger_;

        std::unique_ptr<PowerUpManager> powerUpManager_;

    protected:
        /// Поток опроса устройства
        std::unique_ptr<std::jthread> m_pollingThread;
        /// Служебные байты устройства
        std::vector<uint8_t> hwServiceBytes;
        /**  @brief Фоновый опрос устройства.*/
        void fnDevicePolling();


    private:

        // ============================================================================
        // Купюры
        // ============================================================================
        /// Флаг наличия кэша таблицы номиналов
        bool m_bHasBillTableCache = false;


        /// <summary>
        /// Thread for simulate cash taken event to be fired
        /// </summary>
        void CashTakenThread();

        /// <summary>
		/// Это класс для хранения информации о каждом cash unit storage, который может быть использован для формирования ответов на запросы статуса и конфигурации, а также для хранения актуальной информации о количестве купюр в каждой кассете. В реальной реализации он может быть расширен дополнительными полями и методами для управления состоянием и взаимодействия с устройством.
        /// </summary>
        class CashStorageInfo
        {
        public:
            CashStorageInfo(const XFS4IoTFramework::Storage::CashUnitStorageConfiguration& cashUnitStorageConfig)
                : storageStatus_(XFS4IoTFramework::Storage::CashUnitStorage::StatusEnum::Good)
                , unitStatus_(XFS4IoTFramework::Storage::CashStatusClass::ReplenishmentStatusEnum::Healthy)
                , unitCount_(nullptr, std::make_shared<XFS4IoTFramework::Storage::StorageCashInCountClass>(), 0)
                , cashUnitStorageConfig_(cashUnitStorageConfig)
                , accuracy_(cashUnitStorageConfig.GetCashUnitAdditionalInfo().GetAccuracySupported()
                    ? XFS4IoTFramework::Storage::CashStatusClass::AccuracyEnum::Unknown
                    : XFS4IoTFramework::Storage::CashStatusClass::AccuracyEnum::NotSupported)
            {
            }

			XFS4IoTFramework::Storage::CashUnitStorage::StatusEnum storageStatus_; // Статус хранения (например, Good, Inoperative, Missing, NotConfigured, Manipulated)
			XFS4IoTFramework::Storage::CashStatusClass::ReplenishmentStatusEnum unitStatus_; // Статус наличия (например, Healthy, Low, Empty, Jammed, Unknown)
			XFS4IoTFramework::Storage::CashUnitCountClass unitCount_; // Количество банкнот в кассете (может быть nullptr, если устройство не поддерживает точные счета)
			XFS4IoTFramework::Storage::CashUnitStorageConfiguration cashUnitStorageConfig_; // Конфигурация кассеты (например, поддерживаемые номиналы, емкость)
			XFS4IoTFramework::Storage::CashStatusClass::AccuracyEnum accuracy_; // Точность счетчика банкнот (например, Exact, Estimated, NotSupported)
        };

        // Указатели на сервисы
        std::shared_ptr<XFS4IoTServer::IServiceProvider> setServiceProvider_;
        std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorServiceProvider> cashAcceptorService_;

        // Status objects
        std::shared_ptr < XFS4IoTFramework::Common::CommonStatusClass> commonStatus_;
        std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorStatusClass> cashAcceptorStatus_;
        std::shared_ptr < XFS4IoTFramework::Common::CashManagementStatusClass> cashManagementStatus_;
        XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusClass positionStatus_;

        // Capabilities
        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass cashAcceptorCapabilities_;
        std::shared_ptr < XFS4IoTFramework::Common::CashManagementCapabilitiesClass> cashManagementCapabilities_;
        std::shared_ptr < XFS4IoTFramework::Common::CommonCapabilitiesClass> commonCapabilities_;

        // Cash-in status
        std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> cashInStatus_;
        XFS4IoTFramework::CashAcceptor::DeviceLockStatusClass deviceLockStatus_;

        // Информация о кэше купюр
        std::map<std::string, CashStorageInfo> cashUnitInfo_;

        // банкноты текущей cash-in transaction
        std::map<std::string, XFS4IoTFramework::Storage::CashItemCountClass> acceptedItems_;

		// все поддерживаемые банком номиналы банкнот
        const std::map<std::string, XFS4IoTFramework::Common::CashManagementCapabilitiesClass::BanknoteItem> allBanknoteIDs_;

        // Synchronization
        std::counting_semaphore<1> cashTakenSignal_;
        std::jthread cashTakenThread_;

        // Mutable banknote configuration (копия для изменения)
        std::map<std::string, XFS4IoTFramework::Common::CashManagementCapabilitiesClass::BanknoteItem> configurableBanknoteIDs_; // m_lBillTypes из XFS 3.x

        std::weak_ptr<StateMachine::BlockedWaitTermination> m_p_async_reset_terminator; /**< Токен отмены ожидания */
        
    };

} 