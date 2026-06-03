#pragma once

#include <string>
#include <memory>
#include <optional>
#include <nlohmann/json.hpp>

#include "common/MessageHeader.hpp"
#include "common/XFSConstants.hpp"
#include "MessageRegistry.hpp"

namespace XFS4IoT
{
	class MessageBase
	{
	public:
		virtual ~MessageBase() = default;

		const MessageHeader& Header() const noexcept
		{
			return header_;
		}

		std::string Serialise() const
		{
			nlohmann::json j;
			j["header"] = SerializeHeader();
			SerializePayload(j);
			return j.dump();
		}

		std::string ToString() const
		{
			return Serialise();
		}
		inline std::string ToString(MessageHeader::TypeEnum v) const
		{
			switch (v)
			{
			case MessageHeader::TypeEnum::Command:      return "command";
			case MessageHeader::TypeEnum::Acknowledge: return "acknowledge";
			case MessageHeader::TypeEnum::Event:       return "event";
			case MessageHeader::TypeEnum::Completion:  return "completion";
			case MessageHeader::TypeEnum::Unsolicited: return "unsolicited";
			}
			throw std::logic_error("Unknown TypeEnum");
		}
		inline static MessageHeader::TypeEnum FromString(const std::string& v)
		{
			if (v == "command")      return MessageHeader::TypeEnum::Command;
			if (v == "acknowledge")  return MessageHeader::TypeEnum::Acknowledge;
			if (v == "event")        return MessageHeader::TypeEnum::Event;
			if (v == "completion")   return MessageHeader::TypeEnum::Completion;
			if (v == "unsolicited")  return MessageHeader::TypeEnum::Unsolicited;
			throw std::logic_error("Unknown TypeEnum");
		}

		inline std::string ToString(MessageHeader::StatusEnum v) const
		{
			switch (v)
			{
			case MessageHeader::StatusEnum::InvalidMessage:   return "invalidMessage";
			case MessageHeader::StatusEnum::InvalidRequestID:return "invalidRequestID";
			case MessageHeader::StatusEnum::TooManyRequests: return "tooManyRequests";
			}
			throw std::logic_error("Unknown StatusEnum");
		}

		inline std::string ToString(MessageHeader::CompletionCodeEnum v) const
		{
			switch (v)
			{
			case MessageHeader::CompletionCodeEnum::Success: return "success";
			case MessageHeader::CompletionCodeEnum::CommandErrorCode: return "commandErrorCode";
			case MessageHeader::CompletionCodeEnum::Canceled: return "canceled";
			case MessageHeader::CompletionCodeEnum::DeviceNotReady: return "deviceNotReady";
			case MessageHeader::CompletionCodeEnum::HardwareError: return "hardwareError";
			case MessageHeader::CompletionCodeEnum::InternalError: return "internalError";
			case MessageHeader::CompletionCodeEnum::InvalidCommand: return "invalidCommand";
			case MessageHeader::CompletionCodeEnum::InvalidRequestID: return "invalidRequestID";
			case MessageHeader::CompletionCodeEnum::TimeOut: return "timeOut";
			case MessageHeader::CompletionCodeEnum::UnsupportedCommand: return "unsupportedCommand";
			case MessageHeader::CompletionCodeEnum::InvalidData: return "invalidData";
			case MessageHeader::CompletionCodeEnum::UserError: return "userError";
			case MessageHeader::CompletionCodeEnum::UnsupportedData: return "unsupportedData";
			case MessageHeader::CompletionCodeEnum::FraudAttempt: return "fraudAttempt";
			case MessageHeader::CompletionCodeEnum::SequenceError: return "sequenceError";
			case MessageHeader::CompletionCodeEnum::AuthorisationRequired: return "authorisationRequired";
			case MessageHeader::CompletionCodeEnum::NoCommandNonce: return "noCommandNonce";
			case MessageHeader::CompletionCodeEnum::InvalidToken: return "invalidToken";
			case MessageHeader::CompletionCodeEnum::InvalidTokenNonce: return "invalidTokenNonce";
			case MessageHeader::CompletionCodeEnum::InvalidTokenHMAC: return "invalidTokenHMAC";
			case MessageHeader::CompletionCodeEnum::InvalidTokenFormat: return "invalidTokenFormat";
			case MessageHeader::CompletionCodeEnum::InvalidTokenKeyNoValue: return "invalidTokenKeyNoValue";
			case MessageHeader::CompletionCodeEnum::NotEnoughSpace: return "notEnoughSpace";
			}
			throw std::logic_error("Unknown CompletionCodeEnum");
		}

