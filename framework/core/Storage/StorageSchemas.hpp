#pragma once

#include <string>
#include <optional>
#include <memory>
#include "../CashManagement/CashManagementSchemas.hpp"
#include <nlohmann/json.hpp>

namespace XFS4IoT::Storage
{
    /// <summary>
    /// Status enumeration for storage units
    /// </summary>
    enum class StatusEnum
    {
        Ok,
        Inoperative,
        Missing,
        NotConfigured,
        Manipulated
    };

    /// <summary>
    /// StorageUnitClass - Represents a storage unit with its properties
    /// </summary>
    class StorageUnitClass final
    {
    public:
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="id">An identifier which can be used for cUnitID in CDM/CIM XFS 3.x migration</param>
        /// <param name="position_name">Fixed physical name for the position</param>
        /// <param name="capacity">The nominal capacity of the unit</param>
        /// <param name="status">Current status of the storage unit</param>
        /// <param name="serial_number">The storage unit's serial number if it can be read electronically</param>
        /// <param name="cash">Cash-specific storage information</param>
        explicit StorageUnitClass(
            std::optional<std::string> id = std::nullopt,
            std::optional<std::string> position_name = std::nullopt,
            std::optional<int> capacity = std::nullopt,
            std::optional<StatusEnum> status = std::nullopt,
            std::optional<std::string> serial_number = std::nullopt,
            std::shared_ptr<CashManagement::StorageCashClass> cash = nullptr)
            : id_(id)
            , position_name_(position_name)
            , capacity_(capacity)
            , status_(status)
            , serial_number_(serial_number)
            , cash_(cash)
        {
            // Validate id pattern if provided: ^.{1,5}$
            if (id_.has_value() && !id_.value().empty())
            {
                if (id_.value().length() > 5)
                {
                    throw std::invalid_argument("Id must be between 1 and 5 characters");
                }
            }

            // Validate capacity minimum if provided
            if (capacity_.has_value() && capacity_.value() < 0)
            {
                throw std::invalid_argument("Capacity must be >= 0");
            }
        }

        /// <summary>
        /// An identifier which can be used for cUnitID in CDM/CIM XFS 3.x migration.
        /// Pattern: ^.{1,5}$
        /// Example: "RC1"
        /// </summary>
        [[nodiscard]] const std::optional<std::string>& id() const noexcept { return id_; }

        /// <summary>
        /// Fixed physical name for the position.
        /// Example: "Top Right"
        /// </summary>
        [[nodiscard]] const std::optional<std::string>& position_name() const noexcept { return position_name_; }

        /// <summary>
        /// The nominal capacity of the unit. This may be an estimate as the quality and thickness of the items
        /// stored in the unit may affect how many items can be stored. 
        /// 0 means the capacity is unknown; null means capacity is not applicable.
        /// Example: 100
        /// </summary>
        [[nodiscard]] const std::optional<int>& capacity() const noexcept { return capacity_; }

        /// <summary>
        /// Current status of the storage unit
        /// </summary>
        [[nodiscard]] const std::optional<StatusEnum>& status() const noexcept { return status_; }

        /// <summary>
        /// The storage unit's serial number if it can be read electronically.
        /// Example: "ABCD1234"
        /// </summary>
        [[nodiscard]] const std::optional<std::string>& serial_number() const noexcept { return serial_number_; }

        /// <summary>
        /// Cash-specific storage information
        /// </summary>
        [[nodiscard]] const std::shared_ptr<CashManagement::StorageCashClass>& cash() const noexcept { return cash_; }

        // Setters (if mutable access is needed)
        void set_id(std::optional<std::string> id)
        {
            if (id.has_value() && !id.value().empty() && id.value().length() > 5)
            {
                throw std::invalid_argument("Id must be between 1 and 5 characters");
            }
            id_ = std::move(id);
        }

        void set_position_name(std::optional<std::string> position_name) { position_name_ = std::move(position_name); }

        void set_capacity(std::optional<int> capacity)
        {
            if (capacity.has_value() && capacity.value() < 0)
            {
                throw std::invalid_argument("Capacity must be >= 0");
            }
            capacity_ = capacity;
        }

        void set_status(std::optional<StatusEnum> status) { status_ = status; }
        void set_serial_number(std::optional<std::string> serial_number) { serial_number_ = std::move(serial_number); }
        void set_cash(std::shared_ptr<CashManagement::StorageCashClass> cash) { cash_ = cash; }

        virtual ~StorageUnitClass() = default;


    private:
        std::optional<std::string> id_;
        std::optional<std::string> position_name_;
        std::optional<int> capacity_;
        std::optional<StatusEnum> status_;
        std::optional<std::string> serial_number_;
        std::shared_ptr<CashManagement::StorageCashClass> cash_;
    };

    inline void to_json(nlohmann::json& j, const StorageUnitClass& obj)
    {
        j = nlohmann::json::object();

        if (obj.id().has_value())
            j["id"] = obj.id().value();

        if (obj.position_name().has_value())
            j["positionName"] = obj.position_name().value();

        if (obj.capacity().has_value())
            j["capacity"] = obj.capacity().value();

        if (obj.status().has_value())
            j["status"] = obj.status().value();

        if (obj.serial_number().has_value())
            j["serialNumber"] = obj.serial_number().value();

        if (obj.cash())
            j["cash"] = *obj.cash();
    }

