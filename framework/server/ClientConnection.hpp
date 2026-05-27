#pragma once

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <stop_token>
#include <format>

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include "../core/MessageBase.hpp"
#include "ICommandDispatcher.hpp"
#include "../core/Logger/ILogger.hpp"
#include "IJsonSchemaValidator.hpp"
#include "../core/IMessageDecoder.hpp"

//using namespace XFS4IoT;

namespace XFS4IoTServer
{
    class ClientConnection : public IConnection, public std::enable_shared_from_this<ClientConnection>
    {
    public:
        ClientConnection(
            std::shared_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>> socket,
            std::shared_ptr<XFS4IoT::IMessageDecoder> commandDecoder,
            std::shared_ptr<ICommandDispatcher> commandDispatcher,
            std::shared_ptr<ILogger> logger,
            std::shared_ptr<IJsonSchemaValidator> jsonSchemaValidator);

        ~ClientConnection() override = default;

        boost::asio::awaitable<void> RunAsync(std::stop_token token);

        boost::asio::awaitable<void> SendMessageAsync(
            std::shared_ptr<XFS4IoT::MessageBase> message) override;

    private:
        boost::asio::awaitable<void> HandleIncomingMessage(
            const std::string& messageString,
            std::stop_token token);

        std::shared_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>> socket_;
        std::shared_ptr<XFS4IoT::IMessageDecoder> commandDecoder_;
        std::shared_ptr<ICommandDispatcher> commandDispatcher_;
        std::shared_ptr<ILogger> logger_;
        std::shared_ptr<IJsonSchemaValidator> jsonSchemaValidator_;

        std::mutex sendMutex_;

        static constexpr size_t MAX_BUFFER = 2 * 1024 * 1024; // 2MB
    };

	// потом перенесу в отдельный файл, если понадобится
    class InvalidCommandException : public std::runtime_error
    {
    public:
        explicit InvalidCommandException(const std::string& msg)
            : std::runtime_error(msg) {}
    };

    class InvalidDataException : public std::runtime_error
    {
    public:
        explicit InvalidDataException(const std::string& msg)
            : std::runtime_error(msg) {}
    };
}