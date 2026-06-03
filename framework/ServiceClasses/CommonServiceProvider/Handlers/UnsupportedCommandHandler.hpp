#pragma once

#include <memory>
#include <stop_token>
#include <boost/asio/awaitable.hpp>

#include "../../../server/ICommandHandler.hpp"
#include "../../../server/IConnection.hpp"
#include "../../../server/ICommandDispatcher.hpp"
#include "../../../core/Logger/ILogger.hpp"

#include "../../../core/Common/Commands/UnsupportedCommand.hpp"
#include "../../../core/Common/Completions/GenericCompletion.hpp"

namespace XFS4IoTFramework::Common
{
    class UnsupportedCommandHandler final : public XFS4IoTServer::ICommandHandler
    {
    public:
        UnsupportedCommandHandler(
            std::shared_ptr<XFS4IoTServer::IConnection> connection,
            std::shared_ptr<XFS4IoTServer::ICommandDispatcher> dispatcher,
            std::shared_ptr<ILogger> logger)
            : connection_(std::move(connection))
            , logger_(std::move(logger))
        {
            (void)dispatcher;

            if (!connection_)
            {
                throw std::invalid_argument("UnsupportedCommandHandler: connection is null");
            }

            if (!logger_)
            {
                throw std::invalid_argument("UnsupportedCommandHandler: logger is null");
            }
        }

        boost::asio::awaitable<void> Handle(
            std::shared_ptr<XFS4IoT::MessageBase> command,
            std::stop_token cancel) override
        {
            (void)cancel;

            auto unsupported =
                std::dynamic_pointer_cast<XFS4IoT::Common::Commands::UnsupportedCommand>(command);

            if (!unsupported)
            {
                throw std::invalid_argument(
                    "UnsupportedCommandHandler::Handle: invalid UnsupportedCommand");
            }

            const auto& header = unsupported->Header();

            if (!header.RequestId().has_value())
            {
                throw std::runtime_error(
                    "UnsupportedCommandHandler::Handle: requestId отсутствует");
            }

            auto response =
                std::make_shared<XFS4IoT::Common::Completions::GenericCompletion>(
                    unsupported->GetCommandName(),
                    unsupported->GetVersion(),
                    header.RequestId().value(),
                    XFS4IoT::MessageHeader::CompletionCodeEnum::UnsupportedCommand,
                    "Команда не поддерживается.");

            co_await connection_->SendMessageAsync(response);
        }

        boost::asio::awaitable<void> HandleError(
            std::shared_ptr<XFS4IoT::MessageBase> command,
            std::exception_ptr commandException) override
        {
            (void)commandException;
            co_await Handle(std::move(command), std::stop_token{});
        }

    private:
        std::shared_ptr<XFS4IoTServer::IConnection> connection_;
        std::shared_ptr<ILogger> logger_;
    };
}