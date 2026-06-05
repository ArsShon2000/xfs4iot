// CashInStatusClass.hpp
#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "../StorageServiceProvider/CashUnit.hpp"

namespace XFS4IoTFramework::CashManagement
{
    class CashInStatusClass
    {
    public:
        enum class StatusEnum
        {
            Ok,
            Rollback,
            Active,
            Retract,
            Unknown,
            Reset
        };

        CashInStatusClass()
            : status_(StatusEnum::Unknown)
            , numOfRefusedItems_(0)
            , cashCounts_(std::make_shared<XFS4IoTFramework::Storage::StorageCashCountClass>())
        {
        }

        CashInStatusClass(
            StatusEnum status,
            int numOfRefusedItems,
            std::shared_ptr<XFS4IoTFramework::Storage::StorageCashCountClass> cashCounts)
            : status_(status)
            , numOfRefusedItems_(numOfRefusedItems)
            , cashCounts_(cashCounts ? cashCounts : std::make_shared<XFS4IoTFramework::Storage::StorageCashCountClass>())
        {
        }

        CashInStatusClass(const CashInStatusClass& other)
            : status_(other.status_)
            , numOfRefusedItems_(other.numOfRefusedItems_)
            , cashCounts_(other.cashCounts_
                ? std::make_shared<XFS4IoTFramework::Storage::StorageCashCountClass>(*other.cashCounts_)
                : std::make_shared<XFS4IoTFramework::Storage::StorageCashCountClass>())
        {
        }

        CashInStatusClass& operator=(const CashInStatusClass& other)
        {
            if (this != &other)
            {
                status_ = other.status_;
                numOfRefusedItems_ = other.numOfRefusedItems_;
                cashCounts_ = other.cashCounts_
                    ? std::make_shared<XFS4IoTFramework::Storage::StorageCashCountClass>(*other.cashCounts_)
                    : std::make_shared<XFS4IoTFramework::Storage::StorageCashCountClass>();
            }
            return *this;
        }

        StatusEnum GetStatus() const { return status_; }
        void SetStatus(StatusEnum status) { status_ = status; }

        int GetNumOfRefusedItems() const { return numOfRefusedItems_; }
        void SetNumOfRefusedItems(int numOfRefusedItems) { numOfRefusedItems_ = numOfRefusedItems; }

        std::shared_ptr<XFS4IoTFramework::Storage::StorageCashCountClass> GetCashCounts() const { return cashCounts_; }
        void SetCashCounts(std::shared_ptr<XFS4IoTFramework::Storage::StorageCashCountClass> cashCounts)
        {
            cashCounts_ = cashCounts
                ? cashCounts
                : std::make_shared<XFS4IoTFramework::Storage::StorageCashCountClass>();
        }

        bool operator==(const CashInStatusClass& other) const
        {
            bool cashCountsEqual = (cashCounts_ == nullptr && other.cashCounts_ == nullptr) ||
                (cashCounts_ != nullptr && other.cashCounts_ != nullptr &&
                    *cashCounts_ == *other.cashCounts_);

            return status_ == other.status_ &&
                numOfRefusedItems_ == other.numOfRefusedItems_ &&
                cashCountsEqual;
        }

        bool operator!=(const CashInStatusClass& other) const
        {
            return !(*this == other);
        }

    private:
        StatusEnum status_;
        int numOfRefusedItems_;
        std::shared_ptr<XFS4IoTFramework::Storage::StorageCashCountClass> cashCounts_;
    };

    inline std::string ToString(CashInStatusClass::StatusEnum status)
    {
        switch (status)
        {
        case CashInStatusClass::StatusEnum::Ok:       return "Ok";
        case CashInStatusClass::StatusEnum::Rollback: return "Rollback";
        case CashInStatusClass::StatusEnum::Active:   return "Active";
        case CashInStatusClass::StatusEnum::Retract:  return "Retract";
        case CashInStatusClass::StatusEnum::Unknown:  return "Unknown";
        case CashInStatusClass::StatusEnum::Reset:    return "Reset";
        default:                                      return "Unknown";
        }
    }

    inline CashInStatusClass::StatusEnum StatusEnumFromString(const std::string& str)
    {
        if (str == "Ok")       return CashInStatusClass::StatusEnum::Ok;
        if (str == "Rollback") return CashInStatusClass::StatusEnum::Rollback;
        if (str == "Active")   return CashInStatusClass::StatusEnum::Active;
        if (str == "Retract")  return CashInStatusClass::StatusEnum::Retract;
        if (str == "Unknown")  return CashInStatusClass::StatusEnum::Unknown;
        if (str == "Reset")    return CashInStatusClass::StatusEnum::Reset;
        return CashInStatusClass::StatusEnum::Unknown;
    }

    inline void to_json(nlohmann::json& j, const CashInStatusClass& value)
    {
        j = nlohmann::json{
            { "status", ToString(value.GetStatus()) },
            { "numOfRefusedItems", value.GetNumOfRefusedItems() },
            { "cashCounts", value.GetCashCounts() ? nlohmann::json(*value.GetCashCounts()) : nlohmann::json(nullptr) }
        };
    }

    inline void from_json(const nlohmann::json& j, CashInStatusClass& value)
    {
        std::string statusStr = "Unknown";
        int numOfRefusedItems = 0;

        if (j.contains("status") && !j.at("status").is_null())
        {
            j.at("status").get_to(statusStr);
        }

        if (j.contains("numOfRefusedItems") && !j.at("numOfRefusedItems").is_null())
        {
            j.at("numOfRefusedItems").get_to(numOfRefusedItems);
        }

        value.SetStatus(StatusEnumFromString(statusStr));
        value.SetNumOfRefusedItems(numOfRefusedItems);

        if (j.contains("cashCounts") && !j.at("cashCounts").is_null())
        {
            value.SetCashCounts(
                std::make_shared<XFS4IoTFramework::Storage::StorageCashCountClass>(
                    j.at("cashCounts").get<XFS4IoTFramework::Storage::StorageCashCountClass>()
                )
            );
        }
        else
        {
            value.SetCashCounts(std::make_shared<XFS4IoTFramework::Storage::StorageCashCountClass>());
        }
    }

} // namespace XFS4IoTFramework::CashManagement