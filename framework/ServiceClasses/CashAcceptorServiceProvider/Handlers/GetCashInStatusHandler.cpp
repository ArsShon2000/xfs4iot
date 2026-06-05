#include "GetCashInStatusHandler.hpp"

#include <stdexcept>

#include "../../../core/Exceptions.hpp"

namespace XFS4IoTFramework::CashAcceptor
{
    using GetCashInStatusCommand =
        XFS4IoT::CashAcceptor::Commands::GetCashInStatusCommand;

    using GetCashInStatusCompletion =
        XFS4IoT::CashAcceptor::Completions::GetCashInStatusCompletion;

    using GetCashInStatusCompletionPayloadData =
        XFS4IoT::CashAcceptor::Completions::GetCashInStatusCompletionPayloadData;

    GetCashInStatusHandler::GetCashInStatusHandler(
        std::shared_ptr<XFS4IoTServer::IConnection> connection,
        std::shared_ptr<XFS4IoTServer::ICommandDispatcher> dispatcher,
        std::shared_ptr<ILogger> logger)
        : connection_(std::move(connection))
        , logger_(std::move(logger))
    {
        if (!dispatcher)
            throw std::invalid_argument(
                "GetCashInStatusHandler: dispatcher cannot be null");

        if (!connection_)
            throw std::invalid_argument(
                "GetCashInStatusHandler: connection cannot be null");

        if (!logger_)
            throw std::invalid_argument(
                "GetCashInStatusHandler: logger cannot be null");

        provider_ =
            std::dynamic_pointer_cast<XFS4IoTServer::IServiceProvider>(
                dispatcher);

        if (!provider_)
            throw std::runtime_error(
                "GetCashInStatusHandler: dispatcher is not IServiceProvider");

        auto deviceBase = provider_->GetDevice();

        if (!deviceBase)
            throw std::runtime_error(
                "GetCashInStatusHandler: device cannot be null");

        device_ =
            std::dynamic_pointer_cast<ICashAcceptorDevice>(
                deviceBase);

        if (!device_)
            throw std::runtime_error(
                "GetCashInStatusHandler: device is not ICashAcceptorDevice");

        cashAcceptor_ =
            std::dynamic_pointer_cast<XFS4IoTServer::ICashAcceptorService>(
                provider_);

        if (!cashAcceptor_)
            throw std::runtime_error(
                "GetCashInStatusHandler: provider is not ICashAcceptorService");

        common_ =
            std::dynamic_pointer_cast<XFS4IoTFramework::Common::ICommonService>(
                provider_);

        if (!common_)
            throw std::runtime_error(
                "GetCashInStatusHandler: provider is not ICommonService");
    }

    boost::asio::awaitable<void>
        GetCashInStatusHandler::Handle(
            std::shared_ptr<XFS4IoT::MessageBase> command,
            std::stop_token cancel)
    {
        auto cmd =
            std::dynamic_pointer_cast<GetCashInStatusCommand>(command);

        if (!cmd)
            throw std::invalid_argument(
                "GetCashInStatusHandler::Handle invalid command");

        if (!cmd->Header().RequestId().has_value())
            throw std::runtime_error(
                "GetCashInStatusHandler::Handle requestId missing");

        auto events =
            std::make_shared<GetCashInStatusEvents>(
                connection_,
                cmd->Header().RequestId().value());

        auto result =
            co_await HandleGetCashInStatus(
                cmd,
                events,
                cancel);

        auto response =
            std::make_shared<GetCashInStatusCompletion>(
                cmd->Header().RequestId().value(),
                result.payload,
                result.completionCode,
                result.errorDescription);

        co_await connection_->SendMessageAsync(response);
    }

    boost::asio::awaitable<void>
        GetCashInStatusHandler::HandleError(
            std::shared_ptr<XFS4IoT::MessageBase> command,
            std::exception_ptr commandException)
    {
        auto cmd =
            std::dynamic_pointer_cast<GetCashInStatusCommand>(command);

        if (!cmd)
            throw std::invalid_argument(
                "GetCashInStatusHandler::HandleError invalid command");

        if (!cmd->Header().RequestId().has_value())
            throw std::runtime_error(
                "GetCashInStatusHandler::HandleError requestId missing");

        auto errorCode =
            XFS4IoT::MessageHeader::CompletionCodeEnum::InternalError;

        std::string errorDescription = "Unknown error";

        try
        {
            if (commandException)
                std::rethrow_exception(commandException);
        }
        catch (const std::exception& ex)
        {
            errorDescription = ex.what();
        }

        auto response =
            std::make_shared<GetCashInStatusCompletion>(
                cmd->Header().RequestId().value(),
                nullptr,
                errorCode,
                errorDescription);

        co_await connection_->SendMessageAsync(response);
    }

    boost::asio::awaitable<GetCashInStatusHandler::CommandResult>
        GetCashInStatusHandler::HandleGetCashInStatus(
            std::shared_ptr<GetCashInStatusCommand>,
            std::shared_ptr<IGetCashInStatusEvents>,
            std::stop_token)
    {
        auto cashInStatus =
            cashAcceptor_->GetCashInStatus();

        if (!cashInStatus)
            throw std::runtime_error(
                "CashAcceptor::CashInStatus is not set");

        GetCashInStatusCompletionPayloadData::StatusEnum status =
            GetCashInStatusCompletionPayloadData::StatusEnum::Unknown;

        switch (cashInStatus->GetStatus())
        {
        case XFS4IoTFramework::CashManagement::CashInStatusClass::StatusEnum::Active:
            status = GetCashInStatusCompletionPayloadData::StatusEnum::Active;
            break;

        case XFS4IoTFramework::CashManagement::CashInStatusClass::StatusEnum::Ok:
            status = GetCashInStatusCompletionPayloadData::StatusEnum::Ok;
            break;

        case XFS4IoTFramework::CashManagement::CashInStatusClass::StatusEnum::Reset:
            status = GetCashInStatusCompletionPayloadData::StatusEnum::Reset;
            break;

        case XFS4IoTFramework::CashManagement::CashInStatusClass::StatusEnum::Retract:
            status = GetCashInStatusCompletionPayloadData::StatusEnum::Retract;
            break;

        case XFS4IoTFramework::CashManagement::CashInStatusClass::StatusEnum::Rollback:
            status = GetCashInStatusCompletionPayloadData::StatusEnum::Rollback;
            break;

        default:
            break;
        }

        auto payload =
            std::make_shared<GetCashInStatusCompletionPayloadData>(
                status,
                cashInStatus->GetNumOfRefusedItems(),
                cashInStatus->GetCashCounts()
                ? std::make_shared<
                XFS4IoT::CashManagement::StorageCashCountsClass>(
                    cashInStatus->GetCashCounts())
                : nullptr);

        co_return CommandResult
        {
            payload,
            XFS4IoT::MessageHeader::CompletionCodeEnum::Success,
            ""
        };
    }
}