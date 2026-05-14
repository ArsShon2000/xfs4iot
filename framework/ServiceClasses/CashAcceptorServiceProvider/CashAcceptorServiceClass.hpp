#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include "../../core/Logger/ILogger.hpp"
#include "../../server/IServiceProvider.hpp"
#include "../CommonServiceProvider/ICommonService.hpp"
#include "../StorageServiceProvider/IStorageService.hpp"
#include "DeviceLockStatusClass.hpp"
#include "ICashAcceptorDevice.hpp"
#include "../../core/Events/PropertyChangedEventArgs.hpp"
#include "../CashManagementServiceProvider/CashInStatusClass.hpp"
//#include "CashAcceptorServiceProvider.hpp"
#include "../CommonServiceProvider/CashManagementStatusClass.hpp"
#include "../CommonServiceProvider/CashAcceptorCapabilitiesClass.hpp"
#include "../CashManagementServiceProvider/ICashManagementService.hpp"

namespace XFS4IoTServer
{
    //// Forward declarations
    class CashAcceptorServiceProvider;
    //class IServiceProvider;
    //class ICashAcceptorDevice;
    //class ICommonService;
    //class IStorageService;
    //class PropertyChangedEventArgs;
    //class DeviceLockStatusClass;
    //class CashAcceptorCapabilitiesClass;
    //class CashAcceptorStatusClass;*****

    //namespace XFS4IoTFramework::CashManagement
    //{
    //    class ICashManagementService;****
    //    class CashInStatusClass;
    //}

    //namespace Constants
    //{
    //    extern const std::string DeviceClass;****
    //}

    class CashAcceptorServiceClass
    {
    public:
        CashAcceptorServiceClass(
            std::shared_ptr<IServiceProvider> serviceProvider,
            std::shared_ptr<ILogger> logger);

        virtual ~CashAcceptorServiceClass();

        // Disable copy and move
        //CashAcceptorServiceClass(const CashAcceptorServiceClass&) = delete;
        //CashAcceptorServiceClass& operator=(const CashAcceptorServiceClass&) = delete;
        //CashAcceptorServiceClass(CashAcceptorServiceClass&&) = delete;
        //CashAcceptorServiceClass& operator=(CashAcceptorServiceClass&&) = delete;

        /// <summary>
        /// Common service interface
        /// </summary>
        std::shared_ptr<XFS4IoTFramework::Common::ICommonService> GetCommonService() const
        {
            return commonService_;
        }

        /// <summary>
        /// CashManagement service interface
        /// </summary>
        std::shared_ptr<XFS4IoTFramework::CashManagement::ICashManagementService> GetCashManagement() const
        {
            return cashManagement_;
        }

        /// <summary>
        /// Storage service interface
        /// </summary>
        std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> GetStorage() const
        {
            return storage_;
        }

        /// <summary>
        /// The information about the status of the currently active cash-in transaction or 
        /// in the case where no cash-in transaction is active the status of the most recently 
        /// ended cash-in transaction.
        /// </summary>
        std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> GetCashInStatus() const
        {
            return cashInStatus_;
        }

        /// <summary>
        /// The physical lock/unlock status of the CashAcceptor device and storages.
        /// </summary>
        XFS4IoTFramework::CashAcceptor::DeviceLockStatusClass GetDeviceLockStatus() const
        {
            return deviceLockStatus_;
        }

        /// <summary>
        /// The deplete target and destination information
        /// Key - The storage id can be used for target of the depletion operation.
        /// Value - List of storage id can be used for source of the depletion operation
        /// </summary>
        const std::map<std::string, std::vector<std::string>>& GetDepleteCashUnitSources() const
        {
            return depleteCashUnitSources_;
        }

        /// <summary>
        /// Which storage units can be specified as targets for a given source storage unit 
        /// with the CashAcceptor.Replenish command
        /// </summary>
        const std::vector<std::string>& GetReplenishTargets() const
        {
            return replenishTargets_;
        }

    protected:
        std::shared_ptr<IServiceProvider> serviceProvider_;
        std::shared_ptr<ILogger> logger_;
        std::shared_ptr<XFS4IoTFramework::CashAcceptor::ICashAcceptorDevice> device_;

    private:
        void GetCapabilities();
        void getStatus();

        /// <summary>
        /// Status changed event forwarder
        /// </summary>
        void statusChangedEventForwarder(
            std::shared_ptr<void> sender,
            std::shared_ptr<PropertyChangedEventArgs> propertyInfo);

        std::shared_ptr<XFS4IoTFramework::Common::ICommonService> commonService_;
        std::shared_ptr<XFS4IoTFramework::CashManagement::ICashManagementService> cashManagement_;
        std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage_;
        std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> cashInStatus_;
        XFS4IoTFramework::CashAcceptor::DeviceLockStatusClass deviceLockStatus_;
        std::map<std::string, std::vector<std::string>> depleteCashUnitSources_;
        std::vector<std::string> replenishTargets_;
    };
}