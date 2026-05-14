#pragma once

#include <memory>
#include <vector>
#include <map>
#include <string>
#include <stop_token>
#include <boost/asio/awaitable.hpp>

#include "../../server/IDevice.hpp"
#include "../CommonServiceProvider/CashAcceptorStatusClass.hpp"
#include "../StorageServiceProvider/CashUnit.hpp"
#include "DeviceLockStatusClass.hpp"
#include "../CommonServiceProvider/CashAcceptorCapabilitiesClass.hpp"
#include "../../server/IServiceProvider.hpp"
//#include "../CashManagementServiceProvider/CashInStatus.hpp"
#include "DeviceParameters.hpp"
#include "Events/CashInCommandEvents.hpp"
#include "Events/CashInEndCommandEvents.hpp"
#include "Events/CashInRollbackCommandEvents.hpp"
#include "Events/CashUnitCountCommandEvents.hpp"
#include "Events/PreparePresentCommandEvents.hpp"
#include "../CashManagementServiceProvider/CashInStatusClass.hpp"


namespace XFS4IoTFramework::CashAcceptor
{
    /// <summary>
    /// Cash Acceptor Device Interface
    /// </summary>
    class ICashAcceptorDevice : public virtual XFS4IoTServer::IDevice
    {
    public:
        ~ICashAcceptorDevice() override = default;

        /// <summary>
        /// Before initiating a cash-in operation, an application must issue the CashInStart command to begin a cash-in transaction.
        /// During a cash-in transaction any number of CashIn commands may be issued.
        /// The transaction is ended when either a CashInRollback, CashInEnd, Storage.Retract or Reset command is sent.
        /// </summary>
        virtual boost::asio::awaitable<CashInStartResult> CashInStart(
            const CashInStartRequest& request,
            std::stop_token cancellation) = 0;

        /// <summary>
        /// This command moves items into the cash device from an input position.
        /// On devices with implicit shutter control, the CashAcceptor.InsertItemsEvent will be
        /// generated when the device is ready to start accepting media.
        /// </summary>
        virtual boost::asio::awaitable<CashInResult> CashIn(
            std::shared_ptr<CashInCommandEvents> events,
            const CashInRequest& request,
            std::stop_token cancellation) = 0;

        /// <summary>
        /// This command ends a cash-in transaction. If cash items are on the stacker as a result of a
        /// CashAcceptor.CashIn command these items are moved to the appropriate storage units.
        /// </summary>
        virtual boost::asio::awaitable<CashInEndResult> CashInEnd(
            std::shared_ptr<CashInEndCommandEvents> events,
            std::stop_token cancellation) = 0;

        /// <summary>
        /// This command is used to roll back a cash-in transaction. It causes all the cash items cashed in since the last
        /// CashAcceptor.CashInStart command to be returned to the customer.
        /// </summary>
        virtual boost::asio::awaitable<CashInRollbackResult> CashInRollback(
            std::shared_ptr<CashInRollbackCommandEvents> events,
            std::stop_token cancellation) = 0;

        /// <summary>
        /// This command is used to change the note types the banknote reader should accept during cash-in.
        /// Only note types which are to be changed need to be specified in the command payload.
        /// The values set by this command are persistent.
        /// </summary>
        virtual boost::asio::awaitable<ConfigureNoteTypesResult> ConfigureNoteTypes(
            const ConfigureNoteTypesRequest& request,
            std::stop_token cancellation) = 0;

        /// <summary>
        /// This command is used to configure the currency description configuration data into the banknote reader module.
        /// The format and location of the configuration data is vendor and/or hardware dependent.
        /// </summary>
        virtual boost::asio::awaitable<ConfigureNoteReaderResult> ConfigureNoteReader(
            const ConfigureNoteReaderRequest& request,
            std::stop_token cancellation) = 0;

        /// <summary>
        /// This command counts the items in the storage unit(s). If it is necessary to move items internally to count them,
        /// the items should be returned to the unit from which they originated before completion of the command.
        /// </summary>
        virtual boost::asio::awaitable<CashUnitCountResult> CashUnitCount(
            std::shared_ptr<CashUnitCountCommandEvents> events,
            const CashUnitCountRequest& request,
            std::stop_token cancellation) = 0;

        /// <summary>
        /// This command can be used to lock or unlock a CashAcceptor device or one or more storage units.
        /// </summary>
        virtual boost::asio::awaitable<DeviceLockResult> DeviceLockControl(
            const DeviceLockRequest& request,
            std::stop_token cancellation) = 0;

        /// <summary>
        /// This command opens the shutter and presents items to be taken by the customer.
        /// The shutter is automatically closed after the media is taken.
        /// </summary>
        virtual boost::asio::awaitable<PresentMediaResult> PresentMedia(
            const PresentMediaRequest& request,
            std::stop_token cancellation) = 0;


        /// <summary>
        /// In cases where multiple bunches are to be returned under explicit shutter control,
        /// this command is used for the purpose of moving a remaining bunch to the output position explicitly.
        /// </summary>
        virtual boost::asio::awaitable<PreparePresentResult> PreparePresent(
            std::shared_ptr<PreparePresentCommandEvents> events,
            const PreparePresentRequest& request,
            std::stop_token cancellation) = 0;

