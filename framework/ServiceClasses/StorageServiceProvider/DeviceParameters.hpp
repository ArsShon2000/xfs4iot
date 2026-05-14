#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <optional>
#include "../StorageServiceProvider/CashManagementPresentStatus.hpp"
#include "../CommonServiceProvider/CashManagementCapabilitiesClass.hpp"
#include "../../core/Storage/Completions/StartExchangeCompletion.hpp"
#include "../../core/Storage/Completions/EndExchangeCompletion.hpp"
#include "../../core/Storage/Completions/SetStorageCompletion.hpp"
#include "../../core/common/MessageHeader.hpp"
#include "../../server/DeviceResult.hpp"

namespace XFS4IoTFramework::Storage
{
   

    enum class FailureEnum
    {
        Empty,
        Error,
        Full,
        Locked,
        Invalid,
        Config,
        NotConfigured
    };

    using CompletionCodeEnum = ::XFS4IoT::MessageHeader::CompletionCodeEnum;
#pragma region Exchange Results

    class StartExchangeResult final : public XFS4IoTServer::DeviceResult
    {
        using ErrorCodeEnum = ::XFS4IoT::Storage::Completions::StartExchangeCompletionPayloadData::ErrorCodeEnum;
    public:
        StartExchangeResult(
            CompletionCodeEnum completionCode,
            std::optional < std::string> errorDescription = "",
            std::optional<ErrorCodeEnum> errorCode = std::nullopt)
            : DeviceResult(completionCode, errorDescription)
            , errorCode_(errorCode)
        {
        }

        std::optional<ErrorCodeEnum>
            getErrorCode() const { return errorCode_; }

    private:
        std::optional<ErrorCodeEnum> errorCode_;
    };

    class EndExchangeResult final : public XFS4IoTServer::DeviceResult
    {
        using ErrorCodeEnum = ::XFS4IoT::Storage::Completions::EndExchangeCompletionPayloadData::ErrorCodeEnum;
    public:
        EndExchangeResult(
           CompletionCodeEnum completionCode,
            const std::string& errorDescription = "",
            std::optional<ErrorCodeEnum> errorCode = std::nullopt)
            : DeviceResult(completionCode, errorDescription)
            , errorCode_(errorCode)
        {
        }

        std::optional<ErrorCodeEnum>
            getErrorCode() const { return errorCode_; }

    private:
        std::optional<ErrorCodeEnum> errorCode_;
    };

#pragma endregion

#pragma region Card Storage Configuration

    class SetCardConfiguration final
    {
    public:
        SetCardConfiguration(
            std::optional<int> threshold,
            const std::string& cardId = "")
            : cardId_(cardId)
            , threshold_(threshold)
        {
        }

        const std::string& getCardId() const { return cardId_; }
        void setCardId(const std::string& cardId) { cardId_ = cardId; }

        std::optional<int> getThreshold() const { return threshold_; }
        void setThreshold(std::optional<int> threshold) { threshold_ = threshold; }

    private:
        std::string cardId_;
        std::optional<int> threshold_;
    };

    class SetCardUnitStorage final
    {
    public:
        SetCardUnitStorage(
            std::shared_ptr<SetCardConfiguration> configuration,
            std::optional<int> initialCount)
            : configuration_(configuration)
            , initialCount_(initialCount)
        {
        }

        /// <summary>
        /// If this property is null, no need to change card unit configuration
        /// </summary>
        std::shared_ptr<SetCardConfiguration> getConfiguration() const { return configuration_; }

        /// <summary>
        /// Set to InitialCount and Count, reset RetainCount to zero
        /// If this property is not set, no need to update in the device class
        /// </summary>
        std::optional<int> getInitialCount() const { return initialCount_; }

    private:
        std::shared_ptr<SetCardConfiguration> configuration_;
        std::optional<int> initialCount_;
    };

