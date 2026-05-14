#pragma once

#include <string>
#include <memory>
#include <optional>
#include "../../Completion.hpp"
#include "../../common/MessageHeader.hpp"
#include "../../common/Completions/MessagePayload.hpp"
#include "../CashManagementSchemas.hpp"

namespace XFS4IoT::CashManagement::Completions
{
    /// <summary>
    /// CalibrateCashUnit completion message
    /// XFS4IoT Version: 2.0
    /// </summary>
    
    class CalibrateCashUnitCompletionPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        enum class ErrorCodeEnum
        {
            CashUnitError,
            UnsupportedPosition,
            ExchangeActive,
            InvalidCashUnit
        };

        class ResultClass final
        {
        public:
            /// <summary>
            /// Constructor
            /// </summary>
            ResultClass(
                std::optional<std::string> unit = std::nullopt,
                std::optional<int> numOfBills = std::nullopt,
                std::shared_ptr<ItemTargetDataClass> position = nullptr)
                : unit_(std::move(unit))
                , numOfBills_(numOfBills)
                , position_(position)
            {
            }

            /// <summary>
            /// The object name of the storage unit which has been calibrated as stated by
            /// Storage.GetStorage.
            /// Pattern: ^unit[0-9A-Za-z]+$
            /// </summary>
            const std::optional<std::string>& getUnit() const { return unit_; }
            void setUnit(std::optional<std::string> unit) { unit_ = std::move(unit); }

            /// <summary>
            /// Number of items that were actually dispensed during the calibration process. This value may be different
            /// from that passed in using the input structure if the device always dispenses a default number of
            /// bills. When bills are presented to an output position this is the count of notes presented to the output
            /// position, any other notes rejected during the calibration process are not included in this count as they
            /// will be accounted for within the storage unit counts.
            /// Minimum: 0
            /// </summary>
            std::optional<int> getNumOfBills() const { return numOfBills_; }
            void setNumOfBills(std::optional<int> numOfBills) { numOfBills_ = numOfBills; }

            /// <summary>
            /// Defines where items have been moved to as one of the following:
            /// 
            /// * A single storage unit, further specified by *unit*.
            /// * Internal areas of the device.
            /// * An output position.
            /// 
            /// This may be null if no items were moved.
            /// </summary>
            std::shared_ptr<ItemTargetDataClass> getPosition() const { return position_; }
            void setPosition(std::shared_ptr<ItemTargetDataClass> position) { position_ = position; }

            /// <summary>
            /// Validate that unit matches pattern: ^unit[0-9A-Za-z]+$
            /// </summary>
            bool validateUnit() const;

            /// <summary>
            /// Validate that numOfBills is >= 0
            /// </summary>
            bool validateNumOfBills() const
            {
                return !numOfBills_.has_value() || numOfBills_.value() >= 0;
            }

        private:
            std::optional<std::string> unit_;
            std::optional<int> numOfBills_;
            std::shared_ptr<ItemTargetDataClass> position_;
        };

        /// <summary>
        /// Constructor
        /// </summary>
        explicit CalibrateCashUnitCompletionPayloadData(
            std::optional<ErrorCodeEnum> errorCode = std::nullopt,
            std::shared_ptr<ResultClass> result = nullptr)
            : errorCode_(errorCode)
            , result_(result)
        {
        }

        /// <summary>
        /// Specifies the error code if applicable, otherwise null. Following values are possible:
        /// 
        /// * cashUnitError - A storage unit caused an error. A
        ///   Storage.StorageErrorEvent will be sent with the details.
        /// * unsupportedPosition - The position specified is not valid.
        /// * exchangeActive - The device is in an exchange state.
        /// * invalidCashUnit - The storage unit number specified is not valid.
        /// </summary>
        std::optional<ErrorCodeEnum> GetErrorCode() const { return errorCode_; }
        void setErrorCode(std::optional<ErrorCodeEnum> errorCode) { errorCode_ = errorCode; }

