#pragma once

#include <memory>

#include "CashManagementEvents.hpp"
#include "../../ServiceInterfaces/CashManagement/IResetEvents.hpp"
#include "../../../core/CashManagement/Events/MediaDetectedEvent.hpp"

namespace XFS4IoTFramework::CashManagement
{
    class ResetEvents final
        : public CashManagementEvents
        , public IResetEvents
    {
    public:
        ResetEvents(
            std::shared_ptr<XFS4IoTServer::IConnection> connection,
            int requestId)
            : CashManagementEvents(std::move(connection), requestId)
        {
        }

        boost::asio::awaitable<void> StorageErrorEvent(
            std::shared_ptr<XFS4IoT::Storage::Events::StorageErrorEventPayloadData> payload) override
        {
            auto evt = std::make_shared<XFS4IoT::Storage::Events::StorageErrorEvent>(
                requestId_,
                std::move(payload));
            co_await connection_->SendMessageAsync(evt);
        }

        boost::asio::awaitable<void> InfoAvailableEvent(
            std::shared_ptr<XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData> payload) override
        {
            auto evt = std::make_shared<XFS4IoT::CashManagement::Events::InfoAvailableEvent>(
                requestId_,
                std::move(payload));
            co_await connection_->SendMessageAsync(evt);
        }

        boost::asio::awaitable<void> IncompleteRetractEvent(
            std::shared_ptr<XFS4IoT::CashManagement::Events::IncompleteRetractEventPayloadData> payload) override
        {
            auto evt = std::make_shared<XFS4IoT::CashManagement::Events::IncompleteRetractEvent>(
                requestId_,
                std::move(payload));
            co_await connection_->SendMessageAsync(evt);
        }

        //boost::asio::awaitable<void> MediaDetectedEvent(
        //    std::shared_ptr<XFS4IoT::CashManagement::Events::MediaDetectedEventPayloadData> payload) override
        //{
        //    auto event =
        //        std::make_shared<XFS4IoT::CashManagement::Events::MediaDetectedEvent>(
        //            payload);

        //    co_await connection_->SendMessageAsync(event);
        //}
    };
}