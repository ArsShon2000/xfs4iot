#pragma once

#include <memory>
#include <stop_token>
#include <mutex>
#include <boost/asio/awaitable.hpp>

#include "../../server/IDevice.hpp"
#include "CommonStatusClass.hpp"
#include "CommonCapabilitiesClass.hpp"
#include "DeviceParameters.hpp"

// Forward declarations
//namespace XFS4IoTFramework::Common
//{
//    class DeviceResult;
//    class SetTransactionStateRequest;
//    class GetTransactionStateResult;
//    class GetCommandNonceResult;
//}

namespace XFS4IoTFramework::Common
{
	using namespace XFS4IoTServer;

    /// <summary>
    /// Common Device Interface
    /// </summary>
    class ICommonDevice : public virtual XFS4IoTServer::IDevice
    {
    public:
        ~ICommonDevice() override = default;

        /// <summary>
        /// This method activates or deactivates the power-saving mode.
        /// If the Service Provider receives another execute command while in power saving mode,
        /// the Service Provider automatically exits the power saving mode, and executes the requested command.
        /// If the Service Provider receives an information command while in power saving mode,
        /// the Service Provider will not exit the power saving mode.
        /// </summary>
        /// <remarks>
        /// OBSOLETE: This method is no longer used by the common interface.
        /// Migrate power saving control to PowerManagement interface.
        /// This interface will be removed after version 4.
        /// </remarks>
        [[deprecated("Migrate to PowerManagement interface")]]
        virtual boost::asio::awaitable<DeviceResult> PowerSaveControl(
            int maxPowerSaveRecoveryTime,
            std::stop_token cancel) = 0;

        /// <summary>
        /// This method allows the application to specify the transaction state, which the Service Provider
        /// can then utilize in order to optimize performance. After receiving this command, this Service Provider
        /// can perform the necessary processing to start or end the customer transaction.
        /// This command should be called for every Service Provider that could be used in a customer transaction.
        /// The transaction state applies to every session.
        /// </summary>
        virtual boost::asio::awaitable<DeviceResult> SetTransactionState(
            const SetTransactionStateRequest& request) = 0;

        /// <summary>
        /// This method can be used to get the transaction state.
        /// </summary>
        virtual boost::asio::awaitable<GetTransactionStateResult> GetTransactionState() = 0;

        /// <summary>
        /// Get a nonce to be included in an Authorisation Token for a command that will be used to ensure
        /// end to end security. The hardware will overwrite any existing stored Command nonce with this new value.
        /// The value will be stored for future authentication. Any Authorisation Token received will be compared
        /// with this stored nonce and if the Token doesn't contain the same nonce it will be considered invalid
        /// and rejected, causing the command that contains that Authentication Token to fail.
        /// The nonce must match the algorithm used. For example, HMAC means the nonce must be 128 bit/16 bytes.
        /// </summary>
        virtual boost::asio::awaitable<GetCommandNonceResult> GetCommandNonce() = 0;

        /// <summary>
        /// Clear generated nonce
        /// </summary>
        virtual boost::asio::awaitable<DeviceResult> ClearCommandNonce() = 0;

        /// <summary>
        /// Common Status
        /// </summary>
        virtual std::shared_ptr < CommonStatusClass> GetCommonStatus() const = 0;
        virtual void SetCommonStatus(std::shared_ptr < CommonStatusClass> status) = 0;

        /// <summary>
        /// Common Capabilities
        /// </summary>
        virtual std::shared_ptr < CommonCapabilitiesClass> GetCommonCapabilities() const = 0;
        virtual void SetCommonCapabilities(std::shared_ptr < CommonCapabilitiesClass> capabilities) = 0;
    };

    /// <summary>
    /// Base implementation of ICommonDevice providing common functionality
    /// </summary>
    class CommonDeviceBase : public ICommonDevice
    {
    public:
        CommonDeviceBase() = default;
        ~CommonDeviceBase() override = default;

        // IDevice implementation
        boost::asio::awaitable<void> RunAsync(std::stop_token token) override
        {
            // Default implementation
            co_return;
        }

        void SetServiceProvider(std::shared_ptr<IServiceProvider> serviceProvider) override
        {
            serviceProvider_ = serviceProvider;
        }

        std::shared_ptr<IServiceProvider> GetServiceProvider() const override
        {
            return serviceProvider_;
        }

