#pragma once

#include <memory>
#include <string>
#include <vector>
#include <stop_token>
#include <algorithm>
#include <format>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include "../core/IMessageDecoder.hpp"
#include "../core/Logger/ILogger.hpp"
#include "ICommandDispatcher.hpp"
#include "IConnection.hpp"
#include "IJsonSchemaValidator.hpp"
#include "ClientConnection.hpp"
#include "ServiceProvider.hpp"

//// Forward declarations
//class ICommandDispatcher;
//class ILogger;
//class IConnection;
//class IJsonSchemaValidator;
//class ClientConnection;

namespace XFS4IoTServer
{

    class ServicePublisher;

    /// <summary>
    /// Endpoint that will receive and process commands from a client
    /// </summary>
    class EndPoint
    {
    public:
        EndPoint(boost::asio::io_context& ioContext,
            const std::string& endpointUri,
            std::shared_ptr<XFS4IoT::IMessageDecoder> commandDecoder,
            std::shared_ptr<ICommandDispatcher> commandDispatcher,
            std::shared_ptr<ILogger> logger,
            std::weak_ptr<ServicePublisher> publisher);

        ~EndPoint();

        // Non-copyable, non-movable
        EndPoint(const EndPoint&) = delete;
        EndPoint& operator=(const EndPoint&) = delete;
        EndPoint(EndPoint&&) = delete;
        EndPoint& operator=(EndPoint&&) = delete;

        boost::asio::awaitable<void> RunAsync(std::stop_token token);

        void SetJsonSchemaValidator(std::shared_ptr<IJsonSchemaValidator> validator);

        std::vector<std::shared_ptr<IConnection>> GetConnections() const;

        // Provide access to internal io_context (needed for IServiceProvider implementations)
        boost::asio::io_context& getIoContext() noexcept { 
            return ioContext_; 
        }

    private:
        boost::asio::awaitable<void> AcceptConnections(std::stop_token token);
        std::shared_ptr<ServiceProvider> selectedServiceProvider = nullptr;
        boost::asio::awaitable<void> HandleConnection(
            boost::beast::tcp_stream stream,
            std::stop_token token);

        struct ConnectionDetails
        {
            std::shared_ptr<boost::asio::steady_timer> timer;
            std::shared_ptr<IConnection> connection;
        };

        std::shared_ptr<XFS4IoT::IMessageDecoder> commandDecoder_;
        std::shared_ptr<ICommandDispatcher> commandDispatcher_;
        std::shared_ptr<ILogger> logger_;
        std::shared_ptr<IJsonSchemaValidator> jsonSchemaValidator_;
        std::weak_ptr<ServicePublisher> publisher_;

        boost::asio::io_context& ioContext_;
        std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;

        std::vector<ConnectionDetails> connectionDetails_;
        mutable std::mutex connectionsMutex_;

        std::string endpointUri_;
        unsigned short port_;
    };
}


//#pragma once
//
//#include <memory>
//#include <string>
//#include <vector>
//#include <stop_token>
//#include <algorithm>
//#include <format>
//#include <mutex>
//#include <boost/asio.hpp>
//#include <boost/asio/awaitable.hpp>
//#include <boost/asio/co_spawn.hpp>
//#include <boost/asio/detached.hpp>
//#include <boost/beast.hpp>
//#include <boost/beast/websocket.hpp>
//#include "../core/IMessageDecoder.hpp"
//#include "../core/Logger/ILogger.hpp"
//#include "ICommandDispatcher.hpp"
//#include "IConnection.hpp"
//#include "IJsonSchemaValidator.hpp"
//#include "ClientConnection.hpp"
//
////// Forward declarations
////class IMessageDecoder;
////class ICommandDispatcher;
////class ILogger;
////class IConnection;
////class IJsonSchemaValidator;
////class ClientConnection;
//
//namespace XFS4IoTServer
//{
//    /// <summary>
//    /// Endpoint that will receive and process commands from a client
//    /// </summary>
//    class EndPoint
//    {
//    public:
//        EndPoint(boost::asio::io_context& ioContext,
//            const std::string& endpointUri,
//            std::shared_ptr<XFS4IoT::IMessageDecoder> commandDecoder,
//            std::shared_ptr<ICommandDispatcher> commandDispatcher,
//            std::shared_ptr<ILogger> logger);
//
//        ~EndPoint();
//
//        // Non-copyable, non-movable
//        EndPoint(const EndPoint&) = delete;
//        EndPoint& operator=(const EndPoint&) = delete;
//        EndPoint(EndPoint&&) = delete;
//        EndPoint& operator=(EndPoint&&) = delete;
//
//        boost::asio::awaitable<void> RunAsync(std::stop_token token);
//
//        void SetJsonSchemaValidator(std::shared_ptr<IJsonSchemaValidator> validator);
//
//        std::vector<std::shared_ptr<IConnection>> GetConnections() const;
//
//        // Provide access to internal io_context (needed for IServiceProvider implementations)
//        boost::asio::io_context& getIoContext() noexcept {
//            return ioContext_;
//        }
//
//    private:
//        boost::asio::awaitable<void> AcceptConnections(std::stop_token token);
//
//        boost::asio::awaitable<void> HandleConnection(
//            boost::beast::tcp_stream stream,
//            std::stop_token token);
//
//        struct ConnectionDetails
//        {
//            std::shared_ptr<boost::asio::steady_timer> timer;
//            std::shared_ptr<IConnection> connection;
//        };
//
//        std::shared_ptr<XFS4IoT::IMessageDecoder> commandDecoder_;
//        std::shared_ptr<ICommandDispatcher> commandDispatcher_;
//        std::shared_ptr<ILogger> logger_;
//        std::shared_ptr<IJsonSchemaValidator> jsonSchemaValidator_;
//
//        boost::asio::io_context& ioContext_;
//        std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
//
//        std::vector<ConnectionDetails> connectionDetails_;
//        mutable std::mutex connectionsMutex_;
//
//        std::string endpointUri_;
//        unsigned short port_;
//    };
//}