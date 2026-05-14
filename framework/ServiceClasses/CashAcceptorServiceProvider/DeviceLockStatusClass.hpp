#pragma once

#include <string>
#include <unordered_map>
#include <optional>

namespace XFS4IoTFramework::CashAcceptor
{
    class DeviceLockStatusClass final
    {
    public:
        /// <summary>
        /// Specifies the physical lock/unlock status of the CashAcceptor device. The following values are possible:
        /// 
        /// * Lock - The device is physically locked.
        /// * Unlock - The device is physically unlocked.
        /// * LockUnknown - Due to a hardware error or other condition, the physical lock/unlock status of the 
        /// device cannot be determined.
        /// * LockNotSupported - The Service does not support reporting the physical lock/unlock status of the 
        /// device.
        /// </summary>
        enum class DeviceLockStatusEnum
        {
            Lock,
            Unlock,
            LockUnknown,
            LockNotSupported
        };

        /// <summary>
        /// Specifies the physical lock/unlock status of storage units supported. The following values are possible:
        /// 
        /// * Lock - The storage unit is physically locked.
        /// * Unlock - The storage unit is physically unlocked.
        /// * LockUnknown - Due to a hardware error or other condition, the physical lock/unlock status of the 
        /// storage unit cannot be determined.
        /// </summary>
        enum class UnitLockStatusEnum
        {
            Lock,
            Unlock,
            LockUnknown
        };

        // Конструктор с параметрами
        DeviceLockStatusClass(DeviceLockStatusEnum deviceLockStatus,
            std::optional<std::unordered_map<std::string, UnitLockStatusEnum>> unitLock)
            : lockStatus_(deviceLockStatus)
            , unitLock_(std::move(unitLock))
        {
        }

        // Конструктор по умолчанию
        DeviceLockStatusClass()
            : lockStatus_(DeviceLockStatusEnum::LockNotSupported)
            , unitLock_(std::nullopt)
        {
        }

        // Геттер и сеттер для LockStatus
        DeviceLockStatusEnum getLockStatus() const { return lockStatus_; }
        void setLockStatus(DeviceLockStatusEnum status) { lockStatus_ = status; }

        /// <summary>
        /// Specifying the physical lock/unlock status of storage units. Units that do not support the physical
        /// lock/unlock control are not contained in the dictionary. If there are no units that support physical
        /// lock/unlock control this will be empty.
        /// </summary>
        const std::optional<std::unordered_map<std::string, UnitLockStatusEnum>>& getUnitLock() const
        {
            return unitLock_;
        }

        void setUnitLock(std::optional<std::unordered_map<std::string, UnitLockStatusEnum>> unitLock)
        {
            unitLock_ = std::move(unitLock);
        }

    private:
        DeviceLockStatusEnum lockStatus_;
        std::optional<std::unordered_map<std::string, UnitLockStatusEnum>> unitLock_;
    };
}