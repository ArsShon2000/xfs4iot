
#pragma once

#include <string>
#include <optional>
#include "../core/common/MessageHeader.hpp"

namespace XFS4IoTServer
{
    /// <summary>
    /// Common base class for the result of an operation processed by the device specific class
    /// </summary>
    class DeviceResult
    {
    public:
        /// <summary>
        /// Constructor with completion code
        /// </summary>
        explicit DeviceResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::optional<std::string> errorDescription = std::nullopt)
            : completionCode(completionCode)
            , errorDescription(std::move(errorDescription))
        {
        }

        /// <summary>
        /// Default constructor - Success by default
        /// </summary>
        DeviceResult()
            : completionCode(XFS4IoT::MessageHeader::CompletionCodeEnum::Success)
            , errorDescription(std::nullopt)
        {
        }

        /// <summary>
        /// Completion code of the operation
        /// </summary>
        XFS4IoT::MessageHeader::CompletionCodeEnum completionCode;

        /// <summary>
        /// Optional error description
        /// </summary>
        std::optional<std::string> errorDescription;

        /// <summary>
        /// Check if operation was successful
        /// </summary>
        bool IsSuccess() const
        {
            return completionCode == XFS4IoT::MessageHeader::CompletionCodeEnum::Success;
        }

        /// <summary>
        /// Check if operation failed
        /// </summary>
        bool IsError() const
        {
            return !IsSuccess();
        }

        /// <summary>
        /// Get error description or empty string
        /// </summary>
        std::string GetErrorDescription() const
        {
            return errorDescription.value_or("");
        }
    };
}
