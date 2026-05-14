#pragma once

#include <string>
#include <optional>
#include <memory>
#include <map>
#include "../common/Completions/MessagePayload.hpp"
#include "../../core/CashManagement/CashManagementSchemas.hpp"
#include "../Completion.hpp"

namespace XFS4IoT::CashAcceptor::Completions
{
	/// <summary>
	/// Payload data for CashInEnd completion
	/// </summary>
	class CashInEndPayloadData : public XFS4IoT::MessagePayloadBase
	{
	public:
		enum class ErrorCodeEnum
		{
			CashUnitError,
			NoItems,
			ExchangeActive,
			NoCashInActive,
			PositionNotEmpty,
			SafeDoorOpen
		};

		CashInEndPayloadData(
			std::optional<ErrorCodeEnum> errorCode = std::nullopt,
			std::optional<std::map<std::string, std::shared_ptr<CashManagement::StorageCashInClass>>> storage = std::nullopt)
			: errorCode_(errorCode)
			, storage_(std::move(storage))
		{
		}

		/// <summary>
		/// Specifies the error code if applicable, otherwise null. The following values are possible:
		/// 
		/// * CashUnitError - A problem occurred with a storage unit. A Storage.StorageErrorEvent will be sent.
		/// * NoItems - There were no items to cash-in.
		/// * ExchangeActive - The device is in an exchange state.
		/// * NoCashInActive - There is no cash-in transaction active.
		/// * PositionNotEmpty - The input or output position is not empty.
		/// * SafeDoorOpen - The safe door is open. This device requires the safe door to be closed in order
		///   to perform this command (see Common.Status property).
		/// </summary>
		std::optional<ErrorCodeEnum> GetErrorCode() const { return errorCode_; }
		void SetErrorCode(std::optional<ErrorCodeEnum> errorCode) { errorCode_ = errorCode; }

		/// <summary>
		/// Object containing the storage units which have had items inserted during the associated operation or
		/// transaction. Only storage units whose contents have been modified are included.
		/// </summary>
		std::optional<std::map<std::string, std::shared_ptr<CashManagement::StorageCashInClass>>> GetStorage() const { return storage_; }
		void SetStorage(std::optional<std::map<std::string, std::shared_ptr<CashManagement::StorageCashInClass>>> storage)
		{
			storage_ = std::move(storage);
		}

	private:
		std::optional<ErrorCodeEnum> errorCode_;
		std::optional<std::map<std::string, std::shared_ptr<CashManagement::StorageCashInClass>>> storage_;
	};

	/// <summary>
	/// CashInEnd completion
	/// Version: 2.0
	/// Name: CashAcceptor.CashInEnd
	/// </summary>
	class CashInEndCompletion : public XFS4IoT::Completion<CashInEndPayloadData>
	{
	public:
		static constexpr const char* CompletionName = "CashAcceptor.CashInEnd";
		static constexpr const char* Version = "1.0";

		CashInEndCompletion(
			int requestId,
			std::shared_ptr<CashInEndPayloadData> payload,
			MessageHeader::CompletionCodeEnum completionCode,
			const std::string& errorDescription)
			: Completion<CashInEndPayloadData>(
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

	inline bool CashInEndCompletion::registered_ = []()
		{
			XFS4IoT::MessageBase::RegisterMessage(
				typeid(CashInEndCompletion),
				CashInEndCompletion::CompletionName,
				CashInEndCompletion::Version);
			return true;
		}();

	/// <summary>
	/// Helper function to convert ErrorCodeEnum to string
	/// </summary>
	inline std::string ToString(CashInEndPayloadData::ErrorCodeEnum code)
	{
		switch (code) {
		case CashInEndPayloadData::ErrorCodeEnum::CashUnitError:
			return "cashUnitError";
		case CashInEndPayloadData::ErrorCodeEnum::NoItems:
			return "noItems";
		case CashInEndPayloadData::ErrorCodeEnum::ExchangeActive:
			return "exchangeActive";
		case CashInEndPayloadData::ErrorCodeEnum::NoCashInActive:
			return "noCashInActive";
		case CashInEndPayloadData::ErrorCodeEnum::PositionNotEmpty:
			return "positionNotEmpty";
		case CashInEndPayloadData::ErrorCodeEnum::SafeDoorOpen:
			return "safeDoorOpen";
		default:
			return "unknown";
		}
	}

	inline void to_json(
		nlohmann::json& j,
		const CashInEndPayloadData& p)
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
	}
}