		inline std::string_view ToString(XFSConstants::ServiceClass sc) const
		{
			switch (sc)
			{
			case XFSConstants::ServiceClass::CardReader: return "CardReader";
			case XFSConstants::ServiceClass::Publisher: return "Publisher";
			case XFSConstants::ServiceClass::Printer: return "Printer";
			case XFSConstants::ServiceClass::TextTerminal: return "TextTerminal";
			case XFSConstants::ServiceClass::PinPad: return "PinPad";
			case XFSConstants::ServiceClass::Common: return "Common";
			case XFSConstants::ServiceClass::CashDispenser: return "CashDispenser";
			case XFSConstants::ServiceClass::CashManagement: return "CashManagement";
			case XFSConstants::ServiceClass::Crypto: return "Crypto";
			case XFSConstants::ServiceClass::Keyboard: return "Keyboard";
			case XFSConstants::ServiceClass::KeyManagement: return "KeyManagement";
			case XFSConstants::ServiceClass::Storage: return "Storage";
			case XFSConstants::ServiceClass::Lights: return "Lights";
			case XFSConstants::ServiceClass::Auxiliaries: return "Auxiliaries";
			case XFSConstants::ServiceClass::VendorApplication: return "VendorApplication";
			case XFSConstants::ServiceClass::VendorMode: return "VendorMode";
			case XFSConstants::ServiceClass::BarcodeReader: return "BarcodeReader";
			case XFSConstants::ServiceClass::Biometric: return "Biometric";
			case XFSConstants::ServiceClass::Camera: return "Camera";
			case XFSConstants::ServiceClass::CashAcceptor: return "CashAcceptor";
			case XFSConstants::ServiceClass::Check: return "Check";
			case XFSConstants::ServiceClass::MixedMedia: return "MixedMedia";
			case XFSConstants::ServiceClass::Deposit: return "Deposit";
			case XFSConstants::ServiceClass::German: return "German";
			case XFSConstants::ServiceClass::BanknoteNeutralization: return "BanknoteNeutralization";
			case XFSConstants::ServiceClass::PowerManagement: return "PowerManagement";
			}
			return "Unknown";
		}


		//virtual std::unique_ptr<MessageBase> Clone() const = 0;

		/// <summary>
		/// Parse extended properties from JSON elements dictionary
		/// </summary>
		template<typename T>
		[[nodiscard]] static std::unordered_map<std::string, T> ParseExtendedProperties(
			const std::unordered_map<std::string, nlohmann::json>& elements)
		{
			std::unordered_map<std::string, T> result;

			for (const auto& [key, value] : elements)
			{
				try
				{
					result[key] = value.get<T>();
				}
				catch (const nlohmann::json::exception& e)
				{
					throw std::runtime_error(
						std::string("Failed to parse extended property '") + key + "': " + e.what());
				}
			}

			return result;
		}

		/// <summary>
		/// Create extension data dictionary from typed objects
		/// </summary>
		template<typename T>
		[[nodiscard]] static std::unordered_map<std::string, nlohmann::json> CreateExtensionData(
			const std::unordered_map<std::string, T>& elements)
		{
			std::unordered_map<std::string, nlohmann::json> result;

			for (const auto& [key, value] : elements)
			{
				result[key] = ElementFromObject(value);
			}

			return result;
		}

		/// <summary>
		/// Create JSON element from object
		/// </summary>
		template<typename T>
		[[nodiscard]] static nlohmann::json ElementFromObject(const T& obj)
		{
			return nlohmann::json(obj);
		}


