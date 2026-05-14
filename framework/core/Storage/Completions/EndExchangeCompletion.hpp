#pragma once

#include <string>
#include <memory>
#include <optional>
#include "../../Completion.hpp"
#include "../../common/Completions/MessagePayload.hpp"
#include "../../common/MessageHeader.hpp"

namespace XFS4IoT::Storage::Completions
{
    /// <summary>
    /// EndExchange completion message
    /// XFS4IoT Version: 2.0
    /// </summary>
        class EndExchangeCompletionPayloadData : public XFS4IoT::MessagePayloadBase
        {
        public:
            enum class ErrorCodeEnum
            {
                StorageUnitError,
                NoExchangeActive
            };

            /// <summary>
            /// Constructor with optional error code
            /// </summary>
            explicit EndExchangeCompletionPayloadData(std::optional<ErrorCodeEnum> errorCode = std::nullopt)
                : errorCode_(errorCode)
            {
            }

            /// <summary>
            /// Specifies the error code if applicable, otherwise null. Following values are possible:
            /// 
            /// * storageUnitError - A storage unit problem occurred that meant no storage units could be
            ///   updated. One or more Storage.StorageErrorEvent events will be sent with the details.
            /// * noExchangeActive - There is no exchange active.
            /// </summary>
            std::optional<ErrorCodeEnum> GetErrorCode() const { return errorCode_; }
            void setErrorCode(std::optional<ErrorCodeEnum> errorCode) { errorCode_ = errorCode; }


        private:
            std::optional<ErrorCodeEnum> errorCode_;
        };

    class EndExchangeCompletion : public XFS4IoT::Completion<EndExchangeCompletionPayloadData>
    {
    public:
        static constexpr const char* CompletionName = "Storage.EndExchange";
        static constexpr const char* Version = "1.0";

        /// <summary>
        /// Constructor for EndExchange completion
        /// </summary>
        EndExchangeCompletion(
            int requestId,
            std::shared_ptr<EndExchangeCompletionPayloadData> payload,
            MessageHeader::CompletionCodeEnum completionCode,
            const std::string& errorDescription)
            : Completion<EndExchangeCompletionPayloadData>(
                CompletionName,
                Version,
                requestId,
                std::move(payload),
                completionCode,
                errorDescription)
        {
        }

    private:
        static bool registered_;
    };

    inline bool EndExchangeCompletion::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(EndExchangeCompletion),
                EndExchangeCompletion::CompletionName,
                EndExchangeCompletion::Version);
            return true;
        }();

    // Helper function to convert ErrorCodeEnum to string for serialization
    inline std::string ToString(EndExchangeCompletionPayloadData::ErrorCodeEnum errorCode)
    {
        switch (errorCode)
        {
        case EndExchangeCompletionPayloadData::ErrorCodeEnum::StorageUnitError:
            return "storageUnitError";
        case EndExchangeCompletionPayloadData::ErrorCodeEnum::NoExchangeActive:
            return "noExchangeActive";
        default:
            throw std::invalid_argument("Unknown ErrorCodeEnum value");
        }
    }

    // Helper function to parse string to ErrorCodeEnum
    inline EndExchangeCompletionPayloadData::ErrorCodeEnum parseErrorCodeEnum(const std::string& str)
    {
        if (str == "storageUnitError")
            return EndExchangeCompletionPayloadData::ErrorCodeEnum::StorageUnitError;
        if (str == "noExchangeActive")
            return EndExchangeCompletionPayloadData::ErrorCodeEnum::NoExchangeActive;

        throw std::invalid_argument("Unknown ErrorCodeEnum string: " + str);
    }

    inline void to_json(
        nlohmann::json& j,
        const EndExchangeCompletionPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetErrorCode().has_value())
            j["errorCode"] = ToString(p.GetErrorCode().value());
    }
}