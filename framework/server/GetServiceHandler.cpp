#include "GetServiceHandler.hpp"
#include "../core/ServicePublisher/Events/ServiceDetailEvent.hpp"
#include <format>
#include <stdexcept>

namespace XFS4IoTServer
{
    GetServiceHandler::GetServiceHandler(
        std::shared_ptr<IConnection> connection,
        std::shared_ptr<ICommandDispatcher> dispatcher,
        std::shared_ptr<ILogger> logger)
        : connection_(std::move(connection))
        , logger_(std::move(logger))
    {

        if (!connection_) {
            throw std::invalid_argument("GetServiceHandler: соединение равно null");
        }
        if (!dispatcher) {
            throw std::invalid_argument("GetServiceHandler: диспетчер равен null");
        }
        if (!logger_) {
            throw std::invalid_argument("GetServiceHandler: логгер равен null");
        }

        servicePublisher_ = std::dynamic_pointer_cast<ServicePublisher>(dispatcher);
        if (!servicePublisher_) {
            throw std::runtime_error("GetServiceHandler: диспетчер не является ServicePublisher");
        }
    }

    boost::asio::awaitable<void> GetServiceHandler::Handle(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::stop_token)
    {
        using ServiceDetailPayload =
            XFS4IoT::ServicePublisher::Events::ServiceDetailEventPayloadData;

        using ServiceDetailEvent =
            XFS4IoT::ServicePublisher::Events::ServiceDetailEvent;

        logger_->trace("GetServiceHandler::Handle запущен");

        if (!command) {
            throw std::invalid_argument("GetServiceHandler::Handle: команда равна null");
        }

        auto getServicesCommand =
            std::dynamic_pointer_cast<XFS4IoT::ServicePublisher::Commands::GetServicesCommand>(command);

        if (!getServicesCommand) {
            throw std::runtime_error("GetServiceHandler::Handle: неожиданный тип команды");
        }

        const auto& header = getServicesCommand->Header();
        if (!header.RequestId().has_value()) {
            throw std::runtime_error("GetServiceHandler::Handle: requestId отсутствует");
        }

        using PayloadData = XFS4IoT::ServicePublisher::Completions::PayloadDataGetServicesCompletion;
        using ServiceClass = XFS4IoT::ServicePublisher::ServiceClass;

        std::vector<ServiceClass> services;
        for (const auto& s : servicePublisher_->GetServices()) {
            logger_->trace(std::format("Опубликованный URI WebSocket: {}", s->GetWSUri()));
            services.emplace_back(s->GetWSUri());
        }

        // отправляем событие с деталями сервиса для каждого сервиса, предоставляемого издателем
        auto eventPayload = std::make_shared<ServiceDetailPayload>(
            std::optional<std::string>{"DORS"},
            std::optional<std::vector<ServiceClass>>{services});

        auto serviceDetailEvent =
            std::make_shared<ServiceDetailEvent>(
                header.RequestId().value(),
                eventPayload);

        logger_->trace(std::format(
            "GetServiceHandler serviceDetailEvent json: {}",
            serviceDetailEvent->Serialise()));

        co_await connection_->SendMessageAsync(serviceDetailEvent);


        //services.emplace_back(servicePublisher_->GetWSUri());
        //services.emplace_back("ws://127.0.0.1:80/xfs4iot/v1.0/");

        logger_->trace(std::format("GetServiceHandler services count: {}", services.size()));
        for (const auto& svc : services)
        {
            nlohmann::json svcJson = svc;
            logger_->trace(std::format("Service json: {}", svcJson.dump()));
        }

        auto payload = std::make_shared<PayloadData>("DORS", std::move(services));
        
        nlohmann::json payloadJson = *payload;
        logger_->trace(std::format("GetServiceHandler payload json: {}", payloadJson.dump()));

        auto completion =
            std::make_shared<XFS4IoT::ServicePublisher::Completions::GetServicesCompletion>(
                header.RequestId().value(),
                payload,
                XFS4IoT::MessageHeader::CompletionCodeEnum::Success,
                "");

        logger_->trace(std::format("GetServiceHandler выполнен json: {}", completion->Serialise()));

        co_await connection_->SendMessageAsync(completion);
    }

    boost::asio::awaitable<void> GetServiceHandler::HandleError(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::exception_ptr commandErrorException)
    {
        logger_->trace("GetServiceHandler::HandleError запущен");

        if (!command) {
            throw std::invalid_argument("GetServiceHandler::HandleError: команда равна null");
        }
        if (!commandErrorException) {
            throw std::invalid_argument("GetServiceHandler::HandleError: исключение равно null");
        }

        auto getServicesCommand =
            std::dynamic_pointer_cast<XFS4IoT::ServicePublisher::Commands::GetServicesCommand>(command);

        if (!getServicesCommand) {
            throw std::runtime_error("GetServiceHandler::HandleError: неожиданный тип команды");
        }

        const auto& header = getServicesCommand->Header();
        if (!header.RequestId().has_value()) {
            throw std::runtime_error("GetServiceHandler::HandleError: requestId отсутствует");
        }

        XFS4IoT::MessageHeader::CompletionCodeEnum errorCode =
            XFS4IoT::MessageHeader::CompletionCodeEnum::InternalError;

        std::string errorDescription = "Unknown error";

        try {
            std::rethrow_exception(commandErrorException);
        }
        catch (const InvalidDataException& ex) {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::InvalidData;
            errorDescription = ex.what();
        }
        catch (const std::exception& ex) {
            errorDescription = ex.what();
        }

        auto completion =
            std::make_shared<XFS4IoT::ServicePublisher::Completions::GetServicesCompletion>(
                header.RequestId().value(),
                nullptr,
                errorCode,
                errorDescription);

        co_await connection_->SendMessageAsync(completion);
    }
}