    class SetCardStorageRequest final
    {
    public:
        explicit SetCardStorageRequest(
            std::unordered_map<std::string, std::shared_ptr<SetCardUnitStorage>> cardStorageToSet)
            : cardStorageToSet_(std::move(cardStorageToSet))
        {
        }

        const std::unordered_map<std::string, std::shared_ptr<SetCardUnitStorage>>&
            getCardStorageToSet() const { return cardStorageToSet_; }

    private:
        std::unordered_map<std::string, std::shared_ptr<SetCardUnitStorage>> cardStorageToSet_;
    };

    class SetCardStorageResult final : public XFS4IoTServer::DeviceResult
    {
        using ErrorCodeEnum = ::XFS4IoT::Storage::Completions::StartExchangeCompletionPayloadData::ErrorCodeEnum;
    public:
        SetCardStorageResult(
           CompletionCodeEnum completionCode,
            const std::string& errorDescription = "",
            std::optional<ErrorCodeEnum> errorCode = std::nullopt)
            : DeviceResult(completionCode, errorDescription)
            , errorCode_(errorCode)
            , newCardStorage_()
        {
        }

        SetCardStorageResult(
           CompletionCodeEnum completionCode,
            std::unordered_map<std::string, std::shared_ptr<SetCardUnitStorage>> newCardStorage)
            : DeviceResult(completionCode, "")
            , errorCode_(std::nullopt)
            , newCardStorage_(std::move(newCardStorage))
        {
        }

        std::optional<ErrorCodeEnum>
            getErrorCode() const { return errorCode_; }

        const std::unordered_map<std::string, std::shared_ptr<SetCardUnitStorage>>&
            getNewCardStorage() const { return newCardStorage_; }

    private:
        std::optional<ErrorCodeEnum> errorCode_;
        std::unordered_map<std::string, std::shared_ptr<SetCardUnitStorage>> newCardStorage_;
    };

#pragma endregion

#pragma region Cash Storage Configuration

    /// <summary>
    /// Configuration of the cash unit
    /// </summary>
    class SetCashConfiguration final
    {
    public:
        SetCashConfiguration(
            std::optional<CashCapabilitiesClass::TypesEnum> types,
            std::optional<CashCapabilitiesClass::ItemsEnum> items,
            const std::string& currency,
            std::optional<double> value,
            std::optional<int> highThreshold,
            std::optional<int> lowThreshold,
            std::optional<bool> appLockIn,
            std::optional<bool> appLockOut,
            std::vector<std::string> banknoteItems,
            const std::string& name,
            std::optional<int> maxRetracts)
            : types_(types)
            , items_(items)
            , currency_(currency)
            , value_(value)
            , highThreshold_(highThreshold)
            , lowThreshold_(lowThreshold)
            , appLockIn_(appLockIn)
            , appLockOut_(appLockOut)
            , banknoteItems_(std::move(banknoteItems))
            , name_(name)
            , maxRetracts_(maxRetracts)
        {
        }

        /// <summary>
        /// The types of operation the unit is capable of configured to perform.
        /// </summary>
        std::optional<CashCapabilitiesClass::TypesEnum> getTypes() const { return types_; }
        void setTypes(std::optional<CashCapabilitiesClass::TypesEnum> types) { types_ = types; }

        /// <summary>
        /// The types of cash media the unit is configured to store.
        /// </summary>
        std::optional<CashCapabilitiesClass::ItemsEnum> getItems() const { return items_; }
        void setItems(std::optional<CashCapabilitiesClass::ItemsEnum> items) { items_ = items; }

        /// <summary>
        /// ISO 4217 currency.
        /// </summary>
        const std::string& getCurrency() const { return currency_; }
        void setCurrency(const std::string& currency) { currency_ = currency; }

        /// <summary>
        /// Absolute value of all contents, 0 if mixed.
        /// </summary>
        std::optional<double> getValue() const { return value_; }
        void setValue(std::optional<double> value) { value_ = value; }

        /// <summary>
        /// If specified, ReplenishmentStatus is set to High if count is greater than this number.
        /// </summary>
        std::optional<int> getHighThreshold() const { return highThreshold_; }
        void setHighThreshold(std::optional<int> threshold) { highThreshold_ = threshold; }

