#pragma once

#include <memory>
#include <optional>
#include <string>
#include "Message.hpp"
#include "common/MessageHeader.hpp"

namespace XFS4IoT::Events
{
    /// <summary>
    /// Event message class to be derived from.
    /// </summary>
    /// <typeparam name="T">Type of payload derived from MessagePayloadBase</typeparam>
    template<typename TPayload>
    class Event : public XFS4IoT::Message<TPayload>
    {
    public:
        /// <summary>
        /// Event message class to be derived from.
        /// </summary>
        /// <param name="request_id">Request id of the command this event relates to</param>
        /// <param name="payload">Data for the event</param>
        explicit Event(
            int requestId,
            std::shared_ptr<TPayload> payload = nullptr)
            : XFS4IoT::Message<TPayload>(
                this->LookupMessageName(typeid(*this)),
                this->LookupMessageVersion(typeid(*this)),
                requestId,
                MessageHeader::TypeEnum::Event,
                std::move(payload),
                std::nullopt)
        {
        }

        /// <summary>
        /// Event message class with unique_ptr payload
        /// </summary>
        Event(
            const std::string& name,
            const std::string& version,
            int requestId,
            std::shared_ptr<TPayload> payload = nullptr)
            : XFS4IoT::Message<TPayload>(
                name,
                version,
                requestId,
                MessageHeader::TypeEnum::Event,
                std::move(payload),
                std::nullopt)
        {
        }

        virtual ~Event() = default;

    };

    /// <summary>
    /// Unsolicited event message class to be derived from.
    /// </summary>
    /// <typeparam name="T">Type of payload derived from MessagePayloadBase</typeparam>
    template<typename TPayload>
    class UnsolicitedEvent : public XFS4IoT::Message<TPayload>
    {
    public:
        /// <summary>
        /// Unsolicited event message class to be derived from.
        /// </summary>
        /// <param name="payload">Data for the event</param>
        explicit UnsolicitedEvent(
            std::shared_ptr<TPayload> payload = nullptr)
            : XFS4IoT::Message<TPayload>(
                this->LookupMessageName(typeid(*this)),
                this->LookupMessageVersion(typeid(*this)),
                std::nullopt,
                MessageHeader::TypeEnum::Event,
                std::move(payload),
                std::nullopt)
        {
        }

        /// <summary>
        /// Unsolicited event message with unique_ptr payload
        /// </summary>
        UnsolicitedEvent(
            const std::string& name,
            const std::string& version,
            std::shared_ptr<TPayload> payload = nullptr)
            : XFS4IoT::Message<TPayload>(
                name,
                version,
                std::nullopt,
                MessageHeader::TypeEnum::Event,
                std::move(payload),
                std::nullopt)
        {
        }

        virtual ~UnsolicitedEvent() = default;
    };

} // namespace XFS4IoT::Events