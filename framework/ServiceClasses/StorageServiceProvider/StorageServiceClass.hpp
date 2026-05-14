#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <boost/asio/awaitable.hpp>
#include <nlohmann/json.hpp>
#include "IStorageService.hpp"
#include "../../core/Logger/ILogger.hpp"
#include "CashUnit.hpp"
#include "IStorageDevice.hpp"
#include "../CommonServiceProvider/ICommonService.hpp"
#include "../../core/Persistent/IPersistentData.hpp"
#include "../../server/IServiceProvider.hpp"
#include "../../core/Events/PropertyChangedEventArgs.hpp"

namespace XFS4IoTServer
{
    // Forward declarations
    //class IServiceProvider;
    //class ILogger;
    //class IPersistentData;
    //class IStorageDevice;
    //class ICommonService;
    //class PropertyChangedEventArgs;
    //class CashUnitStorage;
    //class CashUnitStorageConfiguration;
    //class CashUnitCountClass;
    //class StorageCheckCountClass;


    class StorageServiceClass
    {
    public:
        StorageServiceClass(
            std::shared_ptr<IServiceProvider> serviceProvider,
            std::shared_ptr<ILogger> logger,
            std::shared_ptr<IPersistentData> persistentData,
            XFS4IoTFramework::Storage::StorageTypeEnum storageType);

        virtual ~StorageServiceClass();

        // Disable copy and move
        //StorageServiceClass(const StorageServiceClass&) = delete;
        //StorageServiceClass& operator=(const StorageServiceClass&) = delete;
        //StorageServiceClass(StorageServiceClass&&) = delete;
        //StorageServiceClass& operator=(StorageServiceClass&&) = delete;

#pragma region Cash

        /// <summary>
        /// UpdateCashAccounting
        /// Update cash unit status and counts managed by the device specific class.
        /// </summary>
        boost::asio::awaitable<void> UpdateCashAccounting(
            std::optional<std::unordered_map<std::string, std::shared_ptr<XFS4IoTFramework::Storage::CashUnitCountClass>>> countDelta = std::nullopt);

#pragma endregion

        /// <summary>
        /// Store all unit information persistently
        /// </summary>
        void StorePersistent();

        /// <summary>
        /// Type of storage
        /// </summary>
        XFS4IoTFramework::Storage::StorageTypeEnum GetStorageType() const { return storageType_; }

        /// <summary>
        /// Cash storage structure information of this device
        /// </summary>
        const std::unordered_map<std::string, std::shared_ptr<XFS4IoTFramework::Storage::CashUnitStorage>>& GetCashUnits() const
        {
            return cashUnits_;
        }

        /// <summary>
        /// Return XFS4IoT storage structured object.
        /// </summary>
        std::unordered_map<std::string, std::shared_ptr<XFS4IoT::Storage::StorageUnitClass>>
            GetStorages(const std::vector<std::string>& unitIds);

#pragma region Events

        boost::asio::awaitable<void> StorageChangedEvent(
            std::shared_ptr<void> sender,
            std::shared_ptr<PropertyChangedEventArgs> propertyInfo);

#pragma endregion

    protected:
        /// <summary>
        /// Storage or Count changed event handler to be implemented by derived class
        /// </summary>
        virtual boost::asio::awaitable<void> StorageChangedEventHandler(
            std::shared_ptr<void> sender,
            std::shared_ptr<PropertyChangedEventArgs> propertyInfo)
        {
            // Default no-op handler. Derived service classes may override to provide specific processing.
            co_return;
        }

        /// <summary>
        /// Storage threshold event to be implemented by derived class
        /// </summary>
        virtual boost::asio::awaitable<void> StorageThresholdEvent(
            std::shared_ptr<void> payload)
        {
            // Default no-op handler. Derived service classes may override to provide specific processing.
            co_return;
        }

        std::shared_ptr<IServiceProvider> serviceProvider_;
        std::shared_ptr<ILogger> logger_;
        std::shared_ptr<XFS4IoTFramework::Storage::IStorageDevice> device_;
        std::shared_ptr<XFS4IoTFramework::Common::ICommonService> commonService_;

    private:
        /// <summary>
        /// ConstructCashUnits
        /// The method retrieve cash unit structures from the device class. 
        /// The device class must provide cash unit structure information.
        /// </summary>
        void ConstructCashUnits();

        /// <summary>
        /// Update delta counts to the destination of the StorageCashCountClass object
        /// </summary>
        void UpdateDeltaStorageCashCount(
            const std::string& storageId,
            std::shared_ptr<XFS4IoTFramework::Storage::StorageCashCountClass> storageCashCount,
            std::shared_ptr<XFS4IoTFramework::Storage::StorageCashCountClass> storageDeltaCount);

        /// <summary>
        /// Register status changed event to all units supported
        /// </summary>
        void RegisterStorageChangedEvents();

        /// <summary>
        /// Storage or Count changed event forwarder
        /// </summary>
        void StorageChangedEventForwarder(
            std::shared_ptr<void> sender,
            std::shared_ptr<PropertyChangedEventArgs> propertyInfo);

        struct CashUnitStoragePersistent
        {
            int count = 0;
            int status = 0;

            friend void to_json(nlohmann::json& j, const CashUnitStoragePersistent& value)
            {
                j = nlohmann::json{
                    {"count", value.count},
                    {"status", value.status}
                };
            }

            friend void from_json(const nlohmann::json& j, CashUnitStoragePersistent& value)
            {
                value.count = j.value("count", 0);
                value.status = j.value("status", 0);
            }
        };


        using CashUnitStorageMapPersistent =
            std::unordered_map<std::string, CashUnitStoragePersistent>;

        std::shared_ptr<CashUnitStorageMapPersistent> persistedCashUnits_;

        std::string CashUnitsPersistentKey() const;

        void LoadPersistent();
        void ApplyPersistentCashUnits();

        std::shared_ptr<IPersistentData> persistentData_;
        XFS4IoTFramework::Storage::StorageTypeEnum storageType_;
        std::unordered_map<std::string, std::shared_ptr<XFS4IoTFramework::Storage::CashUnitStorage>> cashUnits_;
    };
}