#pragma once

#include <string>
#include <optional>
#include <memory>
#include "../../common/Completions/MessagePayload.hpp"
#include "../../../core/CashManagement/CashManagementSchemas.hpp"
#include "../../Completion.hpp"

namespace XFS4IoT::CashAcceptor::Completions
{
	/// <summary>
	/// CashIn completion
	/// Version: 2.0
	/// Name: CashAcceptor.CashIn
	/// </summary>
	class CashInCompletionPayloadData : public XFS4IoT::MessagePayloadBase
	{
	public:
		enum class ErrorCodeEnum
		{
			CashUnitError,
			TooManyItems,
			NoItems,
			ExchangeActive,
			ShutterNotClosed,
			NoCashInActive,
			PositionNotEmpty,
			SafeDoorOpen,
			ForeignItemsDetected,
			ShutterNotOpen
		};

		CashInCompletionPayloadData(
			std::optional<ErrorCodeEnum> errorCode = std::nullopt,
			std::shared_ptr<CashManagement::StorageCashCountsClass> items = nullptr)
			: errorCode_(errorCode)
			, items_(std::move(items))
		{
		}

		/// <summary>
		/// Specifies the error code if applicable, otherwise null. The following values are possible:
		/// 
		/// * CashUnitError - A problem occurred with a storage unit. A Storage.StorageErrorEvent will be sent.
		/// * TooManyItems - Too many items inserted. The cash-in stacker is full or limit reached.
		/// * NoItems - There were no items to cash-in.
		/// * ExchangeActive - The device is in an exchange state.
		/// * ShutterNotClosed - Shutter failed to close.
		/// * NoCashInActive - There is no cash-in transaction active.
		/// * PositionNotEmpty - The output position is not empty so a cash-in is not possible.
		/// * SafeDoorOpen - The safe door is open. This device requires the safe door to be closed.
		/// * ForeignItemsDetected - Foreign items have been detected inside the input position.
		/// * ShutterNotOpen - Shutter failed to open.
		/// </summary>
		std::optional<ErrorCodeEnum> GetErrorCode() const { return errorCode_; }
		void SetErrorCode(std::optional<ErrorCodeEnum> errorCode) { errorCode_ = errorCode; }

		/// <summary>
		/// Items detected during the command. May be null if no items were detected.
		/// This information is not cumulative over multiple CashIn commands.
		/// </summary>
		std::shared_ptr<CashManagement::StorageCashCountsClass> GetItems() const { return items_; }
		void SetItems(std::shared_ptr<CashManagement::StorageCashCountsClass> items)
		{
			items_ = std::move(items);
		}

	private:
		std::optional<ErrorCodeEnum> errorCode_;
		std::shared_ptr<CashManagement::StorageCashCountsClass> items_;
	};

	class CashInCompletion : public XFS4IoT::Completion<CashInCompletionPayloadData>
	{
	public:
		static constexpr const char* CompletionName = "CashAcceptor.CashIn";
		static constexpr const char* Version = "1.0";
		/// <summary>
		/// Payload data for CashIn completion
		/// </summary>
		CashInCompletion(
			int requestId,
			std::shared_ptr<CashInCompletionPayloadData> payload,
			MessageHeader::CompletionCodeEnum completionCode,
			const std::string& errorDescription)
			: Completion<CashInCompletionPayloadData>(
				CompletionName,
				Version,
				requestId,
				payload,
				completionCode,
				errorDescription)
		{
		}

	private:
		static bool registered_;
	};

	inline bool CashInCompletion::registered_ = []()
		{
			XFS4IoT::MessageBase::RegisterMessage(
				typeid(CashInCompletion),
				CashInCompletion::CompletionName,
				CashInCompletion::Version);
			return true;
		}();

	/// <summary>
	/// Helper function to convert ErrorCodeEnum to string
	/// </summary>
	inline std::string ToString(CashInCompletionPayloadData::ErrorCodeEnum code)
	{
		switch (code) {
		case CashInCompletionPayloadData::ErrorCodeEnum::CashUnitError:
			return "cashUnitError";
		case CashInCompletionPayloadData::ErrorCodeEnum::TooManyItems:
			return "tooManyItems";
		case CashInCompletionPayloadData::ErrorCodeEnum::NoItems:
			return "noItems";
		case CashInCompletionPayloadData::ErrorCodeEnum::ExchangeActive:
			return "exchangeActive";
		case CashInCompletionPayloadData::ErrorCodeEnum::ShutterNotClosed:
			return "shutterNotClosed";
		case CashInCompletionPayloadData::ErrorCodeEnum::NoCashInActive:
			return "noCashInActive";
		case CashInCompletionPayloadData::ErrorCodeEnum::PositionNotEmpty:
			return "positionNotEmpty";
		case CashInCompletionPayloadData::ErrorCodeEnum::SafeDoorOpen:
			return "safeDoorOpen";
		case CashInCompletionPayloadData::ErrorCodeEnum::ForeignItemsDetected:
			return "foreignItemsDetected";
		case CashInCompletionPayloadData::ErrorCodeEnum::ShutterNotOpen:
			return "shutterNotOpen";
		default:
			throw std::invalid_argument("Unknown ErrorCodeEnum value");
		}
	}

	inline void to_json(
		nlohmann::json& j,
		const CashInCompletionPayloadData& p)
	{
		j = nlohmann::json::object();

		if (p.GetErrorCode().has_value())
			j["errorCode"] = ToString(p.GetErrorCode().value());

		if (p.GetItems())
			j["items"] = *p.GetItems();
	}
}