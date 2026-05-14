#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <boost/asio/awaitable.hpp>
#include "../IStorageService.hpp"
#include "../../ServiceInterfaces/CashManagement/ICalibrateCashUnitEvents.hpp"
#include "../../ServiceInterfaces/CashManagement/IRetractEvents.hpp"
#include "../../ServiceInterfaces/CashManagement/IResetEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashInEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/IReplenishEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashInEndEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashInRollbackEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashUnitCountEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/IPreparePresentEvents.hpp"
#include "../DeviceParameters.hpp"
#include "../../ServiceInterfaces/CashDispanser/IDispenseEvents.hpp"
#include "../../ServiceInterfaces/CashDispanser/ITestCashUnitsEvents.hpp"
#include "../../ServiceInterfaces/CashDispanser/ICountEvents.hpp"
#include "../../ServiceInterfaces/CashDispanser/IRejectEvents.hpp"

namespace XFS4IoTFramework::Storage
{
    class StorageErrorCommandEvent
    {
    public:
#pragma region CashManagement

        /// <summary>
        /// Constructor for CalibrateCashUnit events
        /// </summary>
        StorageErrorCommandEvent(
            std::shared_ptr<IStorageService> storageService,
            std::shared_ptr<XFS4IoTFramework::CashManagement::ICalibrateCashUnitEvents> events);

        /// <summary>
        /// Constructor for Reset events
        /// </summary>
        StorageErrorCommandEvent(
            std::shared_ptr<IStorageService> storageService,
            std::shared_ptr<XFS4IoTFramework::CashManagement::IResetEvents> events);

        /// <summary>
        /// Constructor for Retract events
        /// </summary>
        StorageErrorCommandEvent(
            std::shared_ptr<IStorageService> storageService,
            std::shared_ptr<XFS4IoTFramework::CashManagement::IRetractEvents> events);

#pragma endregion

#pragma region CashAcceptor

        /// <summary>
        /// Constructor for CashIn events
        /// </summary>
        StorageErrorCommandEvent(
            std::shared_ptr<IStorageService> storageService,
            std::shared_ptr<XFS4IoTFramework::CashAcceptor::ICashInEvents> events);

        /// <summary>
        /// Constructor for Replenish events
        /// </summary>
        StorageErrorCommandEvent(
            std::shared_ptr<IStorageService> storageService,
            std::shared_ptr<XFS4IoTFramework::CashAcceptor::IReplenishEvents> events);

        /// <summary>
        /// Constructor for CashInEnd events
        /// </summary>
        StorageErrorCommandEvent(
            std::shared_ptr<IStorageService> storageService,
            std::shared_ptr<XFS4IoTFramework::CashManagement::ICashInEndEvents> events);

        /// <summary>
        /// Constructor for CashInRollback events
        /// </summary>
        StorageErrorCommandEvent(
            std::shared_ptr<IStorageService> storageService,
            std::shared_ptr<XFS4IoTFramework::CashManagement::ICashInRollbackEvents> events);

        /// <summary>
        /// Constructor for CashUnitCount events
        /// </summary>
        StorageErrorCommandEvent(
            std::shared_ptr<IStorageService> storageService,
            std::shared_ptr<XFS4IoTFramework::CashManagement::ICashUnitCountEvents> events);

        /// <summary>
        /// Constructor for PreparePresent events
        /// </summary>
        StorageErrorCommandEvent(
            std::shared_ptr<IStorageService> storageService,
            std::shared_ptr<XFS4IoTFramework::CashManagement::IPreparePresentEvents> events);


#pragma endregion

        #pragma region CashDispenser
        
                /// <summary>
                /// Constructor for Dispense events
                /// </summary>
                StorageErrorCommandEvent(
                    std::shared_ptr<IStorageService> storageService,
                    std::shared_ptr<XFS4IoTFramework::CashDispenser::IDispenseEvents> events);
        
                /// <summary>
                /// Constructor for Reject events
                /// </summary>
                StorageErrorCommandEvent(
                    std::shared_ptr<IStorageService> storageService,
                    std::shared_ptr<XFS4IoTFramework::CashDispenser::IRejectEvents> events);
        
                /// <summary>
                /// Constructor for TestCashUnits events
                /// </summary>
                StorageErrorCommandEvent(
                    std::shared_ptr<IStorageService> storageService,
                    std::shared_ptr<XFS4IoTFramework::CashDispenser::ITestCashUnitsEvents> events);
        
                /// <summary>
                /// Constructor for Count events
                /// </summary>
                StorageErrorCommandEvent(
                    std::shared_ptr<IStorageService> storageService,
                    std::shared_ptr<XFS4IoTFramework::CashDispenser::ICountEvents> events);
        
        #pragma endregion

        virtual ~StorageErrorCommandEvent() = default;

        // Disable copy and move
        StorageErrorCommandEvent(const StorageErrorCommandEvent&) = delete;
        StorageErrorCommandEvent& operator=(const StorageErrorCommandEvent&) = delete;
        StorageErrorCommandEvent(StorageErrorCommandEvent&&) = delete;
        StorageErrorCommandEvent& operator=(StorageErrorCommandEvent&&) = delete;

        /// <summary>
        /// Send storage error event to appropriate event interface
        /// </summary>
        boost::asio::awaitable<void> StorageErrorEvent(
            FailureEnum failure,
            const std::vector<std::string>& unitIds);

    private:
        std::shared_ptr<IStorageService> storageService_;

#pragma region CashManagement
        std::shared_ptr<XFS4IoTFramework::CashManagement::IRetractEvents> retractEvents_;
        std::shared_ptr<XFS4IoTFramework::CashManagement::IResetEvents> cashManagementResetEvents_;
        std::shared_ptr<XFS4IoTFramework::CashManagement::ICalibrateCashUnitEvents> calibrateCashUnitEvents_;
#pragma endregion

        #pragma region CashDispenser
                std::shared_ptr<CashDispenser::IDispenseEvents> dispenseEvents_;
                std::shared_ptr<XFS4IoTFramework::CashDispenser::IRejectEvents> rejectEvents_;
                std::shared_ptr<CashDispenser::ITestCashUnitsEvents> testCashUnitsEvents_;
                std::shared_ptr<CashDispenser::ICountEvents> countEvents_;
        #pragma endregion

#pragma region CashAcceptor
        std::shared_ptr<XFS4IoTFramework::CashAcceptor::ICashInEvents> cashInEvents_;
        std::shared_ptr<XFS4IoTFramework::CashManagement::ICashInEndEvents> cashInEndEvents_;
        std::shared_ptr<XFS4IoTFramework::CashManagement::ICashInRollbackEvents> cashInRollbackEvents_;
        std::shared_ptr<XFS4IoTFramework::CashManagement::IPreparePresentEvents> preparePresentEvents_;
        std::shared_ptr<XFS4IoTFramework::CashManagement::ICashUnitCountEvents> cashUnitCountEvents_;
        //std::shared_ptr<XFS4IoTFramework::CashManagement::IDepleteEvents> depleteEvents_;
        std::shared_ptr<XFS4IoTFramework::CashAcceptor::IReplenishEvents> replenishEvents_;
#pragma endregion
    };
}