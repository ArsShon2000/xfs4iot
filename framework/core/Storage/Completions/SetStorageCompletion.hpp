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
    /// SetStorage completion message
    /// XFS4IoT Version: 2.0
    /// </summary>
        class SetStorageCompletionPayloadData final : public XFS4IoT::MessagePayloadBase
        {
        public:
            enum class ErrorCodeEnum
            {
                InvalidUnit,
                NoExchangeActive,
                StorageUnitError
            };

            /// <summary>
            /// Constructor with optional error code
            /// </summary>
            explicit SetStorageCompletionPayloadData(std::optional<ErrorCodeEnum> errorCode = std::nullopt)
                : errorCode_(errorCode)
            {
            }

            /// <summary>
            /// Specifies the error code if applicable, otherwise null. Following values are possible:
            /// 
            /// * invalidUnit - Invalid unit.
            /// * noExchangeActive - The device is not in an exchange state and a request has been made to
            ///   modify information which can only be modified in an exchange state.
            /// * storageUnitError - A problem occurred with a storage unit. A
            ///   Storage.StorageErrorEvent will be posted with the details.
            /// </summary>
            std::optional<ErrorCodeEnum> GetErrorCode() const { return errorCode_; }
            void setErrorCode(std::optional<ErrorCodeEnum> errorCode) { errorCode_ = errorCode; }


        private:
            std::optional<ErrorCodeEnum> errorCode_;
        };

    class SetStorageCompletion final : public XFS4IoT::Completion<SetStorageCompletionPayloadData>
    {
    public:
        static constexpr const char* CompletionName = "Storage.SetStorage";
        static constexpr const char* Version = "1.0";

        /// <summary>
        /// Constructor for SetStorage completion
        /// </summary>
        SetStorageCompletion(
            int requestId,
            std::shared_ptr<SetStorageCompletionPayloadData> payload,
            MessageHeader::CompletionCodeEnum completionCode,
            const std::string& errorDescription)
            : Completion<SetStorageCompletionPayloadData>(
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

    inline bool SetStorageCompletion::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(SetStorageCompletion),
                SetStorageCompletion::CompletionName,
                SetStorageCompletion::Version);
            return true;
        }();

    // Helper function to convert ErrorCodeEnum to string for serialization
    inline std::string ToString(SetStorageCompletionPayloadData::ErrorCodeEnum errorCode)
    {
        switch (errorCode)
        {
        case SetStorageCompletionPayloadData::ErrorCodeEnum::InvalidUnit:
            return "invalidUnit";
        case SetStorageCompletionPayloadData::ErrorCodeEnum::NoExchangeActive:
            return "noExchangeActive";
        case SetStorageCompletionPayloadData::ErrorCodeEnum::StorageUnitError:
            return "storageUnitError";
        default:
            throw std::invalid_argument("Unknown ErrorCodeEnum value");
        }
    }

    inline void to_json(
        nlohmann::json& j,
        const SetStorageCompletionPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetErrorCode().has_value())
            j["errorCode"] = ToString(p.GetErrorCode().value());
    }

    //// Helper function to parse string to ErrorCodeEnum
    //inline SetStorageCompletionPayloadData::ErrorCodeEnum parseErrorCodeEnum(const std::string& str)
    //{
    //    if (str == "invalidUnit")
    //        return SetStorageCompletionPayloadData::ErrorCodeEnum::InvalidUnit;
    //    if (str == "noExchangeActive")
    //        return SetStorageCompletionPayloadData::ErrorCodeEnum::NoExchangeActive;
    //    if (str == "storageUnitError")
    //        return SetStorageCompletionPayloadData::ErrorCodeEnum::StorageUnitError;

    //    throw std::invalid_argument("Unknown ErrorCodeEnum string: " + str);
    //}
}