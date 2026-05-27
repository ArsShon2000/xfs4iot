#include "ClientConnection.hpp"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;

namespace XFS4IoTServer
{
    ClientConnection::ClientConnection(
        std::shared_ptr<websocket::stream<beast::tcp_stream>> socket,
        std::shared_ptr<XFS4IoT::IMessageDecoder> commandDecoder,
        std::shared_ptr<ICommandDispatcher> commandDispatcher,
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<IJsonSchemaValidator> jsonSchemaValidator)
        : socket_(std::move(socket))
        , commandDecoder_(std::move(commandDecoder))
        , commandDispatcher_(std::move(commandDispatcher))
        , logger_(std::move(logger))
        , jsonSchemaValidator_(std::move(jsonSchemaValidator))
    {
    }

    boost::asio::awaitable<void> ClientConnection::RunAsync(std::stop_token token)
    {
        try {
            //std::vector<uint8_t> receivedBuffer(MAX_BUFFER);
            beast::flat_buffer buffer;

            while (!token.stop_requested()) {
				// проверяем, что соединение всё ещё открыто перед каждой итерацией, так как оно могло быть закрыто извне (например, при отключении клиента)
                if (!socket_->is_open()) {
                    break;
                }

                //buffer.clear();
                buffer.consume(buffer.size());

				// Асинхронно читаем сообщение от клиента
                boost::system::error_code ec;
                co_await socket_->async_read(buffer,
                    net::redirect_error(net::use_awaitable, ec));

                if (ec)
                {
                    if (ec == websocket::error::closed ||
                        ec == net::error::eof ||
                        ec == net::error::connection_reset ||
                        ec == net::error::operation_aborted ||
                        ec == beast::http::error::end_of_stream)
                    {
                        logger_->trace(std::format(
                            "{}() - WebSocket соединение закрыто клиентом: code={}, category={}",
                            __FUNCTION__,
                            ec.value(),
                            ec.category().name()));
                        break;
                    }

                    logger_->warn(std::format(
                        "{}() - WebSocket error code={}, category={}",
                        __FUNCTION__,
                        ec.value(),
                        ec.category().name()));

                    break;
                }

				// Проверяем, что сообщение не превышает максимальный размер буфера
                if (socket_->got_text()) {
					// Конвертируем буфер в строку для обработки текстового сообщения
                    std::string message = beast::buffers_to_string(buffer.data());

                    if (message.size() > MAX_BUFFER) {
                        throw std::runtime_error(
                            std::format("Не удалось получить сообщение в пределах MAX_BUFFER. {}",
                                MAX_BUFFER));
                    }

                    co_await HandleIncomingMessage(message, token);
                }
                else if (socket_->got_binary()) {
					// Для бинарных сообщений, если они поддерживаются, можно обработать их аналогично, 
                    // например, конвертируя в строку или 
                    // передавая в другой метод для обработки бинарных данных. 
                    std::string message = beast::buffers_to_string(buffer.data());
                    co_await HandleIncomingMessage(message, token);
                }
            }
        }
        catch (const boost::system::system_error& ex) {
            if (ex.code() == websocket::error::closed ||
                ex.code() == net::error::eof) {
                logger_->warn(std::format("{}() - Server: Клиент закрыл соединение", __FUNCTION__));
            }
            else if (ex.code() == net::error::operation_aborted) {
                logger_->warn(std::format("{}() - Сервер: соединение прервано", __FUNCTION__));
            }
            else if (ex.code() == net::error::connection_reset)
            {
                logger_->trace(std::format(
                    "{}() - Клиент сбросил соединение: {}",
                    __FUNCTION__,
                    ex.code().message()));
            }
            else
            {
                logger_->warn(std::format(
                    "{}() - WebSocket exception: {}",
                    __FUNCTION__,
                    ex.what()));
            }
        }
        catch (const std::exception& ex) {
            logger_->warn(std::format("{}() - Сервер: неожиданное исключение: {}", __FUNCTION__, ex.what()));
            throw;
        }

		// Закрываем соединение и отменяем команды, связанные с этим клиентом
        try
        {
            if (socket_ && socket_->is_open())
            {
                boost::system::error_code ec;
                socket_->close(websocket::close_code::normal, ec);
            }

            if (commandDispatcher_)
            {
                co_await commandDispatcher_->CancelCommandsAsync(
                    shared_from_this(),
                    {},
                    std::stop_token{});
            }
        }
        catch (const std::exception& ex)
        {
            logger_->warn(std::format(
                "{}() - Исключение при cleanup соединения: {}",
                __FUNCTION__,
                ex.what()));
        }
    }