        /// <summary>
        /// The result of the command, detailing where items were moved from and to. May be null if no items were
        /// moved.
        /// </summary>
        std::shared_ptr<ResultClass> GetResult() const { return result_; }
        void setResult(std::shared_ptr<ResultClass> result) { result_ = result; }

        //// Serialization support
        //virtual std::string toJson() const override;
        //virtual void fromJson(const std::string& json) override;

    private:
        std::optional<ErrorCodeEnum> errorCode_;
        std::shared_ptr<ResultClass> result_;

    };

    class CalibrateCashUnitCompletion final : public XFS4IoT::Completion<CalibrateCashUnitCompletionPayloadData>
    {
    public:
        static constexpr const char* CompletionName = "CashManagement.CalibrateCashUnit";
        static constexpr const char* Version = "1.0";

        
        /// <summary>
        /// Constructor for CalibrateCashUnit completion
        /// </summary>
        CalibrateCashUnitCompletion(
            int requestId,
            std::shared_ptr<CalibrateCashUnitCompletionPayloadData> payload,
            MessageHeader::CompletionCodeEnum completionCode,
            const std::string& errorDescription)
            : Completion<CalibrateCashUnitCompletionPayloadData>(
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

    inline bool CalibrateCashUnitCompletion::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(CalibrateCashUnitCompletion),
                CalibrateCashUnitCompletion::CompletionName,
                CalibrateCashUnitCompletion::Version);
            return true;
        }();


    // Helper functions for ErrorCodeEnum
    inline std::string ToString(CalibrateCashUnitCompletionPayloadData::ErrorCodeEnum errorCode)
    {
        switch (errorCode)
        {
        case CalibrateCashUnitCompletionPayloadData::ErrorCodeEnum::CashUnitError:
            return "cashUnitError";
        case CalibrateCashUnitCompletionPayloadData::ErrorCodeEnum::UnsupportedPosition:
            return "unsupportedPosition";
        case CalibrateCashUnitCompletionPayloadData::ErrorCodeEnum::ExchangeActive:
            return "exchangeActive";
        case CalibrateCashUnitCompletionPayloadData::ErrorCodeEnum::InvalidCashUnit:
            return "invalidCashUnit";
        default:
            throw std::invalid_argument("Unknown ErrorCodeEnum value");
        }
    }

    inline CalibrateCashUnitCompletionPayloadData::ErrorCodeEnum parseErrorCodeEnum(const std::string& str)
    {
        if (str == "cashUnitError")
            return CalibrateCashUnitCompletionPayloadData::ErrorCodeEnum::CashUnitError;
        if (str == "unsupportedPosition")
            return CalibrateCashUnitCompletionPayloadData::ErrorCodeEnum::UnsupportedPosition;
        if (str == "exchangeActive")
            return CalibrateCashUnitCompletionPayloadData::ErrorCodeEnum::ExchangeActive;
        if (str == "invalidCashUnit")
            return CalibrateCashUnitCompletionPayloadData::ErrorCodeEnum::InvalidCashUnit;

        throw std::invalid_argument("Unknown ErrorCodeEnum string: " + str);
    }

    inline void to_json(
        nlohmann::json& j,
        const CalibrateCashUnitCompletionPayloadData::ResultClass& r)
    {
        j = nlohmann::json::object();

        if (r.getUnit().has_value())
            j["unit"] = r.getUnit().value();

        if (r.getNumOfBills().has_value())
            j["numOfBills"] = r.getNumOfBills().value();

        if (r.getPosition())
            j["position"] = *r.getPosition();
    }

    inline void to_json(
        nlohmann::json& j,
        const CalibrateCashUnitCompletionPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetErrorCode().has_value())
            j["errorCode"] = ToString(p.GetErrorCode().value());

        if (p.GetResult())
            j["result"] = *p.GetResult();
    }
}