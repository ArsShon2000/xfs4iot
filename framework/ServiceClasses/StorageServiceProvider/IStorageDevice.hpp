#pragma once

#include <memory>
#include <map>
#include <string>
#include <stop_token>
#include <boost/asio/awaitable.hpp>
#include "../../server/IDevice.hpp"
#include "../../ServiceClasses/StorageServiceProvider/CashUnit.hpp"
#include "DeviceParameters.hpp"

// Forward declarations
namespace XFS4IoTFramework::Storage
{
	//using namespace XFS4IoTServer;

    namespace XFS4IoT::CashManagement 
    {
        //class StorageCashCountClass;
        class CashUnitStorage;
        class UnitStorageBase;
    }

    class CashUnitStorageConfiguration;
    class CashUnitCountClass;
    class CashStatusClass;
    class SetCashStorageRequest;
    class SetCashStorageResult;
    //class StartExchangeResult;
    class EndExchangeResult;

    //class CashUnitStorage
    //{
    //public:
    //    enum class StatusEnum
    //    {
    //        Ok,
    //        Inoperative,
    //        Missing,
    //        NotConfigured,
    //        Manipulated
    //    };
    //};
}

namespace XFS4IoTFramework::Storage
{
    /// <summary>
    /// Storage Device Interface
    /// </summary>
    class IStorageDevice : public virtual XFS4IoTServer::IDevice
    {
    public:
        ~IStorageDevice() override = default;

        /// <summary>
        /// Return cash storage information for current configuration and capabilities on the startup.
        /// </summary>
        /// <param name="newCashUnits">Output: Dictionary of storage ID to configuration</param>
        /// <returns>Return true if the cash unit configuration or capabilities are changed, otherwise false</returns>
        virtual bool GetCashStorageConfiguration(
            std::map<std::string, CashUnitStorageConfiguration>& newCashUnits) = 0;

        /// <summary>
        /// Return cash unit counts maintained by the device class
        /// </summary>
        /// <param name="unitCounts">Output: Dictionary of storage ID to counts</param>
        /// <returns>Return true if the device class maintained counts, otherwise false</returns>
        virtual bool GetCashUnitCounts(
            std::map<std::string, CashUnitCountClass>& unitCounts) = 0;

        /// <summary>
        /// Return cash unit initial counts maintained by the device class and only this method is called on the start of day
        /// </summary>
        /// <param name="initialCounts">Output: Dictionary of storage ID to initial counts</param>
        /// <returns>Return true if the device class maintained initial counts, otherwise false</returns>
        virtual bool GetCashUnitInitialCounts(
            std::map<std::string, XFS4IoTFramework::Storage::StorageCashCountClass>& initialCounts) = 0;

        /// <summary>
        /// Return cash storage status.
        /// </summary>
        /// <param name="storageStatus">Output: Dictionary of storage ID to status</param>
        /// <returns>Return true if the device class uses hardware status, otherwise false</returns>
        virtual bool GetCashStorageStatus(
            std::map<std::string, CashUnitStorage::StatusEnum>& storageStatus) = 0;

        /// <summary>
        /// Return cash unit status maintained by the device class
        /// </summary>
        /// <param name="unitStatus">Output: Dictionary of storage ID to replenishment status</param>
        /// <returns>Return true if the device class uses hardware status, otherwise false</returns>
        virtual bool GetCashUnitStatus(
            std::map<std::string, CashStatusClass::ReplenishmentStatusEnum>& unitStatus) = 0;

        /// <summary>
        /// Return accuracy of counts. This method is called if the device class supports feature for count accuracy
        /// </summary>
        /// <param name="storageId">Storage unit identifier</param>
        /// <param name="unitAccuracy">Output: Accuracy of the count</param>
        virtual void GetCashUnitAccuracy(
            const std::string& storageId,
            CashStatusClass::AccuracyEnum& unitAccuracy) = 0;

        /// <summary>
        /// Set new configuration and counters for cash units
        /// </summary>
        /// <returns>Return operation is completed successfully or not and report updates storage information.</returns>
        virtual boost::asio::awaitable<SetCashStorageResult> SetCashStorageAsync(
            const SetCashStorageRequest& request,
            std::stop_token cancellation) = 0;

