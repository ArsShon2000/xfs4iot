#include "EndPoint.hpp"
//#include "../core/IMessageDecoder.hpp"
//#include "ICommandDispatcher.hpp"
//#include "../core/Logger/ILogger.hpp"
//#include "IConnection.hpp"
//#include "IJsonSchemaValidator.hpp"
//#include "ClientConnection.hpp"

#include "ServicePublisher.hpp"
#include <boost/url.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

namespace XFS4IoTServer
{
    EndPoint::EndPoint(boost::asio::io_context& ioContext, 
        const std::string& endpointUri,
        std::shared_ptr<XFS4IoT::IMessageDecoder> commandDecoder,
        std::shared_ptr<ICommandDispatcher> commandDispatcher,
        std::shared_ptr<ILogger> logger,
        std::weak_ptr<ServicePublisher> publisher)
        : ioContext_(ioContext)
        , commandDecoder_(std::move(commandDecoder))
        , commandDispatcher_(std::move(commandDispatcher))
        , logger_(std::move(logger))
        , endpointUri_(endpointUri)
        , publisher_(publisher)
    {
        if (endpointUri_.empty()) {
            throw std::invalid_argument(
                std::format("В конструкторе {} получен недопустимый параметр. {}",
                    "EndPoint", "endpointUri"));
        }
        if (!commandDecoder_) {
            throw std::invalid_argument(
                std::format("В конструкторе {} получен недопустимый параметр. {}",
                    "EndPoint", "commandDecoder"));
        }
        if (!commandDispatcher_) {
            throw std::invalid_argument(
                std::format("В конструкторе {} получен недопустимый параметр. {}",
                    "EndPoint", "commandDispatcher"));
        }
        if (!logger_) {
            throw std::invalid_argument(
                std::format("В конструкторе {} получен недопустимый параметр. {}",
                    "EndPoint", "logger"));
        }

        // Parse URI to extract host and port
        boost::system::result<boost::urls::url_view> result =
            boost::urls::parse_uri(endpointUri_);

        if (!result) {
            throw std::invalid_argument("Invalid endpoint URI");
        }

        auto url = result.value();
        std::string host = std::string(url.host());
        port_ = url.port_number();

        if (port_ == 0) {
            port_ = url.scheme() == "https" || url.scheme() == "wss" ? 443 : 80;
        }

        // Create acceptor
        std::string bindHost = host;
        if (bindHost.empty()) {
            bindHost = "0.0.0.0";
        }
        else if (bindHost == "localhost") {
            bindHost = "127.0.0.1";
        }

        tcp::endpoint endpoint(net::ip::make_address(bindHost), port_);
        acceptor_ = std::make_unique<tcp::acceptor>(ioContext_, endpoint);

        logger_->trace(std::format("{}() - Server: Новый endpoint {}", __FUNCTION__, endpointUri_));
    }

    EndPoint::~EndPoint()
    {
        if (acceptor_ && acceptor_->is_open()) {
            boost::system::error_code ec;
            acceptor_->close(ec);
        }
    }

    boost::asio::awaitable<void> EndPoint::RunAsync(std::stop_token token)
    {
        logger_->trace(std::format("{}() Сервер: {} ожидает (прослушивает) новые подключения", __FUNCTION__, endpointUri_));

        while (!token.stop_requested()) {
            try {
                // Accept new connection
                tcp::socket socket(acceptor_->get_executor());

                // Wait for connection with cancellation support
                boost::system::error_code ec;
                co_await acceptor_->async_accept(socket, net::redirect_error(net::use_awaitable, ec));

                if (ec) {
                    if (ec == net::error::operation_aborted && token.stop_requested()) {
                        break;
                    }
                    logger_->error(std::format("{}() - Сервер: ошибка принятия соединения: {}", __FUNCTION__, ec.message()));
                    continue;
                }

                logger_->trace(std::format("{}() - Server: {} принял новое соединение, всего соединений: {}",
                        __FUNCTION__, endpointUri_, connectionDetails_.size() + 1));
                logger_->trace(std::format(
                    "{}() - принято TCP-соединение от {}:{}",
                    __FUNCTION__,
                    socket.remote_endpoint().address().to_string(),
                    socket.remote_endpoint().port()));

                // Handle connection in separate coroutine
                auto executor = co_await net::this_coro::executor;
                beast::tcp_stream stream(std::move(socket));

                co_spawn(executor,
                    HandleConnection(std::move(stream), token),
                    net::detached);

            }
            catch (const std::exception& ex) {
                logger_->error(std::format("{}() - Сервер: ошибка в RunAsync: {}", __FUNCTION__, ex.what()));

                if (token.stop_requested()) {
                    break;
                }
            }
        }

        // Close acceptor
        if (acceptor_->is_open()) {
            boost::system::error_code ec;
            acceptor_->close(ec);
        }

        logger_->trace(std::format("{}() - Server: EndPoint остановлен", __FUNCTION__));
    }

