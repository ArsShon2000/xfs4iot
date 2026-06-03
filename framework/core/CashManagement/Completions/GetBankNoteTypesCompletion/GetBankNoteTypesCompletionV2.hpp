#pragma once

#include <string>
#include <memory>
#include <optional>
#include <unordered_map>
#include "../../../Completion.hpp"
#include "../../../common/MessageHeader.hpp"
#include "../../../common/Completions/MessagePayload.hpp"
#include "../../CashManagementSchemas.hpp"

namespace XFS4IoT::CashManagement::Completions
{
	/// <summary>
	/// GetBankNoteTypes completion message
	/// XFS4IoT Version: 2.0
	/// </summary>
	class GetBankNoteTypesCompletionV2PayloadData final : public XFS4IoT::MessagePayloadBase
	{
	public:

		/// <summary>
		/// Constructor
		/// </summary>
		explicit GetBankNoteTypesCompletionV2PayloadData(
			std::optional<std::unordered_map<std::string, std::shared_ptr<XFS4IoT::CashManagement::BankNoteClass>>> items = std::nullopt)
			: items_(std::move(items))
		{
		}


		/// <summary>
		/// Object containing the storage units which have had items inserted during the associated operation or
		/// transaction. Only storage units whose contents have been modified are included.
		/// </summary>
		const std::optional<std::unordered_map<std::string, std::shared_ptr<BankNoteClass>>>&
			getItems() const { return items_; }

		void setItems(std::optional<std::unordered_map<std::string, std::shared_ptr<BankNoteClass>>> items)
		{
			items_ = std::move(items);
		}


		//// Serialization support
		//virtual std::string toJson() const override;
		//virtual void fromJson(const std::string& json) override;

	private:
		std::optional<std::unordered_map<std::string, std::shared_ptr<BankNoteClass>>> items_;
	};

	class GetBankNoteTypesCompletionV2 final : public XFS4IoT::Completion<GetBankNoteTypesCompletionV2PayloadData>
	{
	public:
		static constexpr const char* CompletionName = "CashManagement.GetBankNoteTypes";
		static constexpr const char* Version = "2.0";

		/// <summary>
		/// Constructor for GetBankNoteTypes completion
		/// </summary>
		GetBankNoteTypesCompletionV2(
			int requestId,
			std::shared_ptr<GetBankNoteTypesCompletionV2PayloadData> payload,
			MessageHeader::CompletionCodeEnum completionCode,
			const std::string& errorDescription)
			: Completion<GetBankNoteTypesCompletionV2PayloadData>(
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

	inline bool GetBankNoteTypesCompletionV2::registered_ = []()
		{
			XFS4IoT::MessageBase::RegisterMessage(
				typeid(GetBankNoteTypesCompletionV2),
				GetBankNoteTypesCompletionV2::CompletionName,
				GetBankNoteTypesCompletionV2::Version);
			return true;
		}();


	inline void to_json(
		nlohmann::json& j,
		const GetBankNoteTypesCompletionV2PayloadData& p)
	{
		j = nlohmann::json::object();


		if (p.getItems().has_value())
		{
			j["items"] = nlohmann::json::object();

			for (const auto& [key, value] : p.getItems().value())
			{
				if (value)
				{
					j["items"][key] = *value;
				}
			}
		}
	}

}