        /// <summary>
        /// This command puts the device in an exchange state, i.e. a state in which storage units can be emptied,
        /// replenished, removed or replaced. The command will initiate any physical processes which may be necessary
        /// to make the storage units accessible. If this command returns a successful completion the device is in
        /// an exchange state.
        /// </summary>
        virtual boost::asio::awaitable<StartExchangeResult> StartExchangeAsync(
            std::stop_token cancellation) = 0;

        /// <summary>
        /// This command will end the exchange state. If any physical action took place as a result of the
        /// Storage.StartExchange command then this command will cause the cash units to be returned to their
        /// normal physical state. Any necessary device testing will also be initiated.
        /// </summary>
        virtual boost::asio::awaitable<EndExchangeResult> EndExchangeAsync(
            std::stop_token cancellation) = 0;
    };

    /// <summary>
    /// Base implementation of IStorageDevice providing common functionality
    /// </summary>
    class StorageDeviceBase : public IStorageDevice
    {
    public:
        StorageDeviceBase() = default;
        ~StorageDeviceBase() override = default;

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

        // Default implementations returning empty/false
        bool GetCashStorageConfiguration(
            std::map<std::string, CashUnitStorageConfiguration>& newCashUnits) override
        {
            newCashUnits = cashStorageConfiguration_;
            return false; // No changes by default
        }

        bool GetCashUnitCounts(
            std::map<std::string, CashUnitCountClass>& unitCounts) override
        {
            unitCounts = cashUnitCounts_;
            return !cashUnitCounts_.empty();
        }

        bool GetCashUnitInitialCounts(
            std::map<std::string, XFS4IoTFramework::Storage::StorageCashCountClass>& initialCounts) override
        {
            initialCounts = initialCashCounts_;
            return !initialCashCounts_.empty();
        }

        bool GetCashStorageStatus(
            std::map<std::string, CashUnitStorage::StatusEnum>& storageStatus) override
        {
            storageStatus = storageStatus_;
            return !storageStatus_.empty();
        }

        bool GetCashUnitStatus(
            std::map<std::string, CashStatusClass::ReplenishmentStatusEnum>& unitStatus) override
        {
            unitStatus = unitStatus_;
            return !unitStatus_.empty();
        }

        void GetCashUnitAccuracy(
            const std::string& storageId,
            CashStatusClass::AccuracyEnum& unitAccuracy) override
        {
            auto it = unitAccuracy_.find(storageId);
            if (it != unitAccuracy_.end()) {
                unitAccuracy = it->second;
            }
            else {
                unitAccuracy = CashStatusClass::AccuracyEnum::Unknown;
            }
        }

    protected:
        std::shared_ptr<XFS4IoTServer::IServiceProvider> serviceProvider_;

        // Storage data
        std::map<std::string, CashUnitStorageConfiguration> cashStorageConfiguration_;
        std::map<std::string, CashUnitCountClass> cashUnitCounts_;
        std::map<std::string, XFS4IoTFramework::Storage::StorageCashCountClass> initialCashCounts_;
        std::map<std::string, CashUnitStorage::StatusEnum> storageStatus_;
        std::map<std::string, CashStatusClass::ReplenishmentStatusEnum> unitStatus_;
        std::map<std::string, CashStatusClass::AccuracyEnum> unitAccuracy_;
    };
}

