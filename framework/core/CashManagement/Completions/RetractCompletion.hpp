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
	/// Retract completion message
	/// XFS4IoT Version: 2.0
	/// </summary>
	class RetractCompletionPayloadData final : public XFS4IoT::MessagePayloadBase
	{
	public:
		enum class ErrorCodeEnum
		{
			CashUnitError,
			NoItems,
			ExchangeActive,
			ShutterNotClosed,
			ItemsTaken,
			InvalidRetractPosition,
			NotRetractArea,
			ForeignItemsDetected,
			PositionNotEmpty,
			IncompleteRetract
		};

		/// <summary>
		/// Constructor
		/// </summary>
		explicit RetractCompletionPayloadData(
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
		/// * ```cashUnitError``` - A problem occurred with a storage unit. A
		///   [Storage.StorageErrorEvent](#storage.storageerrorevent) will be sent with the details.
		/// * ```noItems``` - There were no items to retract.
		/// * ```exchangeActive``` - The device is in an exchange state.
		/// * ```shutterNotClosed``` - The shutter failed to close.
		/// * ```itemsTaken``` - Items were present at the output position at the start of the operation, but were
		///   removed before the operation was complete - some or all of the items were not retracted.
		/// * ```invalidRetractPosition``` - The *index* is not supported.
		/// * ```notRetractArea``` - The retract area specified in *retractArea* is not supported.
		/// * ```foreignItemsDetected``` - Foreign items have been detected inside the input position.
		/// * ```positionNotEmpty``` - The retract area specified in *retractArea* is not empty so the retract
		///   operation is not possible.
		/// * ```incompleteRetract``` - Some or all of the items were not retracted for a reason not covered by
		///   other error codes. The detail will be reported with a
		///   [CashManagement.IncompleteRetractEvent](#cashmanagement.incompleteretractevent).
		/// </summary>
		/// 
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

	class RetractCompletion final : public XFS4IoT::Completion<RetractCompletionPayloadData>
	{
	public:
		static constexpr const char* CompletionName = "CashManagement.Retract";
		static constexpr const char* Version = "1.0";

		/// <summary>
		/// Constructor for Reset completion
		/// </summary>
		RetractCompletion(
			int requestId,
			std::shared_ptr<RetractCompletionPayloadData> payload,
			MessageHeader::CompletionCodeEnum completionCode,
			const std::string& errorDescription)
			: Completion<RetractCompletionPayloadData>(
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

	inline bool RetractCompletion::registered_ = []()
		{
			XFS4IoT::MessageBase::RegisterMessage(
				typeid(RetractCompletion),
				RetractCompletion::CompletionName,
				RetractCompletion::Version);
			return true;
		}();

	/// <summary>
	/// Helper function to convert ErrorCodeEnum to string
	/// </summary>
	inline std::string ToString(RetractCompletionPayloadData::ErrorCodeEnum code)
	{
		switch (code) {
		case RetractCompletionPayloadData::ErrorCodeEnum::CashUnitError:
			return "cashUnitError";
		case RetractCompletionPayloadData::ErrorCodeEnum::ItemsTaken:
			return "itemsTaken";
		case RetractCompletionPayloadData::ErrorCodeEnum::NoItems:
			return "noItems";
		case RetractCompletionPayloadData::ErrorCodeEnum::ExchangeActive:
			return "exchangeActive";
		case RetractCompletionPayloadData::ErrorCodeEnum::ShutterNotClosed:
			return "shutterNotClosed";
		case RetractCompletionPayloadData::ErrorCodeEnum::InvalidRetractPosition:
			return "invalidRetractPosition";
		case RetractCompletionPayloadData::ErrorCodeEnum::PositionNotEmpty:
			return "positionNotEmpty";
		case RetractCompletionPayloadData::ErrorCodeEnum::NotRetractArea:
			return "notRetractArea";
		case RetractCompletionPayloadData::ErrorCodeEnum::ForeignItemsDetected:
			return "foreignItemsDetected";
		case RetractCompletionPayloadData::ErrorCodeEnum::IncompleteRetract:
			return "incompleteRetract";
		default:
			throw std::invalid_argument("Unknown ErrorCodeEnum value");
		}
	}

	inline void to_json(
		nlohmann::json& j,
		const RetractCompletionPayloadData& p)
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
}