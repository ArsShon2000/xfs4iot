#pragma once

#include "../../CashAcceptorSample.hpp"

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <stop_token>
#include <string>

namespace XFS4IoTSP::CashAcceptor::Sample
{
    class ExecuteCashIn final
    {
    public:
        ExecuteCashIn(
            std::shared_ptr<CashAcceptorSample> handler,
            std::shared_ptr<XFS4IoTFramework::CashAcceptor::CashInCommandEvents> events,
            const XFS4IoTFramework::CashAcceptor::CashInRequest& request,
            std::stop_token cancellation);

        boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashInResult> Execute();

        void InterruptRequest();

    private:
        using CompletionCodeEnum = XFS4IoT::MessageHeader::CompletionCodeEnum;
        using CashInErrorCodeEnum =
            XFS4IoT::CashAcceptor::Completions::CashInCompletionPayloadData::ErrorCodeEnum;
        using RefusedReasonEnum =
            XFS4IoTFramework::CashAcceptor::CashInCommonCommandEvents::RefusedReasonEnum;

        CompletionCodeEnum ValidateProcessingConditions(
            std::optional<CashInErrorCodeEnum>& errorCode,
            std::string& errorDescription) const;

        boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashInResult> CompleteSuccess();

        boost::asio::awaitable<void> Refuse(RefusedReasonEnum reason);
        boost::asio::awaitable<void> SendStorageError(XFS4IoTFramework::Storage::FailureEnum failure);

        void SubscribeForDeviceEvents(std::shared_ptr<StateMachine::BlockedWaitTermination> terminator);
        void AddAcceptedBanknote(uint16_t noteId);
        void AddUnrecognizedBanknote();
        std::optional<std::string> CashItemIdByNoteId(uint16_t noteId) const;
        std::chrono::milliseconds CashInTimeout() const;
        void UpdateInputPositionStatus(bool accepted);

    private:
        std::shared_ptr<CashAcceptorSample> handler_;
        std::shared_ptr<XFS4IoTFramework::CashAcceptor::CashInCommandEvents> events_;
        XFS4IoTFramework::CashAcceptor::CashInRequest request_;
        std::stop_token cancellation_;

        SubscriptionGuard guard_;
        std::weak_ptr<StateMachine::BlockedWaitTermination> asyncTerminator_;

        std::mutex stateMutex_;
        uint16_t processedNoteId_{ 0 };
        int unrecognized_{ 0 };
        bool accepted_{ false };
        bool refused_{ false };
    };
}