        /// <summary>
        /// If specified, ReplenishmentStatus is set to Low if count is lower than this number.
        /// </summary>
        std::optional<int> getLowThreshold() const { return lowThreshold_; }
        void setLowThreshold(std::optional<int> threshold) { lowThreshold_ = threshold; }

        /// <summary>
        /// If true, items cannot be accepted into the storage unit in Cash In operations.
        /// </summary>
        std::optional<bool> getAppLockIn() const { return appLockIn_; }
        void setAppLockIn(std::optional<bool> lockIn) { appLockIn_ = lockIn; }

        /// <summary>
        /// If true, items cannot be dispensed from the storage unit in Cash Out operations.
        /// </summary>
        std::optional<bool> getAppLockOut() const { return appLockOut_; }
        void setAppLockOut(std::optional<bool> lockOut) { appLockOut_ = lockOut; }

        /// <summary>
        /// Lists the cash items which are configured to this unit.
        /// </summary>
        const std::vector<std::string>& getBanknoteItems() const { return banknoteItems_; }
        void setBanknoteItems(std::vector<std::string> items) { banknoteItems_ = std::move(items); }

        /// <summary>
        /// Application configured name of the unit.
        /// </summary>
        const std::string& getName() const { return name_; }
        void setName(const std::string& name) { name_ = name; }

        /// <summary>
        /// If specified, this is the number of retract operations allowed into the unit.
        /// </summary>
        std::optional<int> getMaxRetracts() const { return maxRetracts_; }
        void setMaxRetracts(std::optional<int> maxRetracts) { maxRetracts_ = maxRetracts; }

    private:
        std::optional<CashCapabilitiesClass::TypesEnum> types_;
        std::optional<CashCapabilitiesClass::ItemsEnum> items_;
        std::string currency_;
        std::optional<double> value_;
        std::optional<int> highThreshold_;
        std::optional<int> lowThreshold_;
        std::optional<bool> appLockIn_;
        std::optional<bool> appLockOut_;
        std::vector<std::string> banknoteItems_;
        std::string name_;
        std::optional<int> maxRetracts_;
    };

    class SetCashUnitStorage final
    {
    public:
        SetCashUnitStorage(
            std::shared_ptr<SetCashConfiguration> configuration,
            std::shared_ptr<StorageCashCountClass> initialCounts)
            : configuration_(configuration)
            , initialCounts_(initialCounts)
        {
        }

        /// <summary>
        /// If this property is null, no need to change cash unit configuration
        /// </summary>
        std::shared_ptr<SetCashConfiguration> getConfiguration() const { return configuration_; }

        /// <summary>
        /// Set to InitialCounts
        /// </summary>
        std::shared_ptr<StorageCashCountClass> getInitialCounts() const { return initialCounts_; }

    private:
        std::shared_ptr<SetCashConfiguration> configuration_;
        std::shared_ptr<StorageCashCountClass> initialCounts_;
    };

    class SetCashStorageRequest final
    {
    public:
        explicit SetCashStorageRequest(
            std::unordered_map<std::string, std::shared_ptr<SetCashUnitStorage>> cashStorageToSet)
            : cashStorageToSet_(std::move(cashStorageToSet))
        {
        }

        const std::unordered_map<std::string, std::shared_ptr<SetCashUnitStorage>>&
            getCashStorageToSet() const { return cashStorageToSet_; }

    private:
        std::unordered_map<std::string, std::shared_ptr<SetCashUnitStorage>> cashStorageToSet_;
    };

    class SetCashStorageResult final : public XFS4IoTServer::DeviceResult
    {
        using ErrorCodeEnum = ::XFS4IoT::Storage::Completions::StartExchangeCompletionPayloadData::ErrorCodeEnum;
    public:
        SetCashStorageResult(
           CompletionCodeEnum completionCode,
            const std::string& errorDescription = "",
            std::optional<ErrorCodeEnum> errorCode = std::nullopt)
            : DeviceResult(completionCode, errorDescription)
            , errorCode_(errorCode)
            , newCashStorage_()
        {
        }