    boost::asio::awaitable<void> EndPoint::AcceptConnections(std::stop_token token)
    {
        return boost::asio::awaitable<void>();
    }

    boost::asio::awaitable<void> EndPoint::HandleConnection(
        beast::tcp_stream stream,
        std::stop_token token)
    {
        try {
            selectedServiceProvider = nullptr;
            // Read HTTP request
            beast::flat_buffer buffer;
            http::request<http::string_body> req;

            buffer.consume(buffer.size()); 
            logger_->trace("HandleConnection: ожидание HTTP-запроса на обновление соединения");
            co_await http::async_read(stream, buffer, req, net::use_awaitable);
            logger_->trace(std::format("HandleConnection: HTTP путь = {}", std::string(req.target())));


            std::string target = std::string(req.target());
            logger_->trace(std::format("Получен запрос на обновление соединения для пути={}", target));

            // Check if it's a WebSocket upgrade request
            if (websocket::is_upgrade(req)) {
                // Create WebSocket from stream
                auto ws = std::make_shared<websocket::stream<beast::tcp_stream>>(
                    std::move(stream));

                // Accept the WebSocket handshake
                co_await ws->async_accept(req, net::use_awaitable);
                logger_->trace(std::format(
                    "{}() - WebSocket-соединение установлено для пути {}",
                    __FUNCTION__,
                    target));

                // ============================================
                // 🔥 ROUTING
                // ============================================

                std::shared_ptr<ICommandDispatcher> dispatcher = commandDispatcher_;
                std::shared_ptr<XFS4IoT::IMessageDecoder> decoder = commandDecoder_;

                auto normalize = [](std::string path) {
                    if (!path.empty() && path.back() != '/')
                        path += '/';
                    return path;
                    };

                std::string normalizedTarget = normalize(target);
                logger_->trace(std::format(
                    "{}() - нормализованный путь = {}",
                    __FUNCTION__,
                    normalizedTarget));
                // если не publisher endpoint
                if (normalizedTarget != "/xfs4iot/v1.0/") {

                    if (auto publisher = publisher_.lock())
                    {
                        auto service = publisher->FindServiceByUri(normalizedTarget);

                        if (service) {
                            logger_->trace(std::format(
                                "EndPoint::HandleConnection: выбран указатель сервиса = {}",
                                static_cast<const void*>(service.get())));

                            dispatcher = std::static_pointer_cast<ICommandDispatcher>(service);

                            logger_->trace(std::format(
                                "EndPoint::HandleConnection: указатель диспетчера = {}",
                                static_cast<const void*>(dispatcher.get())));

                            decoder = service->GetMessageDecoder();

                            logger_->trace(std::format(
                                "Выбран диспетчер = {}, типы декодеров = {}",
                                service ? "service" : "publisher",
                                decoder ? decoder->GetSupportedMessageTypes().size() : 0));

                            selectedServiceProvider =
                                std::dynamic_pointer_cast<ServiceProvider>(service);
                        }
                        else {
                            logger_->warn(std::format(
                                "{}() -Неизвестный URI сервиса: {}",
								__FUNCTION__, normalizedTarget));
						}
						logger_->trace(std::format(
							"{}() - dispatcher = {}",
							__FUNCTION__,
							(normalizedTarget == "/xfs4iot/v1.0/" ? "publisher" : "service/publisher fallback")));
					}
					else
					{
                        logger_->trace(std::format("publisher_ уничтожен"));
                    }
                }

                // Create client connection
                auto clientConnection = std::make_shared<ClientConnection>(
                    ws,
                    decoder,
                    dispatcher,
                    logger_,
                    jsonSchemaValidator_
                );

                if (selectedServiceProvider)
                {
                    selectedServiceProvider->AddConnection(clientConnection);
                }

                // Add to connection list
                {
                    std::lock_guard<std::mutex> lock(connectionsMutex_);
                    connectionDetails_.push_back(ConnectionDetails{
                        .timer = nullptr,
                        .connection = clientConnection
                        });
                }

              /*  {
                    std::lock_guard<std::mutex> lock(connectionsMutex_);
                    connectionDetails_.push_back({
                        .timer = nullptr,
                        .connection = clientConnection
                        });
                }*/

                // Run the client connection
                co_await clientConnection->RunAsync(token);

                // Remove from connection list when done
                {
                    std::lock_guard<std::mutex> lock(connectionsMutex_);
                    connectionDetails_.erase(
                        std::remove_if(connectionDetails_.begin(),
                            connectionDetails_.end(),
                            [&clientConnection](const ConnectionDetails& cd) {
                                return cd.connection == clientConnection;
                            }),
                        connectionDetails_.end()
                    );

                    if (selectedServiceProvider)
                    {
                        selectedServiceProvider->RemoveConnection(clientConnection);
                    }
                }

                logger_->trace(std::format("{}() - Сервер: соединение закрыто, осталось: {}",
                        __FUNCTION__, connectionDetails_.size()));
            }
            else {
                // Return 400 Bad Request for non-WebSocket requests
                http::response<http::string_body> res{ http::status::bad_request, req.version() };
                res.set(http::field::server, "XFS4IoT Server");
                res.set(http::field::content_type, "text/plain");
                res.body() = "WebSocket connection required";
                res.prepare_payload();

                co_await http::async_write(stream, res, net::use_awaitable);

                boost::system::error_code ec;
                stream.socket().shutdown(tcp::socket::shutdown_send, ec);
            }
        }
        catch (const boost::system::system_error& ex)
        {
            if (ex.code() == websocket::error::closed ||
                ex.code() == net::error::eof ||
                ex.code() == net::error::connection_reset ||
                ex.code() == net::error::operation_aborted ||
                ex.code() == beast::http::error::end_of_stream)
            {
                logger_->trace(std::format(
                    "{}() - Соединение завершено клиентом. code={}",
                    __FUNCTION__,
                    ex.code().value()));
            }
            else
            {
                std::string msg = ex.what();
                logger_->error(std::format(
                    "{}() - Сервер: ошибка при обработке соединения: {}",
                    __FUNCTION__, msg));
            }
        }
        catch (const std::exception& ex) {
            logger_->error(std::format("{}() - Сервер: ошибка при обработке соединения: {}", __FUNCTION__, ex.what()));
        }
    }

