#pragma once

#include <string>
#include <memory>
#include <optional>
#include "../../Completion.hpp"
#include "../../common/MessageHeader.hpp"
#include "../../common/Completions/MessagePayload.hpp"

namespace XFS4IoT::CashManagement::Completions
{
	/// <summary>
	/// SetTellerInfo completion message
	/// XFS4IoT Version: 2.0
	/// </summary>
	class SetTellerInfoCompletionPayloadData final : public XFS4IoT::MessagePayloadBase
	{
	public:
		enum class ErrorCodeEnum
		{
			InvalidCurrency,
			InvalidTellerId,
			UnsupportedPosition,
			ExchangeActive
		};

		/// <summary>
		/// Constructor
		/// </summary>
		explicit SetTellerInfoCompletionPayloadData(std::optional<ErrorCodeEnum> errorCode = std::nullopt)
			: errorCode_(errorCode)
		{
		}

		/// <summary>
		/// Specifies the error code if applicable, otherwise null. Following values are possible:
		/// 
		/// * invalidCurrency - The specified currency is not currently available.
		/// * invalidTellerId - The teller ID is invalid.
		/// * unsupportedPosition - The position specified is not supported.
		/// * exchangeActive - The target teller is currently in the middle of an exchange operation.
		/// </summary>
		std::optional<ErrorCodeEnum> GetErrorCode() const { return errorCode_; }
		void setErrorCode(std::optional<ErrorCodeEnum> errorCode) { errorCode_ = errorCode; }

		//// Serialization support
		//virtual std::string toJson() const override;
		//virtual void fromJson(const std::string& json) override;

	private:
		std::optional<ErrorCodeEnum> errorCode_;
	};

	class SetTellerInfoCompletion final : public XFS4IoT::Completion<SetTellerInfoCompletionPayloadData>
	{
	public:
		static constexpr const char* CompletionName = "CashManagement.SetTellerInfo";
		static constexpr const char* Version = "1.0";

		/// <summary>
		/// Constructor for SetTellerInfo completion
		/// </summary>
		SetTellerInfoCompletion(
			int requestId,
			std::shared_ptr<SetTellerInfoCompletionPayloadData> payload,
			MessageHeader::CompletionCodeEnum completionCode,
			const std::string& errorDescription)
			: Completion<SetTellerInfoCompletionPayloadData>(
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

	inline bool SetTellerInfoCompletion::registered_ = []()
		{
			XFS4IoT::MessageBase::RegisterMessage(
				typeid(SetTellerInfoCompletion),
				SetTellerInfoCompletion::CompletionName,
				SetTellerInfoCompletion::Version);
			return true;
		}();

	// Helper functions for ErrorCodeEnum
	inline std::string ToString(SetTellerInfoCompletionPayloadData::ErrorCodeEnum errorCode)
	{
		switch (errorCode)
		{
		case SetTellerInfoCompletionPayloadData::ErrorCodeEnum::InvalidCurrency:
			return "invalidCurrency";
		case SetTellerInfoCompletionPayloadData::ErrorCodeEnum::InvalidTellerId:
			return "invalidTellerId";
		case SetTellerInfoCompletionPayloadData::ErrorCodeEnum::UnsupportedPosition:
			return "unsupportedPosition";
		case SetTellerInfoCompletionPayloadData::ErrorCodeEnum::ExchangeActive:
			return "exchangeActive";
		default:
			throw std::invalid_argument("Unknown ErrorCodeEnum value");
		}
	}

	inline void to_json(
		nlohmann::json& j,
		const SetTellerInfoCompletionPayloadData& p)
	{
		j = nlohmann::json::object();

		if (p.GetErrorCode().has_value())
			j["errorCode"] = ToString(p.GetErrorCode().value());

	}

	//inline SetTellerInfoCompletionPayloadData::ErrorCodeEnum parseErrorCodeEnum(const std::string& str)
	//{
	//    if (str == "invalidCurrency")
	//        return SetTellerInfoCompletionPayloadData::ErrorCodeEnum::InvalidCurrency;
	//    if (str == "invalidTellerId")
	//        return SetTellerInfoCompletionPayloadData::ErrorCodeEnum::InvalidTellerId;
	//    if (str == "unsupportedPosition")
	//        return SetTellerInfoCompletionPayloadData::ErrorCodeEnum::UnsupportedPosition;
	//    if (str == "exchangeActive")
	//        return SetTellerInfoCompletionPayloadData::ErrorCodeEnum::ExchangeActive;

	//    throw std::invalid_argument("Unknown ErrorCodeEnum string: " + str);
	//}
}