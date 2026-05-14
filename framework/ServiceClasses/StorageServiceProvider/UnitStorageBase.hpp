#pragma once

#include <string>
#include <memory>
#include "StorageChangedBase.hpp"

namespace XFS4IoTFramework::Storage
{
    /// <summary>
    /// Base class for all type of item units.
    /// </summary>
    class UnitStorageBase : public StorageChangedBaseRecord
    {
    public:
        enum class StatusEnum
        {
            Good,
            Inoperative,
            Missing,
            NotConfigured,
            Manipulated
        };

        /// <summary>
        /// Constructor
        /// </summary>
        UnitStorageBase(
            std::string positionName,
            int capacity,
            StatusEnum status,
            std::string serialNumber)
            : StorageChangedBaseRecord()
            , positionName_(std::move(positionName))
            , capacity_(capacity)
            , status_(status)
            , serialNumber_(std::move(serialNumber))
        {
        }

        virtual ~UnitStorageBase() = default;

        // Disable copy, allow move
        UnitStorageBase(const UnitStorageBase&) = delete;
        UnitStorageBase& operator=(const UnitStorageBase&) = delete;
        UnitStorageBase(UnitStorageBase&&) = default;
        UnitStorageBase& operator=(UnitStorageBase&&) = default;

        /// <summary>
        /// Fixed physical name for the position.
        /// </summary>
        const std::string& getPositionName() const { return positionName_; }

        /// <summary>
        /// Capacity of the storage unit
        /// </summary>
        int getCapacity() const { return capacity_; }

        /// <summary>
        /// Status of this storage
        /// </summary>
        StatusEnum getStatus() const { return status_; }

        void setStatus(StatusEnum value)
        {
            if (status_ != value)
            {
                status_ = value;
                notifyPropertyChanged("Status");
            }
        }

        /// <summary>
        /// The storage unit's serial number if it can be read electronically.
        /// </summary>
        const std::string& getSerialNumber() const { return serialNumber_; }

        // Comparison operators (for record-like behavior)
        virtual bool operator==(const UnitStorageBase& other) const
        {
            return positionName_ == other.positionName_ &&
                capacity_ == other.capacity_ &&
                status_ == other.status_ &&
                serialNumber_ == other.serialNumber_;
        }

        virtual bool operator!=(const UnitStorageBase& other) const
        {
            return !(*this == other);
        }

    protected:
        // For derived classes that need to create copies
        UnitStorageBase(const UnitStorageBase& other, bool /*clone_tag*/)
            : StorageChangedBaseRecord()
            , positionName_(other.positionName_)
            , capacity_(other.capacity_)
            , status_(other.status_)
            , serialNumber_(other.serialNumber_)
        {
        }

    private:
        std::string positionName_;
        int capacity_;
        StatusEnum status_;
        std::string serialNumber_;
    };
}