    void EndPoint::SetJsonSchemaValidator(std::shared_ptr<IJsonSchemaValidator> validator)
    {
        jsonSchemaValidator_ = std::move(validator);
    }

    std::vector<std::shared_ptr<IConnection>> EndPoint::GetConnections() const
    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        std::vector<std::shared_ptr<IConnection>> connections;
        connections.reserve(connectionDetails_.size());

        for (const auto& detail : connectionDetails_) {
            connections.push_back(detail.connection);
        }

        return connections;
    }
}


//
//
//#include "EndPoint.hpp"
////#include "../core/IMessageDecoder.hpp"
////#include "ICommandDispatcher.hpp"
////#include "../core/Logger/ILogger.hpp"
////#include "IConnection.hpp"
////#include "IJsonSchemaValidator.hpp"
////#include "ClientConnection.hpp"
//
//#include "ServicePublisher.hpp"
//#include <boost/url.hpp>
//
//namespace beast = boost::beast;
//namespace http = beast::http;
//namespace websocket = beast::websocket;
//namespace net = boost::asio;
//using tcp = boost::asio::ip::tcp;
//
//namespace XFS4IoTServer
//{
//    EndPoint::EndPoint(boost::asio::io_context& ioContext,
//        const std::string& endpointUri,
//        std::shared_ptr<XFS4IoT::IMessageDecoder> commandDecoder,
//        std::shared_ptr<ICommandDispatcher> commandDispatcher,
//        std::shared_ptr<ILogger> logger,
//        std::weak_ptr<ServicePublisher> publisher)
//        : ioContext_(ioContext)
//        , commandDecoder_(std::move(commandDecoder))
//        , commandDispatcher_(std::move(commandDispatcher))
//        , logger_(std::move(logger))
//        , endpointUri_(endpointUri)
//        , publisher_(publisher)
//    {
//        if (endpointUri_.empty()) {
//            throw std::invalid_argument(
//                std::format("В конструкторе {} получен недопустимый параметр. {}",
//                    "EndPoint", "endpointUri"));
//        }
//        if (!commandDecoder_) {
//            throw std::invalid_argument(
//                std::format("В конструкторе {} получен недопустимый параметр. {}",
//                    "EndPoint", "commandDecoder"));
//        }
//        if (!commandDispatcher_) {
//            throw std::invalid_argument(
//                std::format("В конструкторе {} получен недопустимый параметр. {}",
//                    "EndPoint", "commandDispatcher"));
//        }
//        if (!logger_) {
//            throw std::invalid_argument(
//                std::format("В конструкторе {} получен недопустимый параметр. {}",
//                    "EndPoint", "logger"));
//        }
//
//        // Parse URI to extract host and port
//        boost::system::result<boost::urls::url_view> result =
//            boost::urls::parse_uri(endpointUri_);
//
//        if (!result) {
//            throw std::invalid_argument("Invalid endpoint URI");
//        }
//
//        auto url = result.value();
//        std::string host = std::string(url.host());
//        port_ = url.port_number();
//
//        if (port_ == 0) {
//            port_ = url.scheme() == "https" || url.scheme() == "wss" ? 443 : 80;
//        }
//
//        // Create acceptor
//        std::string bindHost = host;
//        if (bindHost.empty()) {
//            bindHost = "0.0.0.0";
//        }
//        else if (bindHost == "localhost") {
//            bindHost = "127.0.0.1";
//        }
//
//        tcp::endpoint endpoint(net::ip::make_address(bindHost), port_);
//        acceptor_ = std::make_unique<tcp::acceptor>(ioContext_, endpoint);
//
//        logger_->trace(std::format("{}() - Server: New endpoint at {}", __FUNCTION__, endpointUri_));
//    }
//
//    EndPoint::~EndPoint()
//    {
//        if (acceptor_ && acceptor_->is_open()) {
//            boost::system::error_code ec;
//            acceptor_->close(ec);
//        }
//        ioContext_.stop();
//    }
//
//    boost::asio::awaitable<void> EndPoint::RunAsync(std::stop_token token)
//    {
//        logger_->trace(std::format("{}() - Server: {} listening for new connections", __FUNCTION__, endpointUri_));
//
//        while (!token.stop_requested()) {
//            try {
//                // Accept new connection
//                tcp::socket socket(acceptor_->get_executor());
//
//                // Wait for connection with cancellation support
//                boost::system::error_code ec;
//                co_await acceptor_->async_accept(socket, net::redirect_error(net::use_awaitable, ec));
//
//                if (ec) {
//                    if (ec == net::error::operation_aborted && token.stop_requested()) {
//                        break;
//                    }
//                    logger_->error(std::format("{}() - Server: Accept error: {}", __FUNCTION__, ec.message()));
//                    continue;
//                }
//
//                logger_->trace(std::format("{}() - Server: {} accepted new connection, total connections: {}",
//                    __FUNCTION__, endpointUri_, connectionDetails_.size() + 1));
//
//                // Handle connection in separate coroutine
//                auto executor = co_await net::this_coro::executor;
//                beast::tcp_stream stream(std::move(socket));
//
//                co_spawn(executor,
//                    HandleConnection(std::move(stream), token),
//                    net::detached);
//
//            }
//            catch (const std::exception& ex) {
//                logger_->error(std::format("{}() - Server: Error in RunAsync: {}", __FUNCTION__, ex.what()));
//
//                if (token.stop_requested()) {
//                    break;
//                }
//            }
//        }
//
//        // Close acceptor
//        if (acceptor_->is_open()) {
//            boost::system::error_code ec;
//            acceptor_->close(ec);
//        }
//
//        logger_->trace(std::format("{}() - Server: EndPoint stopped", __FUNCTION__));
//    }
//
//    boost::asio::awaitable<void> EndPoint::AcceptConnections(std::stop_token token)
//    {
//        return boost::asio::awaitable<void>();
//    }
//
//    boost::asio::awaitable<void> EndPoint::HandleConnection(
//        beast::tcp_stream stream,
//        std::stop_token token)
//    {
//        try {
//            // Read HTTP request
//            beast::flat_buffer buffer;
//            http::request<http::string_body> req;
//
//            co_await http::async_read(stream, buffer, req, net::use_awaitable);
//
//            // Check if it's a WebSocket upgrade request
//            if (websocket::is_upgrade(req)) {
//                // Create WebSocket from stream
//                auto ws = std::make_shared<websocket::stream<beast::tcp_stream>>(
//                    std::move(stream));
//
//                // Accept the WebSocket handshake
//                co_await ws->async_accept(req, net::use_awaitable);
//
//                // Create client connection
//                auto clientConnection = std::make_shared<ClientConnection>(
//                    ws,
//                    commandDecoder_,
//                    commandDispatcher_,
//                    logger_,
//                    jsonSchemaValidator_
//                );
//
//                // Add to connection list
//                {
//                    std::lock_guard<std::mutex> lock(connectionsMutex_);
//                    connectionDetails_.push_back(ConnectionDetails{
//                        .timer = nullptr,
//                        .connection = clientConnection
//                        });
//                }
//
//                // Run the client connection
//                co_await clientConnection->RunAsync(token);
//
//                // Remove from connection list when done
//                {
//                    std::lock_guard<std::mutex> lock(connectionsMutex_);
//                    connectionDetails_.erase(
//                        std::remove_if(connectionDetails_.begin(),
//                            connectionDetails_.end(),
//                            [&clientConnection](const ConnectionDetails& cd) {
//                                return cd.connection == clientConnection;
//                            }),
//                        connectionDetails_.end()
//                    );
//                }
//
//                logger_->trace(std::format("{}() - Server: Connection closed, remaining: {}",
//                    __FUNCTION__, connectionDetails_.size()));
//            }
//            else {
//                // Return 400 Bad Request for non-WebSocket requests
//                http::response<http::string_body> res{ http::status::bad_request, req.version() };
//                res.set(http::field::server, "XFS4IoT Server");
//                res.set(http::field::content_type, "text/plain");
//                res.body() = "WebSocket connection required";
//                res.prepare_payload();
//
//                co_await http::async_write(stream, res, net::use_awaitable);
//
//                boost::system::error_code ec;
//                stream.socket().shutdown(tcp::socket::shutdown_send, ec);
//            }
//        }
//        catch (const std::exception& ex) {
//            logger_->error(std::format("{}() - Server: Error handling connection: {}", __FUNCTION__, ex.what()));
//        }
//    }
//
//    void EndPoint::SetJsonSchemaValidator(std::shared_ptr<IJsonSchemaValidator> validator)
//    {
//        jsonSchemaValidator_ = std::move(validator);
//    }
//
//    std::vector<std::shared_ptr<IConnection>> EndPoint::GetConnections() const
//    {
//        std::lock_guard<std::mutex> lock(connectionsMutex_);
//        std::vector<std::shared_ptr<IConnection>> connections;
//        connections.reserve(connectionDetails_.size());
//
//        for (const auto& detail : connectionDetails_) {
//            connections.push_back(detail.connection);
//        }
//
//        return connections;
//    }
//}