    boost::asio::awaitable<void> ClientConnection::HandleIncomingMessage(
        const std::string& messageString,
        std::stop_token token)
    {
        logger_->trace(std::format("Входящее необработанное сообщение: {}", messageString));
		// декодирование команды из JSON
        auto command = commandDecoder_->TryUnserialise(messageString);

        if (!command) {
            throw std::runtime_error(
                std::format("Получен некорректный JSON или неизвестная команда в {}.",
                    "HandleIncomingMessage"));
        }

        auto commandBase = std::dynamic_pointer_cast<XFS4IoT::MessageBase>(command);
        if (!commandBase) {
            throw std::runtime_error(
                std::format("Ошибка при десериализации полученного JSON в {}. JSON: {}",
                    "HandleIncomingMessage", messageString));
        }

        // Логирование чувствительной информации
        bool isSerialise = false;
        std::exception_ptr exSerialisePtr;
        try {
            logger_->trace(std::format("{}() - Server: Получено: {}", __FUNCTION__, commandBase->Serialise()));
        }
        catch (const InvalidDataException& ex) {
            isSerialise = true;
            exSerialisePtr = std::current_exception();
        }
        catch (const std::exception& ex) {
            throw std::runtime_error(
                std::format("Перехвачено исключение при сериализации JSON при получении входящего сообщения: {}",
                    ex.what()));
        }
        if (isSerialise)
        {
            co_await commandDispatcher_->DispatchError(
                shared_from_this(), commandBase, exSerialisePtr);
            co_return;
        }

        // Валидация сообщения, если доступен валидатор схемы JSON
        bool schemaValidationFailed = false;
        std::exception_ptr exSchemaPtr;
        try {
            if (jsonSchemaValidator_ && jsonSchemaValidator_->IsSchemaLoaded()) {
                auto failedReason = jsonSchemaValidator_->Validate(messageString);
                if (failedReason.has_value()) {
                    throw InvalidCommandException(
                        std::format("Не удалось пройти валидацию сообщения команды. {}",
                            failedReason.value()));
                }
            }
        }
        catch (const std::exception&) {
            schemaValidationFailed = true;
            exSchemaPtr = std::current_exception();
        }
        if (schemaValidationFailed)
        {
            co_await commandDispatcher_->DispatchError(
                shared_from_this(), commandBase, exSchemaPtr);
            co_return;
        }

        // Обработка команды
        bool isDispatch = false;
        std::exception_ptr exDispatchPtr;
        try {
            co_await commandDispatcher_->Dispatch(shared_from_this(), commandBase, token);
        }
        catch (const std::logic_error&) { // NotImplementedException equivalent
            isDispatch = true;
            exDispatchPtr = std::current_exception();
        }
        catch (const std::exception& ex) {
            throw std::runtime_error(
                std::format("Перехвачено исключение при обработке команды: {}", ex.what()));
        }

        if (isDispatch)
        {
            co_await commandDispatcher_->DispatchError(
                shared_from_this(), commandBase, exDispatchPtr);
        }
    }

    boost::asio::awaitable<void> ClientConnection::SendMessageAsync(
        std::shared_ptr<XFS4IoT::MessageBase> message)
    {
        if (!message) {
            throw std::invalid_argument(
                std::format("Получен объект неожиданного типа для сериализации сообщения в {}.",
                    "SendMessageAsync"));
        }

        try {
            std::string json = message->Serialise();
            logger_->trace(std::format("{}() - Server: Отправляется: {}", __FUNCTION__, json));

			// Проверяем, что соединение всё ещё открыто перед отправкой сообщения, так как оно могло быть закрыто извне (например, при отключении клиента)

                if (!socket_->is_open()) {
                    logger_->warn(std::format("{}() - Server: WebSocket не подключён", __FUNCTION__));
                    co_return;
                }
            

            boost::system::error_code ec;
            co_await socket_->async_write(
                net::buffer(json),
                net::redirect_error(net::use_awaitable, ec));

            if (ec) {
                if (ec == websocket::error::closed) {
                    logger_->warn(std::format("{}() - Server: WebSocket был закрыт", __FUNCTION__));
                }
                else if (ec == net::error::operation_aborted) {
                    logger_->warn(std::format("{}() - Server: WebSocket был прерван", __FUNCTION__));
                }
                else {
                    logger_->warn(std::format("{}() - Server: Ошибка отправки WebSocket: {}", __FUNCTION__, ec.message()));
                }
            }
        }
        catch (const InvalidDataException& ex) {
            throw std::runtime_error(
                std::format("{}() - Класс устройства установил недопустимые данные: {}. {}", __FUNCTION__,
                    message->Header().Name(), ex.what()));
        }
        catch (const std::exception& ex) {
            throw std::runtime_error(
                std::format("Перехвачено исключение при сериализации JSON при отправке сообщения: {}",
                    ex.what()));
        }
    }
}