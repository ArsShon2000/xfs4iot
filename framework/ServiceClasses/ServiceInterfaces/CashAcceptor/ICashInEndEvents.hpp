#pragma once

#include <memory>
#include <boost/asio/awaitable.hpp>

#include "../../../core/Storage/Events/StorageErrorEvent.hpp"
#include "../../../core/CashManagement/Events/NoteErrorEvent.hpp"
#include "../../../core/CashManagement/Events/InfoAvailableEvent.hpp"

namespace XFS4IoTFramework::CashAcceptor
{
    class ICashInEndEvents
    {
    public:
        virtual ~ICashInEndEvents() = default;

        virtual boost::asio::awaitable<void> StorageErrorEvent(
            std::shared_ptr<XFS4IoT::Storage::Events::StorageErrorEventPayloadData> payload) = 0;

        virtual boost::asio::awaitable<void> NoteErrorEvent(
            std::shared_ptr<XFS4IoT::CashManagement::Events::NoteErrorEventPayloadData> payload) = 0;

        virtual boost::asio::awaitable<void> InfoAvailableEvent(
            std::shared_ptr<XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData> payload) = 0;
    };
}