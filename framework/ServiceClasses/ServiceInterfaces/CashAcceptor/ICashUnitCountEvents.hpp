#pragma once

#include <boost/asio/awaitable.hpp>
#include "../../../core/Storage/Events/StorageErrorEvent.hpp"
#include "../../../core/CashManagement/Events/NoteErrorEvent.hpp"
#include "../../../core/CashManagement/Events/InfoAvailableEvent.hpp"

namespace XFS4IoTFramework::CashManagement
{
    /// <summary>
    /// Interface for retract operation events
    /// </summary>
    class ICashUnitCountEvents
    {
    public:
        virtual ~ICashUnitCountEvents() = default;

        /// <summary>
        /// Storage error event
        /// </summary>
        virtual boost::asio::awaitable<void> StorageErrorEvent(
            const XFS4IoT::Storage::Events::StorageErrorEventPayloadData& payload) = 0;

        /// <summary>
        /// Note error event
        /// </summary>
        virtual boost::asio::awaitable<void> NoteErrorEvent(
            const XFS4IoT::CashManagement::Events::NoteErrorEventPayloadData& payload) = 0;

        /// <summary>
        /// Info available event
        /// </summary>
        virtual boost::asio::awaitable<void> InfoAvailableEvent(
            const XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData& payload) = 0;
    };

} // namespace XFS4IoTFramework::CashManagement