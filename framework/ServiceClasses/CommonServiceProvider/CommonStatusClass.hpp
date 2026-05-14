#pragma once

#include <functional>
#include <string>
#include "StatusBase.hpp"

namespace XFS4IoTFramework::Common
{
    /// <summary>
    /// Common status class for device state management
    /// </summary>
    class CommonStatusClass : public StatusBase
    {
    public:
        /// <summary>
        /// Position status enumeration
        /// </summary>
        enum class PositionStatusEnum
        {
            InPosition,
            NotInPosition,
            Unknown
        };

        /// <summary>
        /// Exchange status enumeration
        /// </summary>
        enum class ExchangeEnum
        {
            NotSupported,
            Active,
            Inactive
        };

        /// <summary>
        /// Device status enumeration
        /// </summary>
        enum class DeviceEnum
        {
            Online,
            Offline,
            PowerOff,
            NoDevice,
            HardwareError,
            UserError,
            DeviceBusy,
            FraudAttempt,
            PotentialFraud
        };

        /// <summary>
        /// Anti-fraud module status enumeration
        /// </summary>
        enum class AntiFraudModuleEnum
        {
            NotSupported,
            Ok,
            Inoperable,
            DeviceDetected,
            Unknown
        };

        /// <summary>
        /// End-to-end security status enumeration
        /// </summary>
        enum class EndToEndSecurityEnum
        {
            NotSupported,
            NotEnforced,
            NotConfigured,
            Enforced
        };

        /// <summary>
        /// Error event ID enumeration
        /// </summary>
        enum class ErrorEventIdEnum
        {
            Hardware,
            Software,
            User,
            FraudAttempt
        };

        /// <summary>
        /// Error action enumeration
        /// </summary>
        enum class ErrorActionEnum
        {
            Reset,
            SoftwareError,
            Configuration,
            Clear,
            Maintenance,
            Suspend
        };

        /// <summary>
        /// Constructor
        /// </summary>
        CommonStatusClass(DeviceEnum device,
            PositionStatusEnum devicePosition,
            int powerSaveRecoveryTime,
            AntiFraudModuleEnum antiFraudModule,
            ExchangeEnum exchange,
            EndToEndSecurityEnum endToEndSecurity)
            : device_(device)
            , devicePosition_(devicePosition)
            , powerSaveRecoveryTime_(powerSaveRecoveryTime)
            , antiFraudModule_(antiFraudModule)
            , exchange_(exchange)
            , endToEndSecurity_(endToEndSecurity)
        {
        }

        /// <summary>
        /// Specifies the state of the device. Following values are possible:
        /// 
        /// * Online - The device is online. This is returned when the device is present and operational.
        /// * Offline - The device is offline (e.g. the operator has taken the device offline by turning a switch).
        /// * PowerOff - The device is powered off or physically not connected.
        /// * NoDevice - The device is not intended to be there, e.g. this type of self service machine does not contain such a device or it is internally not configured.
        /// * HardwareError - The device is inoperable due to a hardware error.
        /// * UserError - The device is present but a person is preventing proper device operation.
        /// * DeviceBusy - The device is busy and unable to process a command at this time.
        /// * FraudAttempt - The device is present but is inoperable because it has detected a fraud attempt.
        /// * PotentialFraud - The device has detected a potential fraud attempt and is capable of remaining in service.
        /// </summary>
        DeviceEnum GetDevice() const { return device_; }
        void SetDevice(DeviceEnum value)
        {
            if (device_ != value) {
                device_ = value;
                NotifyPropertyChanged("Device");
            }
        }

        /// <summary>
        /// Position of the device. Following values are possible:
        /// 
        /// * InPosition - The device is in its normal operating position, or is fixed in place and cannot be moved.
        /// * NotInPosition - The device has been removed from its normal operating position.
        /// * Unknown - Due to a hardware error or other condition, the position of the device cannot be determined.
        /// </summary>
        PositionStatusEnum GetDevicePosition() const { return devicePosition_; }
        void SetDevicePosition(PositionStatusEnum value)
        {
            if (devicePosition_ != value) {
                devicePosition_ = value;
                NotifyPropertyChanged("DevicePosition");
            }
        }

        /// <summary>
        /// Specifies the actual number of seconds required by the device to resume its normal operational state from
        /// the current power saving mode. This value is zero if either the power saving mode has not been activated or
        /// no power save control is supported.
        /// </summary>
        /// <remarks>
        /// OBSOLETE: This method is no longer used by the common interface. 
        /// Migrate power saving control to PowerManagement interface.
        /// This interface will be removed after version 4.
        /// </remarks>
        [[deprecated("Migrate to PowerManagement interface")]]
        int GetPowerSaveRecoveryTime() const { return powerSaveRecoveryTime_; }

        [[deprecated("Migrate to PowerManagement interface")]]
        void SetPowerSaveRecoveryTime(int value)
        {
            if (powerSaveRecoveryTime_ != value) {
                powerSaveRecoveryTime_ = value;
                NotifyPropertyChanged("PowerSaveRecoveryTime");
            }
        }

