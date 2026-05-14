#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>
#include <cstdint>
#include <boost/asio/awaitable.hpp>
#include "../../core/Storage/StorageSchemas.hpp"
#include "IStorageDevice.hpp"
#include "../../core/Events/PropertyChangedEventArgs.hpp"
#include "CashUnit.hpp"
#include "../../server/ServiceProvider.hpp"

namespace XFS4IoTFramework::Storage
{
    //// Forward declarations
    //class CashUnitCountClass;
    //class StorageCheckCountClass;
    //class CardUnitStorage;
    //class CashUnitStorage;
    //class CheckUnitStorage;
    //class PrinterUnitStorage;
    //class IBNSUnitStorage;
    //class DepositUnitStorage;
    //class PropertyChangedEventArgs;

    //namespace XFS4IoT::Storage
    //{
        //class StorageUnitClass;
    //}

    enum class StorageTypeEnum : uint32_t
    {
        Cash = 1 << 0,  // 0x01
        Card = 1 << 1,  // 0x02
        Check = 1 << 2,  // 0x04
        Printer = 1 << 3,  // 0x08
        IBNS = 1 << 4,  // 0x10
        Deposit = 1 << 5   // 0x20
    };

    // Операторы для использования enum как флагов
    inline StorageTypeEnum operator|(StorageTypeEnum lhs, StorageTypeEnum rhs)
    {
        return static_cast<StorageTypeEnum>(
            static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    }

    inline StorageTypeEnum operator&(StorageTypeEnum lhs, StorageTypeEnum rhs)
    {
        return static_cast<StorageTypeEnum>(
            static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
    }

    inline StorageTypeEnum& operator|=(StorageTypeEnum& lhs, StorageTypeEnum rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    inline StorageTypeEnum& operator&=(StorageTypeEnum& lhs, StorageTypeEnum rhs)
    {
        lhs = lhs & rhs;
        return lhs;
    }

    inline bool hasFlag(StorageTypeEnum value, StorageTypeEnum flag)
    {
        return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
    }

    class IStorageService /*: public std::enable_shared_from_this<XFS4IoTServer::ServiceProvider>*/
    {
    public:
        virtual ~IStorageService() = default;

        /// <summary>
        /// Update managed card storage information in the framework.
        /// </summary>
        virtual boost::asio::awaitable<void> UpdateCardStorageCount(
            const std::string& storageId,
            int countDelta) = 0;

        /// <summary>
        /// Update managed cash storage information in the framework.
        /// </summary>
        virtual boost::asio::awaitable<void> UpdateCashAccounting(
            std::optional<std::unordered_map<std::string, std::shared_ptr<CashUnitCountClass>>> countDelta = std::nullopt) = 0;

       /* /// <summary>
        /// Update managed check storage information in the framework.
        /// </summary>
        virtual boost::asio::awaitable<void> updateCheckStorageCount(
            std::optional<std::unordered_map<std::string, std::shared_ptr<StorageCheckCountClass>>> countDelta = std::nullopt) = 0;*/

        ///// <summary>
        ///// Update managed printer storage information in the framework.
        ///// </summary>
        //virtual boost::asio::awaitable<void> updatePrinterStorageCount(
        //    const std::string& storageId,
        //    int countDelta) = 0;

        /// <summary>
        /// Update managed deposit storage information in the framework.
        /// </summary>
        virtual boost::asio::awaitable<void> UpdateDepositStorageCount(
            const std::string& storageId,
            int countDelta) = 0;

        /// <summary>
        /// Return which type of storage SP is using
        /// </summary>
        virtual StorageTypeEnum GetStorageType() const = 0;

        /// <summary>
        /// Store CardUnits and CashUnits persistently
        /// </summary>
        virtual void StorePersistent() = 0;

        /// <summary>
        /// Cash storage structure information of this device
        /// </summary>
        virtual const std::unordered_map<std::string, std::shared_ptr<CashUnitStorage>>&
            GetCashUnits() const = 0;


        /// <summary>
        /// Return XFS4IoT storage structured object.
        /// </summary>
		virtual std::unordered_map<std::string, std::shared_ptr<::XFS4IoT::Storage::StorageUnitClass>>
			GetStorages(const std::vector<std::string>& unitIds) = 0;

        /// <summary>
        /// Sending status changed event.
        /// </summary>
        virtual boost::asio::awaitable<void> StorageChangedEvent(
            std::shared_ptr<void> sender,
            std::shared_ptr<XFS4IoTServer::PropertyChangedEventArgs> propertyInfo) = 0;
    };
}