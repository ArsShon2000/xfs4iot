#pragma once

#include <string>
#include <optional>
#include <memory>
#include "../common/Completions/MessagePayload.hpp"
#include "../Completion.hpp"
#include "../common/MessageHeader.hpp"
#include "../MessageBase.hpp"
#include "../../core/CashManagement/CashManagementSchemas.hpp"

namespace XFS4IoT::CashAcceptor::Completions
{
	/// <summary>
		/// Payload data for PreparePresent completion
		/// </summary>
	class PreparePresentPayloadData : public XFS4IoT::MessagePayloadBase
	{
	public:
		enum class ErrorCodeEnum
		{
			UnsupportedPosition,
			PositionNotEmpty,
			NoItems,
			CashUnitError
		};

		PreparePresentPayloadData(std::optional<ErrorCodeEnum> errorCode = std::nullopt)
			: errorCode_(errorCode)
		{
		}

		/// <summary>
		/// Specifies the error code if applicable, otherwise null. The following values are possible:
		/// 
		/// * UnsupportedPosition - The position specified is not supported or is not a valid position for
		///   this command.
		/// * PositionNotEmpty - The input or output position is not empty.
		/// * NoItems - There were no items to present at the specified position.
		/// * CashUnitError - A storage unit caused a problem. A Storage.StorageErrorEvent will be posted with the details.
		/// </summary>
		std::optional<ErrorCodeEnum> GetErrorCode() const { return errorCode_; }
		void SetErrorCode(std::optional<ErrorCodeEnum> errorCode) { errorCode_ = errorCode; }

	private:
		std::optional<ErrorCodeEnum> errorCode_;
	};


	/// <summary>
	/// PreparePresent completion
	/// Version: 2.0
	/// Name: CashAcceptor.PreparePresent
	/// </summary>
	class PreparePresentCompletion : public Completion<PreparePresentPayloadData>
	{
	public:
		static constexpr const char* CompletionName = "CashAcceptor.PreparePresent";
		static constexpr const char* Version = "1.0";

		PreparePresentCompletion(
			int requestId,
			std::shared_ptr<PreparePresentPayloadData> payload,
			MessageHeader::CompletionCodeEnum completionCode,
			const std::string& errorDescription)
			: Completion<PreparePresentPayloadData>(
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

	inline bool PreparePresentCompletion::registered_ = []()
		{
			XFS4IoT::MessageBase::RegisterMessage(
				typeid(PreparePresentCompletion),
				PreparePresentCompletion::CompletionName,
				PreparePresentCompletion::Version);
			return true;
		}();

	/// <summary>
	/// Helper function to convert ErrorCodeEnum to string
	/// </summary>
	inline std::string ToString(PreparePresentPayloadData::ErrorCodeEnum code)
	{
	    switch (code) {
	    case PreparePresentPayloadData::ErrorCodeEnum::UnsupportedPosition:
	        return "unsupportedPosition";
	    case PreparePresentPayloadData::ErrorCodeEnum::PositionNotEmpty:
	        return "positionNotEmpty";
	    case PreparePresentPayloadData::ErrorCodeEnum::NoItems:
	        return "noItems";
	    case PreparePresentPayloadData::ErrorCodeEnum::CashUnitError:
	        return "cashUnitError";
	    default:
	        return "unknown";
	    }
	}

	inline void to_json(
		nlohmann::json& j,
		const PreparePresentPayloadData& p)
	{
		j = nlohmann::json::object();

		if (p.GetErrorCode().has_value())
			j["errorCode"] = ToString(p.GetErrorCode().value());
	}
}