        /// <summary>
        /// Specifies the state of the anti-fraud module. Following values are possible:
        /// 
        /// * NotSupported - No anti-fraud module is available.
        /// * Ok - Anti-fraud module is in a good state and no foreign device is detected.
        /// * Inoperable - Anti-fraud module is inoperable.
        /// * DeviceDetected - Anti-fraud module detected the presence of a foreign device.
        /// * Unknown - The state of the anti-fraud module cannot be determined.
        /// </summary>
        AntiFraudModuleEnum GetAntiFraudModule() const { return antiFraudModule_; }
        void SetAntiFraudModule(AntiFraudModuleEnum value)
        {
            if (antiFraudModule_ != value) {
                antiFraudModule_ = value;
                NotifyPropertyChanged("AntiFraudModule");
            }
        }

        /// <summary>
        /// Exchange status for storage
        /// </summary>
        ExchangeEnum GetExchange() const { return exchange_; }
        void SetExchange(ExchangeEnum value)
        {
            if (exchange_ != value) {
                exchange_ = value;
                NotifyPropertyChanged("Exchange");
            }
        }

        /// <summary>
        /// Specifies the status of end to end security support on this device.
        /// 
        /// * NotSupported - E2E security is not supported by this hardware. Any command can be called without a token.
        /// * NotEnforced - E2E security is supported by this hardware but it is not currently enforced, 
        ///   for example because required keys aren't loaded. It's currently possible to perform E2E commands without a token.
        /// * NotConfigured - E2E security is supported but not correctly configured, for example because required
        ///   keys aren't loaded. Any attempt to perform any command protected by E2E security will fail.
        /// * Enforced - E2E security is supported and correctly configured. E2E security will be enforced.
        ///   Calling E2E protected commands will only be possible if a valid token is given.
        /// </summary>
        EndToEndSecurityEnum GetEndToEndSecurity() const { return endToEndSecurity_; }
        void SetEndToEndSecurity(EndToEndSecurityEnum value)
        {
            if (endToEndSecurity_ != value) {
                endToEndSecurity_ = value;
                NotifyPropertyChanged("EndToEndSecurity");
            }
        }

    private:
        DeviceEnum device_;
        PositionStatusEnum devicePosition_;
        int powerSaveRecoveryTime_;
        AntiFraudModuleEnum antiFraudModule_;
        ExchangeEnum exchange_;
        EndToEndSecurityEnum endToEndSecurity_;
    };

    // Helper functions for enum to string conversion
    inline std::string ToString(CommonStatusClass::DeviceEnum value)
    {
        switch (value) {
        case CommonStatusClass::DeviceEnum::Online: return "online";
        case CommonStatusClass::DeviceEnum::Offline: return "offline";
        case CommonStatusClass::DeviceEnum::PowerOff: return "powerOff";
        case CommonStatusClass::DeviceEnum::NoDevice: return "noDevice";
        case CommonStatusClass::DeviceEnum::HardwareError: return "hardwareError";
        case CommonStatusClass::DeviceEnum::UserError: return "userError";
        case CommonStatusClass::DeviceEnum::DeviceBusy: return "deviceBusy";
        case CommonStatusClass::DeviceEnum::FraudAttempt: return "fraudAttempt";
        case CommonStatusClass::DeviceEnum::PotentialFraud: return "potentialFraud";
        default: return "unknown";
        }
    }

    inline std::string ToString(CommonStatusClass::PositionStatusEnum value)
    {
        switch (value) {
        case CommonStatusClass::PositionStatusEnum::InPosition: return "inPosition";
        case CommonStatusClass::PositionStatusEnum::NotInPosition: return "notInPosition";
        case CommonStatusClass::PositionStatusEnum::Unknown: return "unknown";
        default: return "unknown";
        }
    }

    inline std::string ToString(CommonStatusClass::AntiFraudModuleEnum value)
    {
        switch (value) {
        case CommonStatusClass::AntiFraudModuleEnum::NotSupported: return "notSupported";
        case CommonStatusClass::AntiFraudModuleEnum::Ok: return "ok";
        case CommonStatusClass::AntiFraudModuleEnum::Inoperable: return "inoperable";
        case CommonStatusClass::AntiFraudModuleEnum::DeviceDetected: return "deviceDetected";
        case CommonStatusClass::AntiFraudModuleEnum::Unknown: return "unknown";
        default: return "unknown";
        }
    }

    inline std::string ToString(CommonStatusClass::EndToEndSecurityEnum value)
    {
        switch (value) {
        case CommonStatusClass::EndToEndSecurityEnum::NotSupported: return "notSupported";
        case CommonStatusClass::EndToEndSecurityEnum::NotEnforced: return "notEnforced";
        case CommonStatusClass::EndToEndSecurityEnum::NotConfigured: return "notConfigured";
        case CommonStatusClass::EndToEndSecurityEnum::Enforced: return "enforced";
        default: return "unknown";
        }
    }
}

