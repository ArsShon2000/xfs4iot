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
	/// Payload data for ConfigureNoteReader completion
	/// </summary>
	class ConfigureNoteReaderPayloadData : public XFS4IoT::MessagePayloadBase
	{
	public:
		enum class ErrorCodeEnum
		{
			ExchangeActive,
			CashInActive,
			LoadFailed
		};

		ConfigureNoteReaderPayloadData(
			std::optional<ErrorCodeEnum> errorCode = std::nullopt,
			std::optional<bool> rebootNecessary = std::nullopt)
			: errorCode_(errorCode)
			, rebootNecessary_(rebootNecessary)
		{
		}

		/// <summary>
		/// Specifies the error code if applicable, otherwise null. The following values are possible:
		/// 
		/// * ExchangeActive - The device is in the exchange state.
		/// * CashInActive - A cash-in transaction is active.
		/// * LoadFailed - The load failed because the device is in a state that will not allow the
		///   configuration data to be loaded at this time, for example on some devices there may be notes present
		///   in the storage units when they should not be.
		/// </summary>
		std::optional<ErrorCodeEnum> GetErrorCode() const { return errorCode_; }
		void SetErrorCode(std::optional<ErrorCodeEnum> errorCode) { errorCode_ = errorCode; }

		/// <summary>
		/// If set to true, the machine needs a reboot before the note reader can be accessed again.
		/// </summary>
		std::optional<bool> GetRebootNecessary() const { return rebootNecessary_; }
		void SetRebootNecessary(std::optional<bool> rebootNecessary) { rebootNecessary_ = rebootNecessary; }

	private:
		std::optional<ErrorCodeEnum> errorCode_;
		std::optional<bool> rebootNecessary_;
	};

	/// <summary>
	/// ConfigureNoteReader completion
	/// Version: 2.0
	/// Name: CashAcceptor.ConfigureNoteReader
	/// </summary>
	class ConfigureNoteReaderCompletion : public Completion<ConfigureNoteReaderPayloadData>
	{
	public:
		static constexpr const char* CompletionName = "CashAcceptor.ConfigureNoteReader";
		static constexpr const char* Version = "1.0";

		ConfigureNoteReaderCompletion(
			int requestId,
			std::shared_ptr<ConfigureNoteReaderPayloadData> payload,
			MessageHeader::CompletionCodeEnum completionCode,
			const std::string& errorDescription)
			: Completion<ConfigureNoteReaderPayloadData>(
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

	inline bool ConfigureNoteReaderCompletion::registered_ = []()
		{
			XFS4IoT::MessageBase::RegisterMessage(
				typeid(ConfigureNoteReaderCompletion),
				ConfigureNoteReaderCompletion::CompletionName,
				ConfigureNoteReaderCompletion::Version);
			return true;
		}();

	/// <summary>
	/// Helper function to convert ErrorCodeEnum to string
	/// </summary>
	inline std::string ToString(ConfigureNoteReaderPayloadData::ErrorCodeEnum code)
	{
		switch (code) {
		case ConfigureNoteReaderPayloadData::ErrorCodeEnum::ExchangeActive:
			return "exchangeActive";
		case ConfigureNoteReaderPayloadData::ErrorCodeEnum::CashInActive:
			return "cashInActive";
		case ConfigureNoteReaderPayloadData::ErrorCodeEnum::LoadFailed:
			return "loadFailed";
		default:
			return "unknown";
		}
	}

	inline void to_json(
		nlohmann::json& j,
		const ConfigureNoteReaderPayloadData& p)
	{
		j = nlohmann::json::object();

		if (p.GetErrorCode().has_value())
			j["errorCode"] = ToString(p.GetErrorCode().value());

		if (p.GetRebootNecessary())
			j["rebootNecessary"] = p.GetRebootNecessary();
	}
}