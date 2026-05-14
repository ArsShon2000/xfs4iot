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
	/// GetBankNoteTypes completion message
	/// XFS4IoT Version: 1.0
	/// </summary>
	class GetBankNoteTypesCompletionPayloadData final : public XFS4IoT::MessagePayloadBase
	{
	public:

		/// <summary>
		/// Constructor
		/// </summary>
		explicit GetBankNoteTypesCompletionPayloadData(
			std::optional<std::unordered_map<std::string, std::shared_ptr<BankNoteClass>>> items = std::nullopt)
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

	class GetBankNoteTypesCompletion final : public XFS4IoT::Completion<GetBankNoteTypesCompletionPayloadData>
	{
	public:
		static constexpr const char* CompletionName = "CashManagement.GetBankNoteTypes";
		static constexpr const char* Version = "1.0";

		/// <summary>
		/// Constructor for GetBankNoteTypes completion
		/// </summary>
		GetBankNoteTypesCompletion(
			int requestId,
			std::shared_ptr<GetBankNoteTypesCompletionPayloadData> payload,
			MessageHeader::CompletionCodeEnum completionCode,
			const std::string& errorDescription)
			: Completion<GetBankNoteTypesCompletionPayloadData>(
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

	inline bool GetBankNoteTypesCompletion::registered_ = []()
		{
			XFS4IoT::MessageBase::RegisterMessage(
				typeid(GetBankNoteTypesCompletion),
				GetBankNoteTypesCompletion::CompletionName,
				GetBankNoteTypesCompletion::Version);
			return true;
		}();


	inline void to_json(
		nlohmann::json& j,
		const GetBankNoteTypesCompletionPayloadData& p)
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