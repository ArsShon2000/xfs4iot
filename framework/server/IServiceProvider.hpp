#pragma once

#include <boost/asio/io_context.hpp>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <boost/asio/awaitable.hpp>
#include "../core/MessageBase.hpp"
#include "ICommandDispatcher.hpp"
#include "IDevice.hpp"
#include "IJsonSchemaValidator.hpp"
#include "../core/MessageTypeInfo.hpp"
#include "../core/IMessageDecoder.hpp"

//// Forward declarations
//class IDevice;
//class IConnection;
//class MessageBase;
//class IJsonSchemaValidator;
//struct MessageTypeInfo;

namespace XFS4IoTServer
{
    /// <summary>
    /// Service provider interface that extends command dispatcher functionality
    /// </summary>
    class IServiceProvider : public virtual ICommandDispatcher
        //, public std::enable_shared_from_this<IServiceProvider>
    {
    public:
        ~IServiceProvider() override = default;

        /// <summary>
        /// Get the service name
        /// </summary>
        virtual std::string GetName() const = 0;

        /// <summary>
        /// Get the service URI
        /// </summary>
        virtual std::string GetUri() const = 0;

        /// <summary>
        /// Get the WebSocket service URI
        /// </summary>
        virtual std::string GetWSUri() const = 0;

        /// <summary>
        /// The device class that integrates this service with real hardware.
        /// </summary>
        virtual std::shared_ptr<IDevice> GetDevice() const = 0;

        /// <summary>
        /// Broadcast an unsolicited event to all connections.
        /// </summary>
        /// <param name="payload">The XFS payload for the message</param>
        virtual boost::asio::awaitable<void> BroadcastEvent(
            std::shared_ptr<XFS4IoT::MessageBase> payload) = 0;

        /// <summary>
        /// Broadcast an unsolicited event to specified connections.
        /// </summary>
        /// <param name="connections">The connections to broadcast to</param>
        /// <param name="payload">The XFS payload for the message</param>
        virtual boost::asio::awaitable<void> BroadcastEvent(
            const std::vector<std::shared_ptr<IConnection>>& connections,
            std::shared_ptr<XFS4IoT::MessageBase> payload) = 0;

        /// <summary>
        /// Set JSON schema dependency injection to the services.
        /// </summary>
        virtual void SetJsonSchemaValidator(
            std::shared_ptr<IJsonSchemaValidator> jsonSchemaValidator) = 0;

        /// <summary>
        /// The list of commands and events device class supported and associated with versions.
        /// The framework only supports one latest version of commands and events.
        /// </summary>
        virtual void SetMessagesSupported(
            const std::map<std::string, XFS4IoT::MessageTypeInfo>& messagesSupported) = 0;

        /// <summary>
        /// Get the list of supported messages
        /// </summary>
        virtual std::map<std::string, XFS4IoT::MessageTypeInfo> GetMessagesSupported() const = 0;

        /// <summary>
        /// Получить io_context для асинхронных операций
        /// </summary>
        virtual boost::asio::io_context& getIoContext() = 0;

		/// <summary>
		/// Get the message decoder for this service provider
		/// </summary>
        virtual std::shared_ptr<XFS4IoT::IMessageDecoder> GetMessageDecoder() const = 0;
    };
}