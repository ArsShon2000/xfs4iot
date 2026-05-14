#pragma once

#include <string>
#include <optional>
#include <memory>
#include "../../Completion.hpp"
#include "../../common/Completions/MessagePayload.hpp"
#include "../../common/MessageHeader.hpp"

namespace XFS4IoT::Storage::Completions
{
        class StartExchangeCompletionPayloadData final : public XFS4IoT::MessagePayloadBase
        {
        public:
            enum class ErrorCodeEnum
            {
                StorageUnitError,
                ExchangeActive,
                TransactionActive
            };

            explicit StartExchangeCompletionPayloadData(std::optional<ErrorCodeEnum> errorCode = std::nullopt)
                : errorCode_(errorCode)
            {
            }

            /// <summary>
            /// Specifies the error code if applicable, otherwise null. Following values are possible:
            /// 
            /// * StorageUnitError - An error occurred with a storage unit while performing the exchange
            ///   operation. A Storage.StorageErrorEvent will be sent with the details.
            /// * ExchangeActive - The device is already in an exchange state.
            /// * TransactionActive - A transaction is active.
            /// </summary>
            std::optional<ErrorCodeEnum> GetErrorCode() const { return errorCode_; }

        private:
            std::optional<ErrorCodeEnum> errorCode_;
        };

    class StartExchangeCompletion final : public XFS4IoT::Completion<StartExchangeCompletionPayloadData>
    {
    public:
        static constexpr const char* CompletionName = "Storage.StartExchange";
        static constexpr const char* Version = "1.0";

        StartExchangeCompletion(
            int requestId,
            std::shared_ptr<StartExchangeCompletionPayloadData> payload,
            MessageHeader::CompletionCodeEnum completionCode,
            const std::string& errorDescription)
            : Completion<StartExchangeCompletionPayloadData>(
                CompletionName,
                Version,
                requestId,
                std::move(payload),
                completionCode,
                errorDescription)
        {
        }

        virtual ~StartExchangeCompletion() = default;


    private:
        static bool registered_;
    };

    inline bool StartExchangeCompletion::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(StartExchangeCompletion),
                StartExchangeCompletion::CompletionName,
                StartExchangeCompletion::Version);
            return true;
        }();

    /// <summary>
    /// Helper function to convert ErrorCodeEnum to string
    /// </summary>
    inline std::string ToString(StartExchangeCompletionPayloadData::ErrorCodeEnum code)
    {
        switch (code) {
        case StartExchangeCompletionPayloadData::ErrorCodeEnum::StorageUnitError:
            return "storageUnitError";
        case StartExchangeCompletionPayloadData::ErrorCodeEnum::ExchangeActive:
            return "exchangeActive";
        case StartExchangeCompletionPayloadData::ErrorCodeEnum::TransactionActive:
            return "transactionActive";
        default:
            throw std::invalid_argument("Unknown ErrorCodeEnum value");
        }
    }

    inline void to_json(
        nlohmann::json& j,
        const StartExchangeCompletionPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetErrorCode().has_value())
            j["errorCode"] = ToString(p.GetErrorCode().value());
    }
}