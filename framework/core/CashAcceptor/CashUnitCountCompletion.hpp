#pragma once

#include <string>
#include <optional>
#include <memory>
#include "../common/Completions/MessagePayload.hpp"
#include "../common/MessageHeader.hpp"
#include "../MessageBase.hpp"
#include "../Completion.hpp"

namespace XFS4IoT::CashAcceptor::Completions
{
        /// <summary>
        /// Payload data for CashUnitCount completion
        /// </summary>
        class CashUnitCountPayloadData : public XFS4IoT::MessagePayloadBase
        {
        public:
            enum class ErrorCodeEnum
            {
                InvalidCashUnit,
                CashInActive,
                ExchangeActive,
                TooManyItemsToCount,
                CountPositionNotEmpty,
                CashUnitError
            };

            CashUnitCountPayloadData(std::optional<ErrorCodeEnum> errorCode = std::nullopt)
                : errorCode_(errorCode)
            {
            }

            /// <summary>
            /// Specifies the error code if applicable, otherwise null. The following values are possible:
            /// 
            /// * InvalidCashUnit - At least one of the storage units specified is either invalid or does not
            ///   support being counted. No storage units have been counted.
            /// * CashInActive - A cash-in transaction is active.
            /// * ExchangeActive - The device is in the exchange state.
            /// * TooManyItemsToCount - There were too many items. The required internal position may have been
            ///   of insufficient size. All items should be returned to the storage unit from which they originated.
            /// * CountPositionNotEmpty - A required internal position is not empty so a storage unit count is
            ///   not possible.
            /// * CashUnitError - A storage unit caused a problem. A Storage.StorageErrorEvent will be posted with the details.
            /// </summary>
            std::optional<ErrorCodeEnum> GetErrorCode() const { return errorCode_; }
            void SetErrorCode(std::optional<ErrorCodeEnum> errorCode) { errorCode_ = errorCode; }

        private:
            std::optional<ErrorCodeEnum> errorCode_;
        };


    /// <summary>
    /// CashUnitCount completion
    /// Version: 2.0
    /// Name: CashAcceptor.CashUnitCount
    /// </summary>
    class CashUnitCountCompletion : public Completion<CashUnitCountPayloadData>
    {
    public:
        static constexpr const char* CompletionName = "CashAcceptor.CashUnitCount";
        static constexpr const char* Version = "1.0";
        CashUnitCountCompletion(
            int requestId,
            std::shared_ptr<CashUnitCountPayloadData> payload,
            MessageHeader::CompletionCodeEnum completionCode,
            const std::string& errorDescription)
            : Completion<CashUnitCountPayloadData>(
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

    inline bool CashUnitCountCompletion::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(CashUnitCountCompletion),
                CashUnitCountCompletion::CompletionName,
                CashUnitCountCompletion::Version);
            return true;
        }();

    /// <summary>
    /// Helper function to convert ErrorCodeEnum to string
    /// </summary>
    inline std::string ToString(CashUnitCountPayloadData::ErrorCodeEnum code)
    {
        switch (code) {
        case CashUnitCountPayloadData::ErrorCodeEnum::InvalidCashUnit:
            return "invalidCashUnit";
        case CashUnitCountPayloadData::ErrorCodeEnum::CashInActive:
            return "cashInActive";
        case CashUnitCountPayloadData::ErrorCodeEnum::ExchangeActive:
            return "exchangeActive";
        case CashUnitCountPayloadData::ErrorCodeEnum::TooManyItemsToCount:
            return "tooManyItemsToCount";
        case CashUnitCountPayloadData::ErrorCodeEnum::CountPositionNotEmpty:
            return "countPositionNotEmpty";
        case CashUnitCountPayloadData::ErrorCodeEnum::CashUnitError:
            return "cashUnitError";
        default:
            return "unknown";
        }
    }

    inline void to_json(
        nlohmann::json& j,
        const CashUnitCountPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetErrorCode().has_value())
            j["errorCode"] = ToString(p.GetErrorCode().value());
    }
}