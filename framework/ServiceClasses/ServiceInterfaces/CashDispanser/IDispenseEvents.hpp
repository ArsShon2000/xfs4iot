#pragma once

#include <boost/asio/awaitable.hpp>
#include <memory>
#include "../../../core/CashManagement/Events/NoteErrorEvent.hpp"
#include "../../../core/CashManagement/Events/InfoAvailableEvent.hpp"
#include "../../../core/Storage/Events/StorageErrorEvent.hpp"
#include "../../../core/CashDispenser/Events/DelayedDispenseEvent.hpp"
#include "../../../core/CashDispenser/Events/IncompleteDispenseEvent.hpp"

namespace XFS4IoTFramework::CashDispenser
{
    /// <summary>
    /// Interface for dispense operation events
    /// </summary>
    class IDispenseEvents
    {
    public:
        virtual ~IDispenseEvents() = default;

        /// <summary>
        /// Storage error event
        /// </summary>
        virtual boost::asio::awaitable<void> StorageErrorEvent(
            const XFS4IoT::Storage::Events::StorageErrorEventPayloadData & payload) = 0;

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

        /// <summary>
        /// Delayed dispense event
        /// </summary>
        virtual boost::asio::awaitable<void> DelayedDispenseEvent(
            const XFS4IoT::CashDispenser::Events::DelayedDispenseEventPayloadData& payload) = 0;

        /// <summary>
        /// Start dispense event (no payload)
        /// </summary>
        virtual boost::asio::awaitable<void> StartDispenseEvent() = 0;

        /// <summary>
        /// Incomplete dispense event
        /// </summary>
        //virtual boost::asio::awaitable<void> IncompleteDispenseEvent(
        //    const XFS4IoT::CashDispenser::Events::IncompleteDispenseEventPayloadData& payload) = 0;
    };

} // namespace XFS4IoTFramework::CashDispenser