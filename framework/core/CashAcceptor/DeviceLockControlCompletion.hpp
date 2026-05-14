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
	/// Payload data for DeviceLockControl completion
	/// </summary>
	class DeviceLockControlPayloadData : public XFS4IoT::MessagePayloadBase
	{
	public:
		enum class ErrorCodeEnum
		{
			InvalidCashUnit,
			CashInActive,
			ExchangeActive,
			DeviceLockFailure
		};

		DeviceLockControlPayloadData(std::optional<ErrorCodeEnum> errorCode = std::nullopt)
			: errorCode_(errorCode)
		{
		}

		/// <summary>
		/// Specifies the error code if applicable, otherwise null. The following values are possible:
		/// 
		/// * InvalidCashUnit - The storage unit type specified is invalid.
		/// * CashInActive - A cash-in transaction is active.
		/// * ExchangeActive - The device is in the exchange state.
		/// * DeviceLockFailure - The device and/or the storage units specified could not be locked/unlocked,
		///   e.g., the lock action could not be performed because the storage unit specified to be locked had been
		///   removed.
		/// </summary>
		std::optional<ErrorCodeEnum> GetErrorCode() const { return errorCode_; }
		void SetErrorCode(std::optional<ErrorCodeEnum> errorCode) { errorCode_ = errorCode; }

	private:
		std::optional<ErrorCodeEnum> errorCode_;
	};

	/// <summary>
	/// DeviceLockControl completion
	/// Version: 2.0
	/// Name: CashAcceptor.DeviceLockControl
	/// </summary>
	class DeviceLockControlCompletion : public Completion<DeviceLockControlPayloadData>
	{
	public:
		static constexpr const char* CompletionName = "CashAcceptor.DeviceLockControl";
		static constexpr const char* Version = "1.0";

		DeviceLockControlCompletion(
			int requestId,
			std::shared_ptr<DeviceLockControlPayloadData> payload,
			MessageHeader::CompletionCodeEnum completionCode,
			const std::string& errorDescription)
			: Completion<DeviceLockControlPayloadData>(
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

	inline bool DeviceLockControlCompletion::registered_ = []()
		{
			XFS4IoT::MessageBase::RegisterMessage(
				typeid(DeviceLockControlCompletion),
				DeviceLockControlCompletion::CompletionName,
				DeviceLockControlCompletion::Version);
			return true;
		}();

	/// <summary>
	/// Helper function to convert ErrorCodeEnum to string
	/// </summary>
	inline std::string ToString(DeviceLockControlPayloadData::ErrorCodeEnum code)
	{
		switch (code) {
		case DeviceLockControlPayloadData::ErrorCodeEnum::InvalidCashUnit:
			return "invalidCashUnit";
		case DeviceLockControlPayloadData::ErrorCodeEnum::CashInActive:
			return "cashInActive";
		case DeviceLockControlPayloadData::ErrorCodeEnum::ExchangeActive:
			return "exchangeActive";
		case DeviceLockControlPayloadData::ErrorCodeEnum::DeviceLockFailure:
			return "deviceLockFailure";
		default:
			return "unknown";
		}
	}

	inline void to_json(
		nlohmann::json& j,
		const DeviceLockControlPayloadData& p)
	{
		j = nlohmann::json::object();

		if (p.GetErrorCode().has_value())
			j["errorCode"] = ToString(p.GetErrorCode().value());
	}
}