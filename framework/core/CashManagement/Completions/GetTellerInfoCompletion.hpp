#pragma once

#include <string>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>
#include "../../Completion.hpp"
#include "../../common/MessageHeader.hpp"
#include "../../common/Completions/MessagePayload.hpp"
#include "../CashManagementSchemas.hpp"

namespace XFS4IoT::CashManagement::Completions
{
	/// <summary>
	/// GetTellerInfoCompletion completion message
	/// XFS4IoT Version: 2.0
	/// </summary>
	class GetTellerInfoCompletionPayloadData final : public XFS4IoT::MessagePayloadBase
	{
	public:
		enum class ErrorCodeEnum
		{
			CashUnitError,
			UnsupportedPosition,
			InvalidCashUnit,
			InvalidRetractPosition,
			NotRetractArea,
			PositionNotEmpty,
			ForeignItemsDetected,
			IncompleteRetract
		};

		/// <summary>
		/// Constructor
		/// </summary>
		explicit GetTellerInfoCompletionPayloadData(
			std::optional<ErrorCodeEnum> errorCode = std::nullopt,
			std::vector<TellerDetailsClass> tellerDetails = {})
			: errorCode_(errorCode)
			, tellerDetails_(std::move(tellerDetails))
		{
		}

		/// <summary>
		/// Specifies the error code if applicable, otherwise null. Following values are possible:
		/// 
		/// * cashUnitError - There is a problem with a storage unit. A
		///   Storage.StorageErrorEvent will be posted with the details.
		/// * unsupportedPosition - The output position specified is not supported.
		/// * invalidCashUnit - The storage unit number specified is not valid.
		/// * invalidRetractPosition - The *index* is not supported.
		/// * notRetractArea - The retract area specified in *retractArea* is not supported.
		/// * positionNotEmpty - The retract area specified in *retractArea* is not empty so the moving of
		///   items was not possible.
		/// * foreignItemsDetected - Foreign items have been detected in the input position.
		/// * incompleteRetract - Some or all of the items were not retracted for a reason not covered by
		///   other error codes. The detail will be reported with a
		///   CashManagement.IncompleteRetractEvent.
		/// </summary>
		std::optional<ErrorCodeEnum> GetErrorCode() const { return errorCode_; }
		void setErrorCode(std::optional<ErrorCodeEnum> errorCode) { errorCode_ = errorCode; }

		/// <summary>
		/// Object containing the storage units which have had items inserted during the associated operation or
		/// transaction. Only storage units whose contents have been modified are included.
		/// </summary>
		const std::vector<TellerDetailsClass>&
			getTellerDetails() const { return tellerDetails_; }

		void setTellerDetails(std::vector<TellerDetailsClass> tellerDetails)
		{
			tellerDetails_ = std::move(tellerDetails);
		}


		//// Serialization support
		//virtual std::string toJson() const override;
		//virtual void fromJson(const std::string& json) override;

	private:
		std::optional<ErrorCodeEnum> errorCode_;
		std::vector<TellerDetailsClass> tellerDetails_;
	};

	class GetTellerInfoCompletion final : public XFS4IoT::Completion<GetTellerInfoCompletionPayloadData>
	{
	public:
		static constexpr const char* CompletionName = "CashManagement.GetTellerInfo";
		static constexpr const char* Version = "1.0";

		/// <summary>
		/// Constructor for Reset completion
		/// </summary>
		GetTellerInfoCompletion(
			int requestId,
			std::shared_ptr<GetTellerInfoCompletionPayloadData> payload,
			MessageHeader::CompletionCodeEnum completionCode,
			const std::string& errorDescription)
			: Completion<GetTellerInfoCompletionPayloadData>(
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

	inline bool GetTellerInfoCompletion::registered_ = []()
		{
			XFS4IoT::MessageBase::RegisterMessage(
				typeid(GetTellerInfoCompletion),
				GetTellerInfoCompletion::CompletionName,
				GetTellerInfoCompletion::Version);
			return true;
		}();

	/// <summary>
	/// Helper function to convert ErrorCodeEnum to string
	/// </summary>
	inline std::string ToString(GetTellerInfoCompletionPayloadData::ErrorCodeEnum code)
	{
		switch (code) {
		case GetTellerInfoCompletionPayloadData::ErrorCodeEnum::CashUnitError:
			return "cashUnitError";
		case GetTellerInfoCompletionPayloadData::ErrorCodeEnum::UnsupportedPosition:
			return "unsupportedPosition";
		case GetTellerInfoCompletionPayloadData::ErrorCodeEnum::InvalidCashUnit:
			return "invalidCashUnit";
		case GetTellerInfoCompletionPayloadData::ErrorCodeEnum::InvalidRetractPosition:
			return "invalidRetractPosition";
		case GetTellerInfoCompletionPayloadData::ErrorCodeEnum::NotRetractArea:
			return "notRetractArea";
		case GetTellerInfoCompletionPayloadData::ErrorCodeEnum::PositionNotEmpty:
			return "nositionNotEmpty";
		case GetTellerInfoCompletionPayloadData::ErrorCodeEnum::ForeignItemsDetected:
			return "foreignItemsDetected";
		case GetTellerInfoCompletionPayloadData::ErrorCodeEnum::IncompleteRetract:
			return "incompleteRetract";
		default:
			return "unknown";
		}
	}


	inline void to_json(
		nlohmann::json& j,
		const GetTellerInfoCompletionPayloadData& p)
	{
		j = nlohmann::json::object();

		if (p.GetErrorCode().has_value())
			j["errorCode"] = ToString(p.GetErrorCode().value());

		if (!p.getTellerDetails().empty())
			j["tellerDetails"] = p.getTellerDetails();
	}
			
			
			
			
}