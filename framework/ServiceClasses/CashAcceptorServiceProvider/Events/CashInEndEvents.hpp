#pragma once

#include <memory>

#include "CashAcceptorEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashInEndEvents.hpp"

namespace XFS4IoTFramework::CashAcceptor
{
    class CashInEndEvents final
        : public CashAcceptorEvents
        , public ICashInEndEvents
    {
    public:
        CashInEndEvents(
            std::shared_ptr<XFS4IoTServer::IConnection> connection,
            int requestId)
            : CashAcceptorEvents(std::move(connection), requestId)
        {
        }

        boost::asio::awaitable<void> StorageErrorEvent(
            std::shared_ptr<XFS4IoT::Storage::Events::StorageErrorEventPayloadData> payload) override
        {
            co_await connection_->SendMessageAsync(
                std::make_shared<XFS4IoT::Storage::Events::StorageErrorEvent>(
                    requestId_,
                    std::move(payload)));
        }

        boost::asio::awaitable<void> NoteErrorEvent(
            std::shared_ptr<XFS4IoT::CashManagement::Events::NoteErrorEventPayloadData> payload) override
        {
            co_await connection_->SendMessageAsync(
                std::make_shared<XFS4IoT::CashManagement::Events::NoteErrorEvent>(
                    requestId_,
                    std::move(payload)));
        }

        boost::asio::awaitable<void> InfoAvailableEvent(
            std::shared_ptr<XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData> payload) override
        {
            co_await connection_->SendMessageAsync(
                std::make_shared<XFS4IoT::CashManagement::Events::InfoAvailableEvent>(
                    requestId_,
                    std::move(payload)));
        }
    };
}