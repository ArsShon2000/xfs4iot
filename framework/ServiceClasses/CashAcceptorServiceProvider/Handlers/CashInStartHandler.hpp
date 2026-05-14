#pragma once

#include <memory>
#include <stop_token>
#include <string>

#include <boost/asio/awaitable.hpp>

#include "../../../server/ICommandHandler.hpp"
#include "../../../server/IConnection.hpp"
#include "../../../server/ICommandDispatcher.hpp"
#include "../../../server/IServiceProvider.hpp"

#include "../../../core/Logger/ILogger.hpp"
#include "../../../core/common/MessageHeader.hpp"

#include "../../../core/CashAcceptor/Commands/CashInStartCommand.hpp"
#include "../../../core/CashAcceptor/Completions/CashInStartCompletion.hpp"

#include "../ICashAcceptorDevice.hpp"
#include "../ICashAcceptorService.hpp"
#include "../../CashManagementServiceProvider/ICashManagementService.hpp"
#include "../../CommonServiceProvider/ICommonService.hpp"

namespace XFS4IoTFramework::CashAcceptor
{
    class CashInStartHandler final : public XFS4IoTServer::ICommandHandler
    {
    public:
        CashInStartHandler(
            std::shared_ptr<XFS4IoTServer::IConnection> connection,
            std::shared_ptr<XFS4IoTServer::ICommandDispatcher> dispatcher,
            std::shared_ptr<ILogger> logger);

        boost::asio::awaitable<void> Handle(
            std::shared_ptr<XFS4IoT::MessageBase> command,
            std::stop_token cancel) override;

        boost::asio::awaitable<void> HandleError(
            std::shared_ptr<XFS4IoT::MessageBase> command,
            std::exception_ptr commandException) override;

    private:
        struct CommandResult
        {
            std::shared_ptr<XFS4IoT::CashAcceptor::Completions::CashInStartCompletionPayloadData> payload;
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode;
            std::string errorDescription;
        };

        boost::asio::awaitable<CommandResult> HandleCashInStart(
            std::shared_ptr<XFS4IoT::CashAcceptor::Commands::CashInStartCommand> cashInStart,
            std::stop_token cancel);

    private:
        std::shared_ptr<XFS4IoTServer::IConnection> connection_;
        std::shared_ptr<XFS4IoTServer::IServiceProvider> provider_;

        std::shared_ptr<ICashAcceptorDevice> device_;
        std::shared_ptr<XFS4IoTServer::ICashAcceptorService> cashAcceptor_;
        std::shared_ptr<XFS4IoTFramework::Common::ICommonService> common_;
        std::shared_ptr<XFS4IoTFramework::CashManagement::ICashManagementService> cashManagement_;

        std::shared_ptr<ILogger> logger_;
    };
}