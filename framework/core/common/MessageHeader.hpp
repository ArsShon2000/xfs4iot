#pragma once

#include <string>
#include <optional>
#include <stdexcept>

namespace XFS4IoT
{
    class MessageHeader final
    {
    public:
        /// Possible type of message
        enum class TypeEnum
        {
            Command,
            Acknowledge,
            Event,
            Completion,
            Unsolicited
        };

        enum class StatusEnum
        {
            InvalidMessage,
            InvalidRequestID,
            TooManyRequests
        };

        enum class CompletionCodeEnum
        {
            Success,
            CommandErrorCode,
            Canceled,
            DeviceNotReady,
            HardwareError,
            InternalError,
            InvalidCommand,
            InvalidRequestID,
            TimeOut,
            UnsupportedCommand,
            InvalidData,
            UserError,
            UnsupportedData,
            FraudAttempt,
            SequenceError,
            AuthorisationRequired,
            NoCommandNonce,
            InvalidToken,
            InvalidTokenNonce,
            InvalidTokenHMAC,
            InvalidTokenFormat,
            InvalidTokenKeyNoValue,
            NotEnoughSpace
        };

        inline std::string toString(CompletionCodeEnum code)
        {
            switch (code)
            {
            case CompletionCodeEnum::Success: return "Success";
            case CompletionCodeEnum::CommandErrorCode: return "CommandErrorCode";
            case CompletionCodeEnum::Canceled: return "Canceled";
            case CompletionCodeEnum::DeviceNotReady: return "DeviceNotReady";
            case CompletionCodeEnum::HardwareError: return "HardwareError";
            case CompletionCodeEnum::InternalError: return "InternalError";
            case CompletionCodeEnum::InvalidCommand: return "InvalidCommand";
            case CompletionCodeEnum::InvalidRequestID: return "InvalidRequestID";
            case CompletionCodeEnum::TimeOut: return "TimeOut";
            case CompletionCodeEnum::UnsupportedCommand: return "UnsupportedCommand";
            case CompletionCodeEnum::InvalidData: return "InvalidData";
            case CompletionCodeEnum::UserError: return "UserError";
            case CompletionCodeEnum::UnsupportedData: return "UnsupportedData";
            case CompletionCodeEnum::FraudAttempt: return "FraudAttempt";
            case CompletionCodeEnum::SequenceError: return "SequenceError";
            case CompletionCodeEnum::AuthorisationRequired: return "AuthorisationRequired";
            case CompletionCodeEnum::NoCommandNonce: return "NoCommandNonce";
            case CompletionCodeEnum::InvalidToken: return "InvalidToken";
            case CompletionCodeEnum::InvalidTokenNonce: return "InvalidTokenNonce";
            case CompletionCodeEnum::InvalidTokenHMAC: return "InvalidTokenHMAC";
            case CompletionCodeEnum::InvalidTokenFormat: return "InvalidTokenFormat";
            case CompletionCodeEnum::InvalidTokenKeyNoValue: return "InvalidTokenKeyNoValue";
            case CompletionCodeEnum::NotEnoughSpace: return "NotEnoughSpace";
            default: return "Unknown";
            }
        }

        /// Constructor
        MessageHeader(
            std::string name,
            std::optional<int> requestId,
            std::string version,
            TypeEnum type,
            std::optional<int> timeout = std::nullopt,
            std::optional<StatusEnum> status = std::nullopt,
            std::optional<CompletionCodeEnum> completionCode = std::nullopt,
            std::optional<std::string> errorDescription = std::nullopt)
            : name_(std::move(name)),
            requestId_(requestId),
            type_(type),
            version_(std::move(version)),
            timeout_(timeout),
            status_(status),
            completionCode_(completionCode),
            errorDescription_(std::move(errorDescription))
        {
            if (name_.empty())
                throw std::invalid_argument("Null or empty value for Name in the header.");
        }

        /// Конструктор по умолчанию
        MessageHeader()
            : name_()
            , requestId_(std::nullopt)
            , type_(TypeEnum::Command)
            , version_()
            , timeout_(std::nullopt)
            , status_(std::nullopt)
            , completionCode_(std::nullopt)
            , errorDescription_(std::nullopt)
        {
            // name_ пустой, но не выбрасываем исключение
        }

        // ---- getters (immutable after construction) ----

        const std::string& Name() const noexcept { return name_; }
        const std::optional<int>& RequestId() const noexcept { return requestId_; }
        TypeEnum Type() const noexcept { return type_; }
        const std::string& Version() const noexcept { return version_; }
        const std::optional<int>& Timeout() const noexcept { return timeout_; }
        const std::optional<StatusEnum>& Status() const noexcept { return status_; }
        const std::optional<CompletionCodeEnum>& CompletionCode() const noexcept { return completionCode_; }
        const std::optional<std::string>& ErrorDescription() const noexcept { return errorDescription_; }

    private:
        std::string name_;
        std::optional<int> requestId_;
        TypeEnum type_;
        std::string version_;
        std::optional<int> timeout_;
        std::optional<StatusEnum> status_;
        std::optional<CompletionCodeEnum> completionCode_;
        std::optional<std::string> errorDescription_;
    };

    /// <summary>
    /// Helper function to convert CompletionCodeEnum to string
    /// </summary>
    inline std::string ToString(MessageHeader::CompletionCodeEnum code)
    {
        switch (code) {
        case MessageHeader::CompletionCodeEnum::Success:
            return "success";
        case MessageHeader::CompletionCodeEnum::InternalError:
            return "internalError";
        case MessageHeader::CompletionCodeEnum::UnsupportedCommand:
            return "unsupportedCommand";
        case MessageHeader::CompletionCodeEnum::HardwareError:
            return "hardwareError";
        case MessageHeader::CompletionCodeEnum::UserError:
            return "userError";
        case MessageHeader::CompletionCodeEnum::FraudAttempt:
            return "fraudAttempt";
        case MessageHeader::CompletionCodeEnum::SequenceError:
            return "sequenceError";
        case MessageHeader::CompletionCodeEnum::InvalidToken:
            return "invalidToken";
        case MessageHeader::CompletionCodeEnum::InvalidData:
            return "invalidData";
        case MessageHeader::CompletionCodeEnum::NotEnoughSpace:
            return "notEnoughSpace";
        case MessageHeader::CompletionCodeEnum::Canceled:
            return "canceled";
        case MessageHeader::CompletionCodeEnum::DeviceNotReady:
            return "deviceNotReady";
        case MessageHeader::CompletionCodeEnum::TimeOut:
            return "timeout";
        case MessageHeader::CompletionCodeEnum::CommandErrorCode:
            return "commandErrorCode";
        default:
            return "unknown";
        }
    }
}
