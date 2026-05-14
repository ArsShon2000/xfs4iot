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
	/// Payload data for CashInRollback completion
	/// </summary>
	class CashInRollbackPayloadData : public XFS4IoT::MessagePayloadBase
	{
	public:
		enum class ErrorCodeEnum
		{
			CashUnitError,
			ShutterNotOpen,
			ExchangeActive,
			NoCashInActive,
			PositionNotEmpty,
			NoItems
		};

		CashInRollbackPayloadData(
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
		/// * ShutterNotOpen - The shutter failed to open. In the case of explicit shutter control
		///   the application may have failed to open the shutter before issuing the command.
		/// * ExchangeActive - The device is in an exchange state.
		/// * NoCashInActive - There is no cash-in transaction active.
		/// * PositionNotEmpty - The input or output position is not empty.
		/// * NoItems - There were no items to rollback.
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
	/// CashInRollback completion
	/// Version: 2.0
	/// Name: CashAcceptor.CashInRollback
	/// </summary>
	class CashInRollbackCompletion : public Completion<CashInRollbackPayloadData>
	{
	public:
		static constexpr const char* CompletionName = "CashAcceptor.CashInRollback";
		static constexpr const char* Version = "1.0";
		CashInRollbackCompletion(
			int requestId,
			std::shared_ptr<CashInRollbackPayloadData> payload,
			MessageHeader::CompletionCodeEnum completionCode,
			const std::string& errorDescription)
			: Completion<CashInRollbackPayloadData>(
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

	inline bool CashInRollbackCompletion::registered_ = []()
		{
			XFS4IoT::MessageBase::RegisterMessage(
				typeid(CashInRollbackCompletion),
				CashInRollbackCompletion::CompletionName,
				CashInRollbackCompletion::Version);
			return true;
		}();

	/// <summary>
	/// Helper function to convert ErrorCodeEnum to string
	/// </summary>
	inline std::string ToString(CashInRollbackPayloadData::ErrorCodeEnum code)
	{
		switch (code) {
		case CashInRollbackPayloadData::ErrorCodeEnum::CashUnitError:
			return "cashUnitError";
		case CashInRollbackPayloadData::ErrorCodeEnum::ShutterNotOpen:
			return "shutterNotOpen";
		case CashInRollbackPayloadData::ErrorCodeEnum::ExchangeActive:
			return "exchangeActive";
		case CashInRollbackPayloadData::ErrorCodeEnum::NoCashInActive:
			return "noCashInActive";
		case CashInRollbackPayloadData::ErrorCodeEnum::PositionNotEmpty:
			return "positionNotEmpty";
		case CashInRollbackPayloadData::ErrorCodeEnum::NoItems:
			return "noItems";
		default:
			return "unknown";
		}
	}

	inline void to_json(
		nlohmann::json& j,
		const CashInRollbackPayloadData& p)
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