// Supporting class definitions
//namespace XFS4IoTFramework::Storage
//{
    ///// <summary>
    ///// Cash status class with enumerations
    ///// </summary>
    //class CashStatusClass
    //{
    //public:
    //    enum class ReplenishmentStatusEnum
    //    {
    //        Ok,
    //        Full,
    //        High,
    //        Low,
    //        Empty,
    //        Inoperative,
    //        Missing,
    //        NoValue,
    //        NoReference
    //    };

    //    enum class AccuracyEnum
    //    {
    //        Accurate,
    //        AccurateSet,
    //        Inaccurate,
    //        NotSupported,
    //        Unknown
    //    };
    //};

    ///// <summary>
    ///// Cash unit storage configuration
    ///// </summary>
    //class CashUnitStorageConfiguration
    //{
    //public:
    //    enum class TypeEnum
    //    {
    //        RejectCassette,
    //        BillCassette,
    //        CoinCylinder,
    //        CoinDispenser,
    //        RetractCassette,
    //        Coupon,
    //        Document,
    //        Replenishment,
    //        Recycling,
    //        CashIn
    //    };

    //    CashUnitStorageConfiguration() = default;

    //    // Getters and setters
    //    TypeEnum GetType() const { return type_; }
    //    void SetType(TypeEnum type) { type_ = type; }

    //    const std::string& GetCurrency() const { return currency_; }
    //    void SetCurrency(std::string currency) { currency_ = std::move(currency); }

    //    double GetValue() const { return value_; }
    //    void SetValue(double value) { value_ = value; }

    //    int GetMaximum() const { return maximum_; }
    //    void SetMaximum(int maximum) { maximum_ = maximum; }

    //private:
    //    TypeEnum type_ = TypeEnum::BillCassette;
    //    std::string currency_;
    //    double value_ = 0.0;
    //    int maximum_ = 0;
    //};

    ///// <summary>
    ///// Storage cash count class (for initial counts)
    ///// </summary>
    //class StorageCashCountClass
    //{
    //public:
    //    StorageCashCountClass() = default;

    //    int GetInitialCount() const { return initialCount_; }
    //    void SetInitialCount(int count) { initialCount_ = count; }

    //    double GetInitialAmount() const { return initialAmount_; }
    //    void SetInitialAmount(double amount) { initialAmount_ = amount; }

    //private:
    //    int initialCount_ = 0;
    //    double initialAmount_ = 0.0;
    //};

    ///// <summary>
    ///// Set cash storage request
    ///// </summary>
    //class SetCashStorageRequest
    //{
    //public:
    //    SetCashStorageRequest() = default;

    //    const std::map<std::string, CashUnitCountClass>& GetCounts() const { return counts_; }
    //    void SetCounts(std::map<std::string, CashUnitCountClass> counts)
    //    {
    //        counts_ = std::move(counts);
    //    }

    //private:
    //    std::map<std::string, CashUnitCountClass> counts_;
    //};

    ///// <summary>
    ///// Set cash storage result
    ///// </summary>
    //class SetCashStorageResult
    //{
    //public:
    //  /*  enum class CompletionCodeEnum
    //    {
    //        Success,
    //        InvalidData,
    //        CashUnitError,
    //        UnsupportedCommand
    //    };*/

    //    SetCashStorageResult() = default;

    //    XFS4IoT::MessageHeader::CompletionCodeEnum GetCompletionCode() const { return completionCode_; }
    //    void SetCompletionCode(XFS4IoT::MessageHeader::CompletionCodeEnum code) { completionCode_ = code; }

    //private:
    //    XFS4IoT::MessageHeader::CompletionCodeEnum completionCode_ = XFS4IoT::MessageHeader::CompletionCodeEnum::Success;
    //};

	 ////<summary>
	 ////Start exchange result
	 ////</summary>
    //class StartExchangeResult
    //{
    //public:
    //    //enum class CompletionCodeEnum
    //    //{
    //    //    Success,
    //    //    ExchangeActive,
    //    //    UnsupportedCommand
    //    //};

    //    StartExchangeResult() = default;

    //    XFS4IoT::MessageHeader::CompletionCodeEnum GetCompletionCode() const { return completionCode_; }
    //    void SetCompletionCode(XFS4IoT::MessageHeader::CompletionCodeEnum code) { completionCode_ = code; }

    //private:
    //    XFS4IoT::MessageHeader::CompletionCodeEnum completionCode_ = XFS4IoT::MessageHeader::CompletionCodeEnum::Success;
    //};

    ///// <summary>
    ///// End exchange result
    ///// </summary>
    //class EndExchangeResult
    //{
    //public:
    //    //enum class CompletionCodeEnum
    //    //{
    //    //    Success,
    //    //    NoExchangeActive,
    //    //    UnsupportedCommand
    //    //};

    //    EndExchangeResult() = default;

    //    XFS4IoT::MessageHeader::CompletionCodeEnum GetCompletionCode() const { return completionCode_; }
    //    void SetCompletionCode(XFS4IoT::MessageHeader::CompletionCodeEnum code) { completionCode_ = code; }

    //private:
    //    XFS4IoT::MessageHeader::CompletionCodeEnum completionCode_ = XFS4IoT::MessageHeader::CompletionCodeEnum::Success;
    //};
//}