        /// <summary>
        /// The deplete target and destination information.
        /// Key - The storage id can be used for target of the depletion operation.
        /// Value - List of storage id can be used for source of the depletion operation.
        /// </summary>
        virtual std::map<std::string, std::vector<std::string>> GetDepleteCashUnitSources() = 0;

        /// <summary>
        /// Which storage units can be specified as targets for a given source storage unit
        /// with the CashAcceptor.Replenish command.
        /// </summary>
        virtual std::vector<std::string> GetReplenishTargets() = 0;

        /// <summary>
        /// CashAcceptor Status
        /// </summary>
        virtual const XFS4IoTFramework::Common::CashAcceptorStatusClass& GetCashAcceptorStatus() const = 0;
        virtual void SetCashAcceptorStatus(std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorStatusClass> status) = 0;

        /// <summary>
        /// CashAcceptor Capabilities
        /// </summary>
        virtual XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass GetCashAcceptorCapabilities() const = 0;
        virtual void SetCashAcceptorCapabilities(XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass capabilities) = 0;

        /// <summary>
        /// Status of current cash-in operation.
        /// If this property is set to null, the framework maintains cash-in status.
        /// </summary>
        virtual std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> GetCashInStatus() const = 0;
        virtual void SetCashInStatus(std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> status) = 0;

        /// <summary>
        /// The physical lock/unlock status of the CashAcceptor device and storages.
        /// </summary>
        virtual DeviceLockStatusClass GetDeviceLockStatus() const = 0;
        virtual void SetDeviceLockStatus(DeviceLockStatusClass status) = 0;
    };

    /// <summary>
    /// Base implementation of ICashAcceptorDevice providing common functionality
    /// </summary>
    class CashAcceptorDeviceBase : public ICashAcceptorDevice
    {
    public:
        CashAcceptorDeviceBase() = default;
        ~CashAcceptorDeviceBase() override = default;

        // IDevice implementation
        boost::asio::awaitable<void> RunAsync(std::stop_token token) override
        {
            co_return;
        }

        void SetServiceProvider(std::shared_ptr<XFS4IoTServer::IServiceProvider> serviceProvider) override
        {
            serviceProvider_ = serviceProvider;
        }

        std::shared_ptr<XFS4IoTServer::IServiceProvider> GetServiceProvider() const override
        {
            return serviceProvider_;
        }

        // ICashAcceptorDevice property implementations
        const XFS4IoTFramework::Common::CashAcceptorStatusClass& GetCashAcceptorStatus() const override
        {
            std::lock_guard<std::mutex> lock(statusMutex_);
            return cashAcceptorStatus_;
        }

       /* void SetCashAcceptorStatus(XFS4IoTFramework::Common::CashAcceptorStatusClass status) override
        {
            std::lock_guard<std::mutex> lock(statusMutex_);
            cashAcceptorStatus_ = std::move(status);
        }*/

        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass GetCashAcceptorCapabilities() const override
        {
            std::lock_guard<std::mutex> lock(capabilitiesMutex_);
            return cashAcceptorCapabilities_;
        }

        void SetCashAcceptorCapabilities(XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass capabilities) override
        {
            std::lock_guard<std::mutex> lock(capabilitiesMutex_);
            cashAcceptorCapabilities_ = std::move(capabilities);
        }

        std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> GetCashInStatus() const override
        {
            std::lock_guard<std::mutex> lock(cashInStatusMutex_);
            return cashInStatus_;
        }

        void SetCashInStatus(std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> status) override
        {
            std::lock_guard<std::mutex> lock(cashInStatusMutex_);
            cashInStatus_ = status;
        }

        DeviceLockStatusClass GetDeviceLockStatus() const override
        {
            std::lock_guard<std::mutex> lock(lockStatusMutex_);
            return deviceLockStatus_;
        }

        void SetDeviceLockStatus(DeviceLockStatusClass status) override
        {
            std::lock_guard<std::mutex> lock(lockStatusMutex_);
            deviceLockStatus_ = std::move(status);
        }

        std::map<std::string, std::vector<std::string>> GetDepleteCashUnitSources() override
        {
            std::lock_guard<std::mutex> lock(depleteMutex_);
            return depleteCashUnitSources_;
        }

        std::vector<std::string> GetReplenishTargets() override
        {
            std::lock_guard<std::mutex> lock(replenishMutex_);
            return replenishTargets_;
        }

    protected:
        std::shared_ptr<XFS4IoTServer::IServiceProvider> serviceProvider_;

        mutable std::mutex statusMutex_;
        XFS4IoTFramework::Common::CashAcceptorStatusClass cashAcceptorStatus_;

        mutable std::mutex capabilitiesMutex_;
        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass cashAcceptorCapabilities_;

        mutable std::mutex cashInStatusMutex_;
        std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> cashInStatus_;

        mutable std::mutex lockStatusMutex_;
        DeviceLockStatusClass deviceLockStatus_;

        mutable std::mutex depleteMutex_;
        std::map<std::string, std::vector<std::string>> depleteCashUnitSources_;

        mutable std::mutex replenishMutex_;
        std::vector<std::string> replenishTargets_;
    };
}