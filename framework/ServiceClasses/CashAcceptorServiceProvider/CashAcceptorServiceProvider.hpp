// CashAcceptorServiceProvider.hpp
#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include "../../server/EndpointDetails.hpp"
#include "../../server/IDevice.hpp"
#include "../../server/ServiceProvider.hpp"
#include "../../core/Logger/ILogger.hpp"
#include "../../core/Persistent/IPersistentData.hpp"
#include "../../core/common/XFSConstants.hpp"
#include <boost/asio/awaitable.hpp>
#include "ICashAcceptorService.hpp"
#include "../CommonServiceProvider/CashManagementCapabilitiesClass.hpp"
#include "../CommonServiceProvider/ICommonService.hpp"
#include "../CommonServiceProvider/CommonServiceClass.hpp"
#include "../StorageServiceProvider/IStorageService.hpp"
#include "../StorageServiceProvider/StorageServiceClass.hpp"
#include "../CashManagementServiceProvider/ICashManagementService.hpp"
#include "../CashManagementServiceProvider/CashManagementServiceClass.hpp"
#include "CashAcceptorServiceClass.hpp"
#include "../CommonServiceProvider/CommonStatusClass.hpp"
#include "../CashManagementServiceProvider/CashInStatusClass.hpp"

namespace XFS4IoTServer
{
    /// <summary>
    /// Default implementation of a cash acceptor service provider.
    /// This Service allows to combine CashDispenser for a cash recycler device, or
    /// CheckScanner for cash and check recycler configuration with IBNS.
    /// </summary>
    class CashAcceptorServiceProvider : public ServiceProvider,
        public ICashAcceptorService,
        public XFS4IoTFramework::CashManagement::ICashManagementService,
        public XFS4IoTFramework::Common::ICommonService,
        public XFS4IoTFramework::Storage::IStorageService
        //, public std::enable_shared_from_this<CashAcceptorServiceProvider>
    {

    public:
        CashAcceptorServiceProvider(
            const EndpointDetails& endpointDetails,
            const std::string& serviceName,
            std::shared_ptr<IDevice> device,
            std::shared_ptr<ILogger> logger,
            std::shared_ptr<IPersistentData> persistentData);

        virtual ~CashAcceptorServiceProvider() = default;

        // Инициализация дочерних сервисов — вызывать сразу после std::make_shared(...)
        void Initialize(std::shared_ptr<ILogger> logger, std::shared_ptr<IPersistentData> persistentData);


        // ============================================================================
        // CashManagement unsolicited events
        // ============================================================================

        /// <summary>
        /// Items taken event
        /// </summary>
        boost::asio::awaitable<void> ItemsTakenEvent(
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position,
            std::optional<std::string> additionalBunches = std::nullopt) override;

        /// <summary>
        /// Items inserted event
        /// </summary>
        boost::asio::awaitable<void> ItemsInsertedEvent(
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position) override;

        /// <summary>
        /// Items presented event
        /// </summary>
        boost::asio::awaitable<void> ItemsPresentedEvent(
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position,
            std::optional<std::string> additionalBunches) override;

        // ============================================================================
        // Storage Service
        // ============================================================================

        /// <summary>
        /// Update storage count from the framework after media movement command is processed
        /// </summary>
        boost::asio::awaitable<void> UpdateCardStorageCount(
            const std::string& storageId,
            int countDelta) override;

        /// <summary>
        /// UpdateCashAccounting
        /// Update cash unit status and counts managed by the device specific class.
        /// </summary>
        boost::asio::awaitable<void> UpdateCashAccounting(
            const std::optional < std::unordered_map<std::string, std::shared_ptr<XFS4IoTFramework::Storage::CashUnitCountClass>>> countDelta = {}) override;

        ///// <summary>
        ///// Update managed check storage information in the framework.
        ///// </summary>
        //boost::asio::awaitable<void> UpdateCheckStorageCount(
        //    const std::map<std::string, StorageCheckCountClass>& countDelta = {}) override;

        ///// <summary>
        ///// Update managed printer storage information in the framework.
        ///// </summary>
        //boost::asio::awaitable<void> UpdatePrinterStorageCount(
        //    const std::string& storageId,
        //    int countDelta) override;

        /// <summary>
        /// Update managed deposit storage information in the framework.
        /// </summary>
        boost::asio::awaitable<void> UpdateDepositStorageCount(
            const std::string& storageId,
            int countDelta) override;

        /// <summary>
        /// Store persistent data
        /// </summary>
        void StorePersistent() override;

        /// <summary>
        /// Return which type of storage SP is using
        /// </summary>
        XFS4IoTFramework::Storage::StorageTypeEnum GetStorageType() const override;

        ///// <summary>
        ///// Card storage structure information of this device
        ///// </summary>
        //const std::map<std::string, CardUnitStorage>& GetCardUnits() const override;

        /// <summary>
        /// Cash storage structure information of this device
        /// </summary>
        const std::unordered_map<
            std::string, 
            std::shared_ptr<XFS4IoTFramework::Storage::CashUnitStorage>
        >& GetCashUnits() const override;

        ///// <summary>
        ///// Check storage structure information of this device
        ///// </summary>
        //const std::map<std::string, CheckUnitStorage>& GetCheckUnits() const override;

        ///// <summary>
        ///// Deposit storage structure information of this device
        ///// </summary>
        //const std::map<std::string, DepositUnitStorage>& GetDepositUnits() const override;

        /// <summary>
        /// Return XFS4IoT storage structured object.
        /// </summary>
        std::unordered_map<std::string, std::shared_ptr<XFS4IoT::Storage::StorageUnitClass>> GetStorages(
            const std::vector<std::string>& unitIds) override;

        // ============================================================================
        // Storage Unsolicited events
        // ============================================================================

        boost::asio::awaitable<void> MediaDetectedEvent(
            std::optional<std::string> storageId,
            XFS4IoTFramework::CashManagement::ItemTargetEnum target,
            std::optional<int> index = std::nullopt) override;

        /// <summary>
        /// Sending status changed event.
        /// </summary>
        boost::asio::awaitable<void> StorageChangedEvent(
            std::shared_ptr<void> sender,
            std::shared_ptr<XFS4IoTServer::PropertyChangedEventArgs> propertyInfo) override;

        // ============================================================================
        // Common unsolicited events
        // ============================================================================

        boost::asio::awaitable<void> StatusChangedEvent(
            std::shared_ptr<void> sender,
            std::shared_ptr<PropertyChangedEventArgs> propertyInfo) override;

        boost::asio::awaitable<void> NonceClearedEvent(
            const std::string& reasonDescription) override;

        boost::asio::awaitable<void> ErrorEvent(
            XFS4IoTFramework::Common::CommonStatusClass::ErrorEventIdEnum eventId,
            XFS4IoTFramework::Common::CommonStatusClass::ErrorActionEnum action,
            const std::string& vendorDescription) override;

        // ============================================================================
        // Common Service
        // ============================================================================

        /// <summary>
        /// Stores Common interface capabilities internally
        /// </summary>
        std::shared_ptr<XFS4IoTFramework::Common::CommonCapabilitiesClass> GetCommonCapabilities() const override;
        void SetCommonCapabilities(std::shared_ptr<XFS4IoTFramework::Common::CommonCapabilitiesClass> capabilities) override;

        /// <summary>
        /// Common Status
        /// </summary>
        std::shared_ptr<XFS4IoTFramework::Common::CommonStatusClass> GetCommonStatus() const override;
        void SetCommonStatus(std::shared_ptr<XFS4IoTFramework::Common::CommonStatusClass> status) override;

        /// <summary>
        /// Stores CashAcceptor interface capabilities internally
        /// </summary>
        std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass> GetCashAcceptorCapabilities() const override;
        void SetCashAcceptorCapabilities(std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass> capabilities) override;

        /// <summary>
        /// CashAcceptor Status
        /// </summary>
        std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorStatusClass> GetCashAcceptorStatus() const override;
        void SetCashAcceptorStatus(std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorStatusClass> status) override;

        /// <summary>
        /// Stores CashManagement interface capabilities internally
        /// </summary>
        std::shared_ptr<XFS4IoTFramework::Common::CashManagementCapabilitiesClass> GetCashManagementCapabilities() const override;
        void SetCashManagementCapabilities(std::shared_ptr<XFS4IoTFramework::Common::CashManagementCapabilitiesClass> capabilities) override;

        /// <summary>
        /// CashManagement Status
        /// </summary>
        std::shared_ptr<XFS4IoTFramework::Common::CashManagementStatusClass> GetCashManagementStatus() const override;
        void SetCashManagementStatus(const std::shared_ptr<XFS4IoTFramework::Common::CashManagementStatusClass> status) override;

        // ============================================================================
        // CashManagement Service
        // ============================================================================

        /// <summary>
        /// The framework maintains cash-in status
        /// </summary>
        const std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> GetCashInStatusManaged() const override;

        /// <summary>
        /// Store cash-in in status persistently
        /// </summary>
        void StoreCashInStatus() override;

        /// <summary>
        /// The last status of the most recent attempt to present or return items to the customer.
        /// </summary>
        const std::unordered_map<XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum,
            std::shared_ptr<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>>&
            GetLastCashManagementPresentStatus() const override;

        /// <summary>
        /// Store present status persistently
        /// </summary>
        void StoreCashManagementPresentStatus() override;

        /// <summary>
        /// This list provides the functionality to blacklist notes and allows additional flexibility.
        /// </summary>
        const std::shared_ptr<XFS4IoTFramework::CashManagement::ItemClassificationListClass>& GetItemClassificationList() const override;

        /// <summary>
        /// Store classification list persistently
        /// </summary>
        void StoreItemClassificationList() override;

        // ============================================================================
        // CashAcceptor Service
        // ============================================================================

        /// <summary>
        /// The information about the status of the currently active cash-in transaction.
        /// </summary>
        std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> GetCashInStatus() const override;

        /// <summary>
        /// The physical lock/unlock status of the CashAcceptor device and storages.
        /// </summary>
        XFS4IoTFramework::CashAcceptor::DeviceLockStatusClass GetDeviceLockStatus() const override;

        /// <summary>
        /// The deplete target and destination information
        /// </summary>
        const std::map<std::string, std::vector<std::string>>&
            GetDepleteCashUnitSources() const override;

        /// <summary>
        /// Which storage units can be specified as targets for replenish
        /// </summary>
        const std::vector<std::string>& GetReplenishTargets() const override;

    private:
        std::shared_ptr<CashAcceptorServiceClass> cashAcceptor_;
        std::shared_ptr<CashManagementServiceClass> cashManagementService_;
        std::shared_ptr<StorageServiceClass> storageService_;
        std::shared_ptr<CommonServiceClass> commonService_;

        //// Optional services
        //std::shared_ptr<CashDispenserServiceClass> cashDispenserService_;
        //std::shared_ptr<BanknoteNeutralizationServiceClass> ibnsService_;
        //std::shared_ptr<CheckServiceClass> checkService_;
    };

} // namespace XFS4IoTServer