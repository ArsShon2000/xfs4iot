#pragma once

#include <boost/asio/awaitable.hpp>
#include "../../../core/Storage/Events/StorageErrorEvent.hpp"
#include "../../../core/CashManagement/Events/InfoAvailableEvent.hpp"
#include "../../../core/CashManagement/Events/IncompleteRetractEvent.hpp"
#include "../../../core/CashManagement/Events/MediaDetectedEvent.hpp"

namespace XFS4IoTFramework::CashManagement
{
    /// <summary>
    /// Interface for retract operation events
    /// </summary>
    class IResetEvents
    {
    public:
        virtual ~IResetEvents() = default;

        /// <summary>
        /// Storage error event
        /// </summary>
        virtual boost::asio::awaitable<void> StorageErrorEvent(
            std::shared_ptr<XFS4IoT::Storage::Events::StorageErrorEventPayloadData> payload) = 0;

        /// <summary>
        /// Info available event
        /// </summary>
        virtual boost::asio::awaitable<void> InfoAvailableEvent(
            std::shared_ptr<XFS4IoT::CashManagement::Events::InfoAvailableEventPayloadData> payload) = 0;

        /// <summary>
        /// Incomplete retract event
        /// </summary>
        virtual boost::asio::awaitable<void> IncompleteRetractEvent(
            std::shared_ptr<XFS4IoT::CashManagement::Events::IncompleteRetractEventPayloadData> payload) = 0;

        /*virtual boost::asio::awaitable<void> MediaDetectedEvent(
            std::shared_ptr<XFS4IoT::CashManagement::Events::MediaDetectedEventPayloadData> payload) = 0;*/
    };

} // namespace XFS4IoTFramework::CashManagement