    //*****************************************************************************

    /// <summary>
    /// SetStorageUnitClass - Used for setting/updating storage unit information
    /// </summary>
    class SetStorageUnitClass final
    {
    public:
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="cash">Cash-specific storage information to set</param>
        explicit SetStorageUnitClass(
            std::shared_ptr<CashManagement::StorageSetCashClass> cash = nullptr)
            : cash_(cash)
        {
        }

        /// <summary>
        /// Cash-specific storage information to set
        /// </summary>
        [[nodiscard]] const std::shared_ptr<CashManagement::StorageSetCashClass>& cash() const noexcept { return cash_; }

        void set_cash(std::shared_ptr<CashManagement::StorageSetCashClass> cash) { cash_ = cash; }

        virtual ~SetStorageUnitClass() = default;

        //// Copy and move semantics
        //SetStorageUnitClass(const SetStorageUnitClass&) = default;
        //SetStorageUnitClass& operator=(const SetStorageUnitClass&) = default;
        //SetStorageUnitClass(SetStorageUnitClass&&) noexcept = default;
        //SetStorageUnitClass& operator=(SetStorageUnitClass&&) noexcept = default;

    private:
        std::shared_ptr<CashManagement::StorageSetCashClass> cash_;
    };

    // Helper functions for enum conversion
    [[nodiscard]] inline std::string ToString(StatusEnum status)
    {
        switch (status)
        {
        case StatusEnum::Ok:             return "ok";
        case StatusEnum::Inoperative:    return "inoperative";
        case StatusEnum::Missing:        return "missing";
        case StatusEnum::NotConfigured:  return "notConfigured";
        case StatusEnum::Manipulated:    return "manipulated";
        }
        throw std::logic_error("Unknown StatusEnum");
    }

    inline void to_json(nlohmann::json& j, const StatusEnum& value)
    {
        j = ToString(value);
    }

    [[nodiscard]] inline StatusEnum StatusEnumFromString(const std::string& str)
    {
        if (str == "ok") return StatusEnum::Ok;
        if (str == "inoperative") return StatusEnum::Inoperative;
        if (str == "missing") return StatusEnum::Missing;
        if (str == "notConfigured") return StatusEnum::NotConfigured;
        if (str == "manipulated") return StatusEnum::Manipulated;
        throw std::invalid_argument("Unknown StatusEnum string: " + str);
    }


    // JSON serialization support for StatusEnum
    NLOHMANN_JSON_SERIALIZE_ENUM(StatusEnum, {
        {StatusEnum::Ok, "ok"},
        {StatusEnum::Inoperative, "inoperative"},
        {StatusEnum::Missing, "missing"},
        {StatusEnum::NotConfigured, "notConfigured"},
        {StatusEnum::Manipulated, "manipulated"}
        })

    //    // JSON serialization for StorageUnitClass
    //    inline void to_json(nlohmann::json& j, const StorageUnitClass& unit)
    //{
    //    if (unit.id().has_value())
    //        j["id"] = unit.id().value();

    //    if (unit.position_name().has_value())
    //        j["positionName"] = unit.position_name().value();

    //    if (unit.capacity().has_value())
    //        j["capacity"] = unit.capacity().value();

    //    if (unit.status().has_value())
    //        j["status"] = unit.status().value();

    //    if (unit.serial_number().has_value())
    //        j["serialNumber"] = unit.serial_number().value();

    //    if (unit.cash())
    //        j["cash"] = *unit.cash();
    //}

    //inline void from_json(const nlohmann::json& j, StorageUnitClass& unit)
    //{
    //    if (j.contains("id"))
    //        unit.set_id(j["id"].get<std::string>());

    //    if (j.contains("positionName"))
    //        unit.set_position_name(j["positionName"].get<std::string>());

    //    if (j.contains("capacity"))
    //        unit.set_capacity(j["capacity"].get<int>());

    //    if (j.contains("status"))
    //        unit.set_status(j["status"].get<StatusEnum>());

    //    if (j.contains("serialNumber"))
    //        unit.set_serial_number(j["serialNumber"].get<std::string>());

    //    if (j.contains("cash"))
    //    {
    //        auto cash = std::make_shared<CashManagement::StorageCashClass>(
    //            j["cash"].get<CashManagement::StorageCashClass>());
    //        unit.set_cash(cash);
    //    }
    //}

    //// JSON serialization for SetStorageUnitClass
    //inline void to_json(nlohmann::json& j, const SetStorageUnitClass& unit)
    //{
    //    if (unit.cash())
    //        j["cash"] = *unit.cash();
    //}

    //inline void from_json(const nlohmann::json& j, SetStorageUnitClass& unit)
    //{
    //    if (j.contains("cash"))
    //    {
    //        auto cash = std::make_shared<CashManagement::StorageSetCashClass>(
    //            j["cash"].get<CashManagement::StorageSetCashClass>());
    //        unit.set_cash(cash);
    //    }
    //}
} // namespace XFS4IoT::Storage