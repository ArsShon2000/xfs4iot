#pragma once

#include <string>
#include <memory>
#include <optional>
#include <unordered_map>
#include "../../Completion.hpp"
#include "../../common/MessageHeader.hpp"
#include "../../common/Completions/MessagePayload.hpp"
#include "../CashManagementSchemas.hpp"

namespace XFS4IoT::CashManagement::Completions
{
	/// <summary>
	/// Reset completion message
	/// XFS4IoT Version: 2.0
	/// </summary>
	class ResetCompletionPayloadData final : public XFS4IoT::MessagePayloadBase
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
		explicit ResetCompletionPayloadData(
			std::optional<ErrorCodeEnum> errorCode = std::nullopt,
			std::optional<std::unordered_map<std::string, std::shared_ptr<StorageCashInClass>>> storage = std::nullopt,
			std::shared_ptr<StorageCashCountsClass> transport = nullptr,
			std::shared_ptr<StorageCashCountsClass> stacker = nullptr)
			: errorCode_(errorCode)
			, storage_(std::move(storage))
			, transport_(transport)
			, stacker_(stacker)
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
		const std::optional<std::unordered_map<std::string, std::shared_ptr<StorageCashInClass>>>&
			GetStorage() const { return storage_; }

		void setStorage(std::optional<std::unordered_map<std::string, std::shared_ptr<StorageCashInClass>>> storage)
		{
			storage_ = std::move(storage);
		}

		/// <summary>
		/// List of items moved to transport by this transaction or command.
		/// </summary>
		std::shared_ptr<StorageCashCountsClass> getTransport() const { return transport_; }
		void setTransport(std::shared_ptr<StorageCashCountsClass> transport) { transport_ = transport; }

		/// <summary>
		/// List of items moved to stacker by this transaction or command.
		/// </summary>
		std::shared_ptr<StorageCashCountsClass> getStacker() const { return stacker_; }
		void setStacker(std::shared_ptr<StorageCashCountsClass> stacker) { stacker_ = stacker; }

		//// Serialization support
		//virtual std::string toJson() const override;
		//virtual void fromJson(const std::string& json) override;

	private:
		std::optional<ErrorCodeEnum> errorCode_;
		std::optional<std::unordered_map<std::string, std::shared_ptr<StorageCashInClass>>> storage_;
		std::shared_ptr<StorageCashCountsClass> transport_;
		std::shared_ptr<StorageCashCountsClass> stacker_;
	};

	class ResetCompletion final : public XFS4IoT::Completion<ResetCompletionPayloadData>
	{
	public:
		static constexpr const char* CompletionName = "CashManagement.Reset";
		static constexpr const char* Version = "1.0";

		/// <summary>
		/// Constructor for Reset completion
		/// </summary>
		ResetCompletion(
			int requestId,
			std::shared_ptr<ResetCompletionPayloadData> payload,
			MessageHeader::CompletionCodeEnum completionCode,
			const std::string& errorDescription)
			: Completion<ResetCompletionPayloadData>(
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

	inline bool ResetCompletion::registered_ = []()
		{
			XFS4IoT::MessageBase::RegisterMessage(
				typeid(ResetCompletion),
				ResetCompletion::CompletionName,
				ResetCompletion::Version);
			return true;
		}();

	// Helper functions for ErrorCodeEnum
	inline std::string ToString(ResetCompletionPayloadData::ErrorCodeEnum errorCode)
	{
		switch (errorCode)
		{
		case ResetCompletionPayloadData::ErrorCodeEnum::CashUnitError:
			return "cashUnitError";
		case ResetCompletionPayloadData::ErrorCodeEnum::UnsupportedPosition:
			return "unsupportedPosition";
		case ResetCompletionPayloadData::ErrorCodeEnum::InvalidCashUnit:
			return "invalidCashUnit";
		case ResetCompletionPayloadData::ErrorCodeEnum::InvalidRetractPosition:
			return "invalidRetractPosition";
		case ResetCompletionPayloadData::ErrorCodeEnum::NotRetractArea:
			return "notRetractArea";
		case ResetCompletionPayloadData::ErrorCodeEnum::PositionNotEmpty:
			return "positionNotEmpty";
		case ResetCompletionPayloadData::ErrorCodeEnum::ForeignItemsDetected:
			return "foreignItemsDetected";
		case ResetCompletionPayloadData::ErrorCodeEnum::IncompleteRetract:
			return "incompleteRetract";
		default:
			throw std::invalid_argument("Unknown ErrorCodeEnum value");
		}
	}

	inline void to_json(
		nlohmann::json& j,
		const ResetCompletionPayloadData& p)
	{
		j = nlohmann::json::object();

		if (p.GetErrorCode().has_value())
			j["errorCode"] = ToString(p.GetErrorCode().value());

		if (p.GetStorage().has_value())
		{
			j["storage"] = nlohmann::json::object();

			for (const auto& [key, value] : p.GetStorage().value())
			{
				if (value)
				{
					j["storage"][key] = *value;
				}
			}
		}

		if (p.getStacker())
			j["stacker"] = *p.getStacker();

		if (p.getTransport())
			j["transport"] = *p.getTransport();
	}

	//inline ResetCompletionPayloadData::ErrorCodeEnum parseErrorCodeEnum(const std::string& str)
	//{
	//    if (str == "cashUnitError")
	//        return ResetCompletionPayloadData::ErrorCodeEnum::CashUnitError;
	//    if (str == "unsupportedPosition")
	//        return ResetCompletionPayloadData::ErrorCodeEnum::UnsupportedPosition;
	//    if (str == "invalidCashUnit")
	//        return ResetCompletionPayloadData::ErrorCodeEnum::InvalidCashUnit;
	//    if (str == "invalidRetractPosition")
	//        return ResetCompletionPayloadData::ErrorCodeEnum::InvalidRetractPosition;
	//    if (str == "notRetractArea")
	//        return ResetCompletionPayloadData::ErrorCodeEnum::NotRetractArea;
	//    if (str == "positionNotEmpty")
	//        return ResetCompletionPayloadData::ErrorCodeEnum::PositionNotEmpty;
	//    if (str == "foreignItemsDetected")
	//        return ResetCompletionPayloadData::ErrorCodeEnum::ForeignItemsDetected;
	//    if (str == "incompleteRetract")
	//        return ResetCompletionPayloadData::ErrorCodeEnum::IncompleteRetract;

	//    throw std::invalid_argument("Unknown ErrorCodeEnum string: " + str);
	//}
}