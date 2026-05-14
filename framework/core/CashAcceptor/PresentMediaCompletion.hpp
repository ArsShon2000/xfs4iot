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
	/// Payload data for PresentMedia completion
	/// </summary>
	class PresentMediaPayloadData : public XFS4IoT::MessagePayloadBase
	{
	public:
		enum class ErrorCodeEnum
		{
			UnsupportedPosition,
			ShutterNotOpen,
			NoItems,
			ExchangeActive,
			ForeignItemsDetected
		};

		PresentMediaPayloadData(std::optional<ErrorCodeEnum> errorCode = std::nullopt)
			: errorCode_(errorCode)
		{
		}

		/// <summary>
		/// Specifies the error code if applicable, otherwise null. The following values are possible:
		/// 
		/// * UnsupportedPosition - The position specified is not supported or is not a valid position for
		///   this command.
		/// * ShutterNotOpen - Shutter failed to open.
		/// * NoItems - There were no items to present at the specified position.
		/// * ExchangeActive - The device is in the exchange state.
		/// * ForeignItemsDetected - Foreign items have been detected in the input position.
		/// </summary>
		std::optional<ErrorCodeEnum> GetErrorCode() const { return errorCode_; }
		void SetErrorCode(std::optional<ErrorCodeEnum> errorCode) { errorCode_ = errorCode; }

	private:
		std::optional<ErrorCodeEnum> errorCode_;
	};

	/// <summary>
	/// PresentMedia completion
	/// Version: 2.0
	/// Name: CashAcceptor.PresentMedia
	/// </summary>
	class PresentMediaCompletion : public Completion<PresentMediaPayloadData>
	{
	public:
		static constexpr const char* CompletionName = "CashAcceptor.PresentMedia";
		static constexpr const char* Version = "1.0";

		PresentMediaCompletion(
			int requestId,
			std::shared_ptr<PresentMediaPayloadData> payload,
			MessageHeader::CompletionCodeEnum completionCode,
			const std::string& errorDescription)
			: Completion<PresentMediaPayloadData>(
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

	inline bool PresentMediaCompletion::registered_ = []()
		{
			XFS4IoT::MessageBase::RegisterMessage(
				typeid(PresentMediaCompletion),
				PresentMediaCompletion::CompletionName,
				PresentMediaCompletion::Version);
			return true;
		}();

	/// <summary>
	/// Helper function to convert ErrorCodeEnum to string
	/// </summary>
	inline std::string ToString(PresentMediaPayloadData::ErrorCodeEnum code)
	{
		switch (code) {
		case PresentMediaPayloadData::ErrorCodeEnum::UnsupportedPosition:
			return "unsupportedPosition";
		case PresentMediaPayloadData::ErrorCodeEnum::ShutterNotOpen:
			return "shutterNotOpen";
		case PresentMediaPayloadData::ErrorCodeEnum::NoItems:
			return "noItems";
		case PresentMediaPayloadData::ErrorCodeEnum::ExchangeActive:
			return "exchangeActive";
		case PresentMediaPayloadData::ErrorCodeEnum::ForeignItemsDetected:
			return "foreignItemsDetected";
		default:
			return "unknown";
		}
	}

	inline void to_json(
		nlohmann::json& j,
		const PresentMediaPayloadData& p)
	{
		j = nlohmann::json::object();

		if (p.GetErrorCode().has_value())
			j["errorCode"] = ToString(p.GetErrorCode().value());
	}
}