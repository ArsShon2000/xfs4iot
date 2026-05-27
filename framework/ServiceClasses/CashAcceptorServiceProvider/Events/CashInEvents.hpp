#pragma once

#include <memory>

#include "CashAcceptorEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashInEvents.hpp"
#include "../../../core/CashAcceptor/Events/InsertItemsEvent.hpp"

namespace XFS4IoTFramework::CashAcceptor
{
    class CashInEvents final
        : public CashAcceptorEvents
        , public ICashInEvents
    {
    public:
        CashInEvents(
            std::shared_ptr<XFS4IoTServer::IConnection> connection,
            int requestId)
            : CashAcceptorEvents(std::move(connection), requestId)
        {
        }

        boost::asio::awaitable<void> StorageErrorEvent(
            const XFS4IoT::Storage::Events::StorageErrorEventPayloadData& payload) override
        {
            auto evt = std::make_shared<XFS4IoT::Storage::Events::StorageErrorEvent>(
                requestId_,
                std::make_shared<XFS4IoT::Storage::Events::StorageErrorEventPayloadData>(payload));

            co_await connection_->SendMessageAsync(evt);
        }

        boost::asio::awaitable<void> NoteErrorEvent(
            const XFS4IoT::CashManagement::Events::NoteErrorEventPayloadData& payload) override
        {
            auto evt = std::make_shared<XFS4IoT::CashManagement::Events::NoteErrorEvent>(
                requestId_,
                std::make_shared<XFS4IoT::CashManagement::Events::NoteErrorEventPayloadData>(payload));

            co_await connection_->SendMessageAsync(evt);
        }

        boost::asio::awaitable<void> InfoAvailableEvent(
            const XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData& payload) override
        {
            auto evt = std::make_shared<XFS4IoT::CashManagement::Events::InfoAvailableEvent>(
                requestId_,
                std::make_shared<XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData>(payload));

            co_await connection_->SendMessageAsync(evt);
        }

        boost::asio::awaitable<void> InputRefuseEvent(
            const XFS4IoT::CashAcceptor::Events::InputRefuseEventPayloadData& payload) override
        {
            auto evt = std::make_shared<XFS4IoT::CashAcceptor::Events::InputRefuseEvent>(
                requestId_,
                std::make_shared<XFS4IoT::CashAcceptor::Events::InputRefuseEventPayloadData>(payload));

            co_await connection_->SendMessageAsync(evt);
        }

        boost::asio::awaitable<void> SubCashInEvent(
            const XFS4IoT::CashAcceptor::Events::SubCashInEventPayloadData& payload) override
        {
            auto evt = std::make_shared<XFS4IoT::CashAcceptor::Events::SubCashInEvent>(
                requestId_,
                std::make_shared<XFS4IoT::CashAcceptor::Events::SubCashInEventPayloadData>(payload));

            co_await connection_->SendMessageAsync(evt);
        }

        boost::asio::awaitable<void> InsertItemsEvent() override
        {
            auto evt =
                std::make_shared<XFS4IoT::CashAcceptor::Events::InsertItemsEvent>(
                    requestId_);

            co_await connection_->SendMessageAsync(evt);
        }

        boost::asio::awaitable<void> IncompleteRetractEvent(
            const XFS4IoT::CashManagement::Events::IncompleteRetractEventPayloadData& payload) override
        {
            auto evt = std::make_shared<XFS4IoT::CashManagement::Events::IncompleteRetractEvent>(
                requestId_,
                std::make_shared<XFS4IoT::CashManagement::Events::IncompleteRetractEventPayloadData>(payload));

            co_await connection_->SendMessageAsync(evt);
        }
    };
}