#pragma once

#include <memory>
#include <optional>
#include <string>
#include <stdexcept>

#include <nlohmann/json.hpp>

#include "../../Completion.hpp"
#include "../../MessageBase.hpp"

namespace XFS4IoT::CashAcceptor::Completions
{
    class CashInStartCompletionPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        enum class ErrorCodeEnum
        {
            InvalidTellerId,
            UnsupportedPosition,
            ExchangeActive,
            CashInActive,
            SafeDoorOpen
        };

        explicit CashInStartCompletionPayloadData(
            std::optional<ErrorCodeEnum> errorCode = std::nullopt)
            : errorCode_(errorCode)
        {
        }

        const std::optional<ErrorCodeEnum>& GetErrorCode() const noexcept
        {
            return errorCode_;
        }

        void SetErrorCode(std::optional<ErrorCodeEnum> errorCode)
        {
            errorCode_ = errorCode;
        }

    private:
        std::optional<ErrorCodeEnum> errorCode_;
    };

    inline std::string toString(CashInStartCompletionPayloadData::ErrorCodeEnum value)
    {
        switch (value)
        {
        case CashInStartCompletionPayloadData::ErrorCodeEnum::InvalidTellerId:
            return "invalidTellerId";
        case CashInStartCompletionPayloadData::ErrorCodeEnum::UnsupportedPosition:
            return "unsupportedPosition";
        case CashInStartCompletionPayloadData::ErrorCodeEnum::ExchangeActive:
            return "exchangeActive";
        case CashInStartCompletionPayloadData::ErrorCodeEnum::CashInActive:
            return "cashInActive";
        case CashInStartCompletionPayloadData::ErrorCodeEnum::SafeDoorOpen:
            return "safeDoorOpen";
        }

        throw std::invalid_argument(
            "Unknown CashInStartCompletionPayloadData::ErrorCodeEnum value");
    }

    inline void to_json(
        nlohmann::json& j,
        const CashInStartCompletionPayloadData::ErrorCodeEnum& value)
    {
        j = toString(value);
    }

    inline void to_json(
        nlohmann::json& j,
        const CashInStartCompletionPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetErrorCode().has_value())
        {
            j["errorCode"] = p.GetErrorCode().value();
        }
    }

    class CashInStartCompletion final
        : public XFS4IoT::Completion<CashInStartCompletionPayloadData>
    {
    public:
        static constexpr const char* CompletionName = "CashAcceptor.CashInStart";
        static constexpr const char* Version = "1.0";

        CashInStartCompletion(
            int requestId,
            std::shared_ptr<CashInStartCompletionPayloadData> payload,
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            const std::string& errorDescription)
            : XFS4IoT::Completion<CashInStartCompletionPayloadData>(
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

    inline bool CashInStartCompletion::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(CashInStartCompletion),
                CashInStartCompletion::CompletionName,
                CashInStartCompletion::Version);
            return true;
        }();
}