		static void RegisterMessage(const std::type_info& type,
			const std::string& name,
			const std::string& version)
		{
			GetMessageRegistry().Register(type, name, version);
		}

	protected:
		MessageBase(std::string name,
			std::optional<int> requestId,
			std::string version,
			MessageHeader::TypeEnum type,
			std::optional<int> timeout = std::nullopt,
			std::optional<MessageHeader::StatusEnum> status = std::nullopt,
			std::optional<MessageHeader::CompletionCodeEnum> completionCode = std::nullopt,
			std::optional<std::string> errorDescription = std::nullopt)
		{
			if (type != MessageHeader::TypeEnum::Command) {
				timeout = std::nullopt;
			}
			else if (!timeout.has_value()) {
				timeout = 0;
			}

			header_ = MessageHeader(std::move(name),
				requestId,
				std::move(version), 
				type,
				timeout, 
				status, 
				completionCode, 
				std::move(errorDescription));
		}

		/// <summary>
		/// Internal constructor of the base message object
		/// For use by JsonSerializer/Deserializer.
		/// </summary>
		/// <param name="header">header contents</param>
		explicit MessageBase(MessageHeader header)
			: header_(std::move(header))
		{
			if (header_.Name().empty())
			{
				throw std::runtime_error(
					"Invalid command Name attribute is set for the command or response structure in the MessageBase constructor. Type: " +
					ToString(header_.Type()));
			}
		}

		static std::string LookupMessageName(const std::type_info& type)
		{
			// Implementation depends on your attribute/registry system
			// You might use a static map or template specialization
			return GetMessageRegistry().GetName(type);
		}

		static std::string LookupMessageVersion(const std::type_info& type)
		{
			return GetMessageRegistry().GetVersion(type);
		}

		/// <summary>
		/// Virtual method for derived classes to serialize their payload
		/// </summary>
		virtual void SerializePayload(nlohmann::json& j) const = 0;

	private:
		MessageHeader header_;

		//nlohmann::json SerializeHeader() const
		//{
		//	nlohmann::json j;
		//	j["name"] = header_.Name();
		//	if (header_.RequestId().has_value())
		//		j["requestId"] = header_.RequestId().value();
		//	j["version"] = header_.Version();
		//	j["type"] = static_cast<int>(header_.Type());
		//	if (header_.Timeout().has_value())
		//		j["timeout"] = header_.Timeout().value();
		//	if (header_.Status().has_value())
		//		j["status"] = static_cast<int>(header_.Status().value());
		//	if (header_.CompletionCode().has_value())
		//		j["completionCode"] = static_cast<int>(header_.CompletionCode().value());
		//	if (header_.ErrorDescription().has_value())
		//		j["errorDescription"] = header_.ErrorDescription().value();
		//	return j;
		//}
		nlohmann::json SerializeHeader() const
		{
			nlohmann::json j;
			j["name"] = header_.Name();

			if (header_.RequestId().has_value())
				j["requestId"] = header_.RequestId().value();

			j["version"] = header_.Version();
			j["type"] = ToString(header_.Type());

			if (header_.Timeout().has_value())
				j["timeout"] = header_.Timeout().value();

			if (header_.Status().has_value())
				j["status"] = ToString(header_.Status().value());

			if (header_.CompletionCode().has_value())
				j["completionCode"] = ToString(header_.CompletionCode().value());

			if (header_.ErrorDescription().has_value() && !header_.ErrorDescription()->empty())
				j["errorDescription"] = header_.ErrorDescription().value();

			return j;
		}

		static MessageRegistry& GetMessageRegistry()
		{
			static MessageRegistry registry;
			return registry;
		}

	//protected:
	//	// Helper to register message types (call in derived class constructors)
	//	static void RegisterMessage(const std::type_info& type,
	//		const std::string& name,
	//		const std::string& version)
	//	{
	//		GetMessageRegistry().Register(type, name, version);
	//	}
	};
}
