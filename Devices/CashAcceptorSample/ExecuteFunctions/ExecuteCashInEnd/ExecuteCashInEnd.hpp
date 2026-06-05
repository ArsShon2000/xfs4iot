#pragma once

#include "../../CashAcceptorSample.hpp"

#include <memory>
#include <optional>
#include <stop_token>
#include <string>
#include <unordered_map>

namespace XFS4IoTSP::CashAcceptor::Sample
{
    class ExecuteCashInEnd final
    {
    public:
        ExecuteCashInEnd(
            std::shared_ptr<CashAcceptorSample> handler,
            std::shared_ptr<XFS4IoTFramework::CashAcceptor::CashInEndCommandEvents> events,
            std::stop_token cancellation);

        boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashInEndResult> Execute();

        void InterruptRequest();

    private:
        using CompletionCodeEnum = XFS4IoT::MessageHeader::CompletionCodeEnum;
        using CashInEndErrorCodeEnum =
            XFS4IoT::CashAcceptor::Completions::CashInEndCompletionPayloadData::ErrorCodeEnum;

        CompletionCodeEnum ValidateProcessingConditions(
            std::optional<CashInEndErrorCodeEnum>& errorCode,
            std::string& errorDescription) const;

        std::unordered_map<std::string, std::shared_ptr<XFS4IoTFramework::Storage::CashUnitCountClass>>
            BuildMovementAndCommit();

        XFS4IoTFramework::Storage::StorageCashCountClass AcceptedStorageCount() const;
        nlohmann::json AcceptedNotesByNoteId() const;
        std::string CashInStorageId() const;
        void UpdateStatusesAfterSuccess();

    private:
        std::shared_ptr<CashAcceptorSample> handler_;
        std::shared_ptr<XFS4IoTFramework::CashAcceptor::CashInEndCommandEvents> events_;
        std::stop_token cancellation_;

        std::weak_ptr<StateMachine::BlockedWaitTermination> asyncTerminator_;
    };
}