        SetCashStorageResult(
           CompletionCodeEnum completionCode,
            std::unordered_map<std::string, std::shared_ptr<SetCashUnitStorage>> newCashStorage)
            : DeviceResult(completionCode, "")
            , errorCode_(std::nullopt)
            , newCashStorage_(std::move(newCashStorage))
        {
        }

        std::optional<ErrorCodeEnum>
            getErrorCode() const { return errorCode_; }

        const std::unordered_map<std::string, std::shared_ptr<SetCashUnitStorage>>&
            getNewCashStorage() const { return newCashStorage_; }

    private:
        std::optional<ErrorCodeEnum> errorCode_;
        std::unordered_map<std::string, std::shared_ptr<SetCashUnitStorage>> newCashStorage_;
    };

#pragma endregion

//#pragma region Check Storage Configuration
//
//    /// <summary>
//    /// Configuration of the check unit
//    /// </summary>
//    class SetCheckConfiguration final
//    {
//    public:
//        SetCheckConfiguration(
//            std::optional<CheckCapabilitiesClass::TypesEnum> types,
//            const std::string& id,
//            std::optional<int> highThreshold,
//            std::optional<int> retractHighThreshold)
//            : types_(types)
//            , id_(id)
//            , highThreshold_(highThreshold)
//            , retractHighThreshold_(retractHighThreshold)
//        {
//        }
//
//        std::optional<CheckCapabilitiesClass::TypesEnum> getTypes() const { return types_; }
//        void setTypes(std::optional<CheckCapabilitiesClass::TypesEnum> types) { types_ = types; }
//
//        const std::string& getId() const { return id_; }
//        void setId(const std::string& id) { id_ = id; }
//
//        std::optional<int> getHighThreshold() const { return highThreshold_; }
//        void setHighThreshold(std::optional<int> threshold) { highThreshold_ = threshold; }
//
//        std::optional<int> getRetractHighThreshold() const { return retractHighThreshold_; }
//        void setRetractHighThreshold(std::optional<int> threshold) { retractHighThreshold_ = threshold; }
//
//    private:
//        std::optional<CheckCapabilitiesClass::TypesEnum> types_;
//        std::string id_;
//        std::optional<int> highThreshold_;
//        std::optional<int> retractHighThreshold_;
//    };
//
//    class SetCheckUnitStorage final
//    {
//    public:
//        SetCheckUnitStorage(
//            std::shared_ptr<SetCheckConfiguration> configuration,
//            std::optional<int> mediaInCount,
//            std::optional<int> count,
//            std::optional<int> retractOperations)
//            : configuration_(configuration)
//            , mediaInCount_(mediaInCount)
//            , count_(count)
//            , retractOperations_(retractOperations)
//        {
//        }
//
//        std::shared_ptr<SetCheckConfiguration> getConfiguration() const { return configuration_; }
//        std::optional<int> getMediaInCount() const { return mediaInCount_; }
//        std::optional<int> getCount() const { return count_; }
//        std::optional<int> getRetractOperations() const { return retractOperations_; }
//
//    private:
//        std::shared_ptr<SetCheckConfiguration> configuration_;
//        std::optional<int> mediaInCount_;
//        std::optional<int> count_;
//        std::optional<int> retractOperations_;
//    };
//
//    class SetCheckStorageRequest final
//    {
//    public:
//        explicit SetCheckStorageRequest(
//            std::unordered_map<std::string, std::shared_ptr<SetCheckUnitStorage>> checkStorageToSet)
//            : checkStorageToSet_(std::move(checkStorageToSet))
//        {
//        }
//
//        const std::unordered_map<std::string, std::shared_ptr<SetCheckUnitStorage>>&
//            getCheckStorageToSet() const { return checkStorageToSet_; }
//
//    private:
//        std::unordered_map<std::string, std::shared_ptr<SetCheckUnitStorage>> checkStorageToSet_;
//    };
//
//    class SetCheckStorageResult final : public XFS4IoTServer::DeviceResult
//    {
//    public:
//        SetCheckStorageResult(
//           CompletionCodeEnum completionCode,
//            const std::string& errorDescription = "",
//            std::optional<XFS4IoT::Storage::Completions::SetStorageCompletionPayloadData::ErrorCodeEnum> errorCode = std::nullopt)
//            : DeviceResult(completionCode, errorDescription)
//            , errorCode_(errorCode)
//            , newCheckStorage_()
//        {
//        }
//
//        SetCheckStorageResult(
//           CompletionCodeEnum completionCode,
//            std::unordered_map<std::string, std::shared_ptr<SetCheckUnitStorage>> newCheckStorage)
//            : DeviceResult(completionCode, "")
//            , errorCode_(std::nullopt)
//            , newCheckStorage_(std::move(newCheckStorage))
//        {
//        }
//
//        std::optional<XFS4IoT::Storage::Completions::SetStorageCompletionPayloadData::ErrorCodeEnum>
//            getErrorCode() const { return errorCode_; }
//
//        const std::unordered_map<std::string, std::shared_ptr<SetCheckUnitStorage>>&
//            getNewCheckStorage() const { return newCheckStorage_; }
//
//    private:
//        std::optional<XFS4IoT::Storage::Completions::SetStorageCompletionPayloadData::ErrorCodeEnum> errorCode_;
//        std::unordered_map<std::string, std::shared_ptr<SetCheckUnitStorage>> newCheckStorage_;
//    };
//
//#pragma endregion

//#pragma region Printer Storage Configuration
//
//    /// <summary>
//    /// Configuration of the printer unit
//    /// </summary>
//    class SetPrinterConfiguration final
//    {
//    public:
//        SetPrinterConfiguration() = default;
//    };
//
//    class SetPrinterUnitStorage final
//    {
//    public:
//        SetPrinterUnitStorage(
//            std::shared_ptr<SetPrinterConfiguration> configuration = nullptr,
//            std::optional<int> initialCount = std::nullopt)
//            : configuration_(configuration)
//            , initialCount_(initialCount)
//        {
//        }
//
//        std::shared_ptr<SetPrinterConfiguration> getConfiguration() const { return configuration_; }
//        std::optional<int> getInitialCount() const { return initialCount_; }
//
//    private:
//        std::shared_ptr<SetPrinterConfiguration> configuration_;
//        std::optional<int> initialCount_;
//    };
//
//    class SetPrinterStorageRequest final
//    {
//    public:
//        explicit SetPrinterStorageRequest(
//            std::unordered_map<std::string, std::shared_ptr<SetPrinterUnitStorage>> printerStorageToSet)
//            : printerStorageToSet_(std::move(printerStorageToSet))
//        {
//        }
//
//        const std::unordered_map<std::string, std::shared_ptr<SetPrinterUnitStorage>>&
//            getPrinterStorageToSet() const { return printerStorageToSet_; }
//
//    private:
//        std::unordered_map<std::string, std::shared_ptr<SetPrinterUnitStorage>> printerStorageToSet_;
//    };
//
//    class SetPrinterStorageResult final : public XFS4IoTServer::DeviceResult
//    {
//    public:
//        SetPrinterStorageResult(
//           CompletionCodeEnum completionCode,
//            const std::string& errorDescription = "",
//            std::optional<XFS4IoT::Storage::Completions::SetStorageCompletionPayloadData::ErrorCodeEnum> errorCode = std::nullopt)
//            : DeviceResult(completionCode, errorDescription)
//            , errorCode_(errorCode)
//            , newPrinterStorage_()
//        {
//        }
//
//        SetPrinterStorageResult(
//           CompletionCodeEnum completionCode,
//            std::unordered_map<std::string, std::shared_ptr<SetPrinterUnitStorage>> newPrinterStorage)
//            : DeviceResult(completionCode, "")
//            , errorCode_(std::nullopt)
//            , newPrinterStorage_(std::move(newPrinterStorage))
//        {
//        }
//
//        std::optional<XFS4IoT::Storage::Completions::SetStorageCompletionPayloadData::ErrorCodeEnum>
//            getErrorCode() const { return errorCode_; }
//
//        const std::unordered_map<std::string, std::shared_ptr<SetPrinterUnitStorage>>&
//            getNewPrinterStorage() const { return newPrinterStorage_; }
//
//    private:
//        std::optional<XFS4IoT::Storage::Completions::SetStorageCompletionPayloadData::ErrorCodeEnum> errorCode_;
//        std::unordered_map<std::string, std::shared_ptr<SetPrinterUnitStorage>> newPrinterStorage_;
//    };
//
//#pragma endregion

//#pragma region Deposit Storage Configuration
//
//    /// <summary>
//    /// Configuration of the deposit unit
//    /// </summary>
//    class SetDepositConfiguration final
//    {
//    public:
//        SetDepositConfiguration() = default;
//    };
//
//    class SetDepositUnitStorage final
//    {
//    public:
//        SetDepositUnitStorage(
//            std::shared_ptr<SetDepositConfiguration> configuration = nullptr,
//            std::optional<int> initialCount = std::nullopt)
//            : configuration_(configuration)
//            , initialCount_(initialCount)
//        {
//        }
//
//        std::shared_ptr<SetDepositConfiguration> getConfiguration() const { return configuration_; }
//        std::optional<int> getInitialCount() const { return initialCount_; }
//
//    private:
//        std::shared_ptr<SetDepositConfiguration> configuration_;
//        std::optional<int> initialCount_;
//    };
//
//    class SetDepositStorageRequest final
//    {
//    public:
//        explicit SetDepositStorageRequest(
//            std::unordered_map<std::string, std::shared_ptr<SetDepositUnitStorage>> depositStorageToSet)
//            : depositStorageToSet_(std::move(depositStorageToSet))
//        {
//        }
//
//        const std::unordered_map<std::string, std::shared_ptr<SetDepositUnitStorage>>&
//            getDepositStorageToSet() const { return depositStorageToSet_; }
//
//    private:
//        std::unordered_map<std::string, std::shared_ptr<SetDepositUnitStorage>> depositStorageToSet_;
//    };
//
//    class SetDepositStorageResult final : public XFS4IoTServer::DeviceResult
//    {
//    public:
//        SetDepositStorageResult(
//           CompletionCodeEnum completionCode,
//            const std::string& errorDescription = "",
//            std::optional<XFS4IoT::Storage::Completions::SetStorageCompletionPayloadData::ErrorCodeEnum> errorCode = std::nullopt)
//            : DeviceResult(completionCode, errorDescription)
//            , errorCode_(errorCode)
//            , newDepositStorage_()
//        {
//        }
//
//        SetDepositStorageResult(
//           CompletionCodeEnum completionCode,
//            std::unordered_map<std::string, std::shared_ptr<SetDepositUnitStorage>> newDepositStorage)
//            : DeviceResult(completionCode, "")
//            , errorCode_(std::nullopt)
//            , newDepositStorage_(std::move(newDepositStorage))
//        {
//        }
//
//        std::optional<XFS4IoT::Storage::Completions::SetStorageCompletionPayloadData::ErrorCodeEnum>
//            getErrorCode() const { return errorCode_; }
//
//        const std::unordered_map<std::string, std::shared_ptr<SetDepositUnitStorage>>&
//            getNewDepositStorage() const { return newDepositStorage_; }
//
//    private:
//        std::optional<XFS4IoT::Storage::Completions::SetStorageCompletionPayloadData::ErrorCodeEnum> errorCode_;
//        std::unordered_map<std::string, std::shared_ptr<SetDepositUnitStorage>> newDepositStorage_;
//    };
//
//#pragma endregion

}