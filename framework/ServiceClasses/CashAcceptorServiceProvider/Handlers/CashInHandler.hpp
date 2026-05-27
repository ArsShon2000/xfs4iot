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
#include "../../../core/CashAcceptor/Commands/CashInCommand.hpp"
#include "../../../core/CashAcceptor/Completions/CashInCompletion.hpp"
#include "../ICashAcceptorDevice.hpp"
#include "../ICashAcceptorService.hpp"
#include "../Events/CashInEvents.hpp"
#include "../../ServiceInterfaces/CashAcceptor/ICashInEvents.hpp"
#include "../Events/CashInCommandEvents.hpp"
#include "../../CommonServiceProvider/ICommonService.hpp"
#include "../../CashManagementServiceProvider/ICashManagementService.hpp"

namespace XFS4IoTFramework::CashAcceptor
{
    class CashInHandler final : public XFS4IoTServer::ICommandHandler
    {
    public:
        CashInHandler(
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
            std::shared_ptr<XFS4IoT::CashAcceptor::Completions::CashInCompletionPayloadData> payload;
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode;
            std::string errorDescription;
        };

        boost::asio::awaitable<CommandResult> HandleCashIn(
            std::shared_ptr<XFS4IoT::CashAcceptor::Commands::CashInCommand> cashIn,
            std::shared_ptr<ICashInEvents> events,
            std::stop_token cancel);

    private:
        std::shared_ptr<XFS4IoTServer::IConnection> connection_;
        std::shared_ptr<XFS4IoTServer::IServiceProvider> provider_;

        std::shared_ptr<ICashAcceptorDevice> device_;
        std::shared_ptr<XFS4IoTServer::ICashAcceptorService> cashAcceptor_;
        std::shared_ptr<XFS4IoTFramework::Common::ICommonService> common_;
        std::shared_ptr<XFS4IoTFramework::CashManagement::ICashManagementService> cashManagement_;
        std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage_;

        std::shared_ptr<ILogger> logger_;
    };
}