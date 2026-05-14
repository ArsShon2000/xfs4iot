// IRejectEvents.hpp
#pragma once

#include <boost/asio/awaitable.hpp>
#include <memory>
#include "../../../core/Storage/Events/StorageErrorEvent.hpp"
#include "../../../core/CashManagement/Events/InfoAvailableEvent.hpp"

namespace XFS4IoTFramework::CashDispenser
{
    /// <summary>
    /// Interface for Reject command events
    /// </summary>
    class IRejectEvents
    {
    public:
        virtual ~IRejectEvents() = default;

        /// <summary>
        /// Storage error event
        /// Fired when a storage unit error occurs during reject operation
        /// </summary>
        virtual boost::asio::awaitable<void> StorageErrorEvent(
            const XFS4IoT::Storage::Events::StorageErrorEventPayloadData& payload) = 0;

        /// <summary>
        /// Info available event
        /// Fired when item information becomes available during reject operation
        /// </summary>
        virtual boost::asio::awaitable<void> InfoAvailableEvent(
            const XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData& payload) = 0;
    };

} // namespace XFS4IoTFramework::CashDispenser