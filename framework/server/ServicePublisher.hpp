#pragma once

#include <memory>
#include <string>
#include <vector>
#include <regex>
#include <format>

#include "CommandDispatcher.hpp"
#include "IServiceProvider.hpp"
#include "EndpointDetails.hpp"
#include "EndPoint.hpp"
#include "../core/MessageDecoder.hpp"
#include "IJsonSchemaValidator.hpp"

namespace XFS4IoTServer
{
    /// <summary>
    /// Server publisher is responsible for the service discovery 
    /// endpoint, and managing services.
    /// </summary>
    class ServicePublisher : public CommandDispatcher,
        public IServiceProvider,
        public std::enable_shared_from_this<ServicePublisher>
    {
    public:
        /// <summary>
        /// A new service publisher.
        /// </summary>
        /// <remarks>
        /// The new service publisher will automatically bind to the next available port 
        /// (as defined by XFS4IoT.)
        /// </remarks>
        ServicePublisher(boost::asio::io_context& ioContext,
            std::shared_ptr<ILogger> logger,
            std::shared_ptr<IJsonSchemaValidator> jsonSchemaValidator = nullptr);

        ~ServicePublisher() override;

        // Non-copyable, non-movable
        ServicePublisher(const ServicePublisher&) = delete;
        ServicePublisher& operator=(const ServicePublisher&) = delete;

        boost::asio::awaitable<void> RunAsync(
            std::shared_ptr<CancellationSource> cancellationSource) override;

        void Add(std::shared_ptr<IServiceProvider> service);

        boost::asio::awaitable<void> BroadcastEvent(
            std::shared_ptr<XFS4IoT::MessageBase> payload) override;

        boost::asio::awaitable<void> BroadcastEvent(
            const std::vector<std::shared_ptr<IConnection>>& connections,
            std::shared_ptr<XFS4IoT::MessageBase> payload) override;

        void SetJsonSchemaValidator(
            std::shared_ptr<IJsonSchemaValidator> jsonSchemaValidator) override;

        void SetMessagesSupported(
            const std::map<std::string, XFS4IoT::MessageTypeInfo>& messagesSupported) override;

        std::map<std::string, XFS4IoT::MessageTypeInfo> GetMessagesSupported() const override;

        void SetEventsSupported(
            const std::map<std::string, XFS4IoT::MessageTypeInfo>& eventsSupported) override {}

        std::map<std::string, XFS4IoT::MessageTypeInfo> GetEventsSupported() const override {
			return {};
        }

        // IServiceProvider implementation
        std::string GetName() const override { return name_; }
        std::string GetUri() const override { return uri_; }
        std::string GetWSUri() const override { return wsUri_; }
        std::shared_ptr<XFS4IoT::IMessageDecoder> GetMessageDecoder() const override;

        std::shared_ptr<IDevice> GetDevice() const override
        {
            throw std::logic_error(
                "A device object was requested from the Publisher service, "
                "but the publisher service does not have a device class");
        }

        const std::vector<std::shared_ptr<IServiceProvider>>& GetServices() const
        {
            return services_;
        }

        std::shared_ptr<EndpointDetails> GetEndpointDetails() const
        {
            return endpointDetails_;
        }

        // Provide io_context to satisfy IServiceProvider
        boost::asio::io_context& getIoContext() override;

        std::shared_ptr<ICommandDispatcher> GetDispatcherPtr() override;


        void Initialize();

        std::shared_ptr<IServiceProvider> FindServiceByUri(const std::string& target);

    private:
        std::shared_ptr<ILogger> logger_;
        std::vector<std::shared_ptr<IServiceProvider>> services_; 
        std::unique_ptr<EndPoint> endPoint_;
        std::shared_ptr<XFS4IoT::MessageDecoder> commandDecoder_;
        std::shared_ptr<IJsonSchemaValidator> jsonSchemaValidator_;
        std::shared_ptr<EndpointDetails> endpointDetails_;
        boost::asio::io_context& ioContext_;

        std::string name_;
        std::string uri_;
        std::string wsUri_;
    };
}