        // ICommonDevice property implementations
        std::shared_ptr < CommonStatusClass> GetCommonStatus() const override
        {
            std::lock_guard<std::mutex> lock(statusMutex_);
            return commonStatus_;
        }

        void SetCommonStatus(std::shared_ptr < CommonStatusClass> status) override
        {
            std::lock_guard<std::mutex> lock(statusMutex_);
            commonStatus_ = std::move(status);
        }

        std::shared_ptr < CommonCapabilitiesClass> GetCommonCapabilities() const override
        {
            std::lock_guard<std::mutex> lock(capabilitiesMutex_);
            return commonCapabilities_;
        }

        void SetCommonCapabilities(std::shared_ptr < CommonCapabilitiesClass> capabilities) override
        {
            std::lock_guard<std::mutex> lock(capabilitiesMutex_);
            commonCapabilities_ = std::move(capabilities);
        }

        // Default implementations for deprecated method
        [[deprecated("Migrate to PowerManagement interface")]]
        boost::asio::awaitable<DeviceResult> PowerSaveControl(
            int maxPowerSaveRecoveryTime,
            std::stop_token cancel) override
        {
            // Default implementation - can be overridden
            DeviceResult result;
            // Deprecated functionality
            co_return result;
        }

    protected:
        std::shared_ptr<IServiceProvider> serviceProvider_;

        mutable std::mutex statusMutex_;
        std::shared_ptr < CommonStatusClass> commonStatus_;

        mutable std::mutex capabilitiesMutex_;
        std::shared_ptr < CommonCapabilitiesClass> commonCapabilities_;
    };
}

//// Forward declaration for CommonCapabilitiesClass (if not already defined)
//namespace XFS4IoTFramework::Common
//{
//    /// <summary>
//    /// Device result class
//    /// </summary>
//    class DeviceResult
//    {
//    public:
//        //enum class CompletionCodeEnum
//        //{
//        //    Success,
//        //    InvalidData,
//        //    UnsupportedCommand,
//        //    HardwareError,
//        //    InternalError
//        //};
//
//        DeviceResult() : completionCode_(XFS4IoT::MessageHeader::CompletionCodeEnum::Success) {}
//
//        explicit DeviceResult(XFS4IoT::MessageHeader::CompletionCodeEnum code, const std::optional<std::string>& errorDescription)
//            : completionCode_(code)
//        {
//            errorDescription.value_or(errorDescription_);
//        }
//
//        XFS4IoT::MessageHeader::CompletionCodeEnum GetCompletionCode() const { return completionCode_; }
//        void SetCompletionCode(XFS4IoT::MessageHeader::CompletionCodeEnum code) { completionCode_ = code; }
//
//        const std::string& GetErrorDescription() const { return errorDescription_; }
//        void SetErrorDescription(std::string description)
//        {
//            errorDescription_ = std::move(description);
//        }
//
//    private:
//        XFS4IoT::MessageHeader::CompletionCodeEnum completionCode_;
//        std::string errorDescription_;
//    };
//
//    /// <summary>
//    /// Set transaction state request
//    /// </summary>
//    class SetTransactionStateRequest
//    {
//    public:
//        enum class StateEnum
//        {
//            Active,
//            Idle
//        };
//
//        explicit SetTransactionStateRequest(StateEnum state) : state_(state) {}
//
//        StateEnum GetState() const { return state_; }
//
//    private:
//        StateEnum state_;
//    };
//
//    /// <summary>
//    /// Get transaction state result
//    /// </summary>
//    class GetTransactionStateResult : public DeviceResult
//    {
//    public:
//        enum class StateEnum
//        {
//            Active,
//            Idle
//        };
//
//        GetTransactionStateResult() = default;
//
//        StateEnum GetState() const { return state_; }
//        void SetState(StateEnum state) { state_ = state; }
//
//    private:
//        StateEnum state_ = StateEnum::Idle;
//    };
//
//    /// <summary>
//    /// Get command nonce result
//    /// </summary>
//    class GetCommandNonceResult : public DeviceResult
//    {
//    public:
//        GetCommandNonceResult() = default;
//
//        const std::vector<uint8_t>& GetNonce() const { return nonce_; }
//        void SetNonce(std::vector<uint8_t> nonce) { nonce_ = std::move(nonce); }
//
//    private:
//        std::vector<uint8_t> nonce_;
//    };
//}