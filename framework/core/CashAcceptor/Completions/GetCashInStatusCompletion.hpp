#pragma once

#include <string>
#include <optional>
#include <memory>
#include "../../common/Completions/MessagePayload.hpp"
#include "../../../core/CashManagement/CashManagementSchemas.hpp"
#include "../../Completion.hpp"

namespace XFS4IoT::CashAcceptor::Completions
{
	class GetCashInStatusCompletionPayloadData : public XFS4IoT::MessagePayloadBase
	{
	public:
		enum class StatusEnum
		{
			Ok,
			Rollback,
			Active,
			Retract,
			Unknown,
			Reset
		};

		GetCashInStatusCompletionPayloadData(
			std::optional<StatusEnum> status = std::nullopt,
			std::optional<int> numOfRefused = std::nullopt,
			std::shared_ptr<CashManagement::StorageCashCountsClass> items = nullptr)
			: status_(status)
			, numOfRefused(numOfRefused)
			, noteNumberList_(std::move(items))
		{
		}

		std::optional<StatusEnum> GetStatus() const { return status_; }
		void SetStatus(std::optional<StatusEnum> status) { status_ = status; }

		std::optional<int> GetNumOfRefused() const { return numOfRefused; }
		void SetNumOfRefused(std::optional<int> numOfRefused) { this->numOfRefused = numOfRefused; }

		std::shared_ptr<CashManagement::StorageCashCountsClass> GetNoteNumberList() const { return noteNumberList_; }
		void SetNoteNumberList(std::shared_ptr<CashManagement::StorageCashCountsClass> noteNumberList)
		{
			noteNumberList_ = std::move(noteNumberList);
		}

	private:
		std::optional<StatusEnum> status_;
		std::optional<int> numOfRefused;
		std::shared_ptr<CashManagement::StorageCashCountsClass> noteNumberList_;
	};

	class GetCashInStatusCompletion : public XFS4IoT::Completion<GetCashInStatusCompletionPayloadData>
	{
	public:
		static constexpr const char* CompletionName = "CashAcceptor.GetCashInStatusCompletion";
		static constexpr const char* Version = "1.0";

		GetCashInStatusCompletion(
			int requestId,
			std::shared_ptr<GetCashInStatusCompletionPayloadData> payload,
			MessageHeader::CompletionCodeEnum completionCode,
			const std::string& errorDescription)
			: Completion<GetCashInStatusCompletionPayloadData>(
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

	inline bool GetCashInStatusCompletion::registered_ = []()
		{
			XFS4IoT::MessageBase::RegisterMessage(
				typeid(GetCashInStatusCompletion),
				GetCashInStatusCompletion::CompletionName,
				GetCashInStatusCompletion::Version);
			return true;
		}();


	inline std::string ToString(GetCashInStatusCompletionPayloadData::StatusEnum code)
	{
		switch (code) {
		case GetCashInStatusCompletionPayloadData::StatusEnum::Ok:
			return "ok";
		case GetCashInStatusCompletionPayloadData::StatusEnum::Rollback:
			return "rollback";
		case GetCashInStatusCompletionPayloadData::StatusEnum::Active:
			return "active";
		case GetCashInStatusCompletionPayloadData::StatusEnum::Retract:
			return "retract";
		case GetCashInStatusCompletionPayloadData::StatusEnum::Unknown:
			return "unknown";
		case GetCashInStatusCompletionPayloadData::StatusEnum::Reset:
			return "reset";
		default:
			throw std::invalid_argument("Unknown StatusEnum value");
		}
	}


	inline void to_json(
		nlohmann::json& j,
		const GetCashInStatusCompletionPayloadData& p)
	{
		j = nlohmann::json::object();

		if (p.GetStatus().has_value())
			j["status"] = ToString(p.GetStatus().value());

		if (p.GetNumOfRefused().has_value())
			j["numOfRefused"] = p.GetNumOfRefused().value();

		if (p.GetNoteNumberList())
			j["noteNumberList"] = *p.GetNoteNumberList();
	}
}
