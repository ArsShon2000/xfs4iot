#pragma once

#include <memory>
#include <stop_token>
#include <boost/asio/awaitable.hpp>

#include "../../server/IDevice.hpp"
#include "../CommonServiceProvider/CashManagementStatusClass.hpp"
#include "DeviceParameters.hpp"
#include "Events/RetractCommandEvents.hpp"
#include "Events/ResetCommandEvents.hpp"
#include "Events/CalibrateCashUnitCommandEvents.hpp"

//// Forward declarations
//namespace XFS4IoTFramework::CashManagement
//{
//    class RetractCommandEvents;
//    class RetractRequest;
//    class RetractResult;
//    class OpenCloseShutterRequest;
//    class OpenCloseShutterResult;
//    class ResetCommandEvents;
//    class ResetDeviceRequest;
//    class ResetDeviceResult;
//    class CalibrateCashUnitCommandEvents;
//    class CalibrateCashUnitRequest;
//    class CalibrateCashUnitResult;
//    class GetTellerInfoRequest;
//    class GetTellerInfoResult;
//    class SetTellerInfoRequest;
//    class SetTellerInfoResult;
//    class GetItemInfoRequest;
//    class GetItemInfoResult;
//}

namespace XFS4IoTFramework::CashManagement
{
    //using namespace XFS4IoTFramework::Common;

    /// <summary>
    /// Cash Management Device Interface
    /// </summary>
    class ICashManagementDevice : public virtual XFS4IoTServer::IDevice
    {
    public:
        ~ICashManagementDevice() override = default;

        /// <summary>
        /// CashManagement Status
        /// </summary>
        virtual std::shared_ptr<XFS4IoTFramework::Common::CashManagementStatusClass> GetCashManagementStatus() const = 0;
        virtual void SetCashManagementStatus(std::shared_ptr<XFS4IoTFramework::Common::CashManagementStatusClass> status) = 0;

        /// <summary>
        /// CashManagement Capabilities
        /// </summary>
        virtual std::shared_ptr<XFS4IoTFramework::Common::CashManagementCapabilitiesClass> GetCashManagementCapabilities() const = 0;
        virtual void SetCashManagementCapabilities(std::shared_ptr<XFS4IoTFramework::Common::CashManagementCapabilitiesClass> capabilities) = 0;

        /// <summary>
        /// This method will retract items which may have been in customer access from an
        /// output position or from internal areas within the CashDispenser. Retracted items
        /// will be moved to either a retract cash unit, a reject cash unit, item cash units,
        /// the transport or the intermediate stacker. After the items are retracted the
        /// shutter is closed automatically, even if the ShutterControl capability is set
        /// to false.
        /// </summary>
        virtual boost::asio::awaitable<RetractResult> RetractAsync(
            std::shared_ptr<RetractCommandEvents> events,
            const RetractRequest& request,
            std::stop_token cancellation) = 0;

        /// <summary>
        /// OpenCloseShutterAsync - Perform shutter operation to open or close.
        /// </summary>
        virtual boost::asio::awaitable<OpenCloseShutterResult> OpenCloseShutterAsync(
            const OpenCloseShutterRequest& request,
            std::stop_token cancellation) = 0;

        /// <summary>
        /// ResetDeviceAsync - Perform a hardware reset which will attempt to return the CashDispenser
        /// device to a known good state.
        /// </summary>
        virtual boost::asio::awaitable<ResetDeviceResult> ResetDeviceAsync(
            std::shared_ptr<ResetCommandEvents> events,
            const ResetDeviceRequest& request,
            std::stop_token cancellation) = 0;

        /// <summary>
        /// This method will cause a vendor dependent sequence of hardware events which will
        /// calibrate one or more physical cash units associated with a logical cash unit.
        /// </summary>
        virtual boost::asio::awaitable<CalibrateCashUnitResult> CalibrateCashUnitAsync(
            std::shared_ptr<CalibrateCashUnitCommandEvents> events,
            const CalibrateCashUnitRequest& request,
            std::stop_token cancellation) = 0;

        /// <summary>
        /// This command only applies to Teller devices. It allows the application to obtain
        /// counts for each currency assigned to the teller. These counts represent the total
        /// amount of currency dispensed by the teller in all transactions. This command
        /// also enables the application to obtain the position assigned to each teller.
        /// The teller information is persistent.
        /// </summary>
        virtual boost::asio::awaitable<GetTellerInfoResult> GetTellerInfoAsync(
            const GetTellerInfoRequest& request,
            std::stop_token cancellation) = 0;

        /// <summary>
        /// This command allows the application to initialize counts for each currency assigned
        /// to the teller. The values set by this command are persistent. This command only
        /// applies to Teller ATMs.
        /// </summary>
        virtual boost::asio::awaitable<SetTellerInfoResult> SetTellerInfoAsync(
            const SetTellerInfoRequest& request,
            std::stop_token cancellation) = 0;

        /// <summary>
        /// This command is used to get information about detected items. It can be used
        /// to get information about individual items, all items of a certain classification,
        /// or all items that have information available. This information is available from
        /// the point where the first CashManagement.InfoAvailableEvent is generated until
        /// a transaction or replenishment command is executed including the following:
        /// - CashAcceptor.CashInStart
        /// - CashAcceptor.CashIn
        /// - CashAcceptor.CashInEnd
        /// - CashAcceptor.CashInRollback
        /// - CashAcceptor.CreateSignature
        /// - CashAcceptor.Replenish
        /// - CashAcceptor.CashUnitCount
        /// - CashAcceptor.Deplete
        /// - CashManagement.Retract
        /// - CashManagement.Reset
        /// - CashManagement.OpenShutter
        /// - CashManagement.CloseShutter
        /// - CashManagement.CalibrateCashUnit
        /// - CashDispenser.Dispense
        /// - CashDispenser.Present
        /// - CashDispenser.Reject
        /// - CashDispenser.Count
        /// - CashDispenser.TestCashUnits
        /// - Storage.StartExchange
        /// - Storage.EndExchange
        /// 
        /// In addition, since the item information is not cumulative and can be replaced by any command
        /// that can move notes, it is recommended that applications that are interested
        /// in the available information should query for it following the CashManagement.InfoAvailableEvent
        /// but before any other command is executed.
        /// </summary>
        virtual GetItemInfoResult GetItemInfoInfo(const GetItemInfoRequest& request) = 0;
    };

    /// <summary>
    /// Base implementation of ICashManagementDevice providing common functionality
    /// </summary>
    class CashManagementDeviceBase : public ICashManagementDevice
    {
    public:
        CashManagementDeviceBase() = default;
        ~CashManagementDeviceBase() override = default;

        // IDevice implementation
        boost::asio::awaitable<void> RunAsync(std::stop_token token) override
        {
            // Default implementation
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

        // ICashManagementDevice implementation
        std::shared_ptr<XFS4IoTFramework::Common::CashManagementStatusClass> GetCashManagementStatus() const override
        {
            std::lock_guard<std::mutex> lock(statusMutex_);
            return cashManagementStatus_;
        }

        void SetCashManagementStatus(std::shared_ptr<XFS4IoTFramework::Common::CashManagementStatusClass> status) override
        {
            std::lock_guard<std::mutex> lock(statusMutex_);
            cashManagementStatus_ = std::move(status);
        }

        std::shared_ptr<XFS4IoTFramework::Common::CashManagementCapabilitiesClass> GetCashManagementCapabilities() const override
        {
            std::lock_guard<std::mutex> lock(capabilitiesMutex_);
            return cashManagementCapabilities_;
        }

        void SetCashManagementCapabilities(std::shared_ptr<XFS4IoTFramework::Common::CashManagementCapabilitiesClass> capabilities) override
        {
            std::lock_guard<std::mutex> lock(capabilitiesMutex_);
            cashManagementCapabilities_ = std::move(capabilities);
        }

    protected:
        std::shared_ptr<XFS4IoTServer::IServiceProvider> serviceProvider_;

        mutable std::mutex statusMutex_;
        std::shared_ptr<XFS4IoTFramework::Common::CashManagementStatusClass> cashManagementStatus_;

        mutable std::mutex capabilitiesMutex_;
        std::shared_ptr<XFS4IoTFramework::Common::CashManagementCapabilitiesClass> cashManagementCapabilities_;
    };
}