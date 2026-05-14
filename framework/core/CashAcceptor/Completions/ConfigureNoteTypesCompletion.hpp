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
    class ConfigureNoteTypesCompletionPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        enum class ErrorCodeEnum
        {
            ExchangeActive,
            CashInActive
        };

        explicit ConfigureNoteTypesCompletionPayloadData(
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

    inline std::string toString(
        ConfigureNoteTypesCompletionPayloadData::ErrorCodeEnum value)
    {
        switch (value)
        {
        case ConfigureNoteTypesCompletionPayloadData::ErrorCodeEnum::ExchangeActive:
            return "exchangeActive";
        case ConfigureNoteTypesCompletionPayloadData::ErrorCodeEnum::CashInActive:
            return "cashInActive";
        }

        throw std::invalid_argument(
            "Unknown ConfigureNoteTypesCompletionPayloadData::ErrorCodeEnum value");
    }

    inline void to_json(
        nlohmann::json& j,
        const ConfigureNoteTypesCompletionPayloadData::ErrorCodeEnum& value)
    {
        j = toString(value);
    }

    inline void to_json(
        nlohmann::json& j,
        const ConfigureNoteTypesCompletionPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetErrorCode().has_value())
        {
            j["errorCode"] = p.GetErrorCode().value();
        }
    }

    class ConfigureNoteTypesCompletion final
        : public XFS4IoT::Completion<ConfigureNoteTypesCompletionPayloadData>
    {
    public:
        static constexpr const char* CompletionName = "CashAcceptor.ConfigureNoteTypes";
        static constexpr const char* Version = "1.0";

        ConfigureNoteTypesCompletion(
            int requestId,
            std::shared_ptr<ConfigureNoteTypesCompletionPayloadData> payload,
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            const std::string& errorDescription)
            : XFS4IoT::Completion<ConfigureNoteTypesCompletionPayloadData>(
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

    inline bool ConfigureNoteTypesCompletion::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(ConfigureNoteTypesCompletion),
                ConfigureNoteTypesCompletion::CompletionName,
                ConfigureNoteTypesCompletion::Version);
            return true;
        }();
}