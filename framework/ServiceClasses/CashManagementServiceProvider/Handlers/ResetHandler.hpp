#pragma once

#include <memory>
#include <stop_token>
#include <string>
#include <unordered_map>

#include <boost/asio/awaitable.hpp>

#include "../../../server/ICommandHandler.hpp"
#include "../../../server/IConnection.hpp"
#include "../../../server/ICommandDispatcher.hpp"
#include "../../../server/IServiceProvider.hpp"
#include "../../../core/Logger/ILogger.hpp"
#include "../../../core/common/MessageHeader.hpp"

#include "../../../core/CashManagement/Commands/ResetCommand.hpp"
#include "../../../core/CashManagement/Completions/ResetCompletion.hpp"

#include "../ICashManagementDevice.hpp"
#include "../ICashManagementService.hpp"
#include "../../CommonServiceProvider/ICommonService.hpp"

namespace XFS4IoTFramework::CashManagement
{
    class ResetHandler final : public XFS4IoTServer::ICommandHandler
    {
    public:
        ResetHandler(
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
            std::shared_ptr<XFS4IoT::CashManagement::Completions::ResetCompletionPayloadData> payload;
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode;
            std::string errorDescription;
        };

        boost::asio::awaitable<CommandResult> HandleReset(
            std::shared_ptr<IResetEvents> events,
            std::shared_ptr<XFS4IoT::CashManagement::Commands::ResetCommand> reset,
            std::stop_token cancel);

    private:
        std::shared_ptr<XFS4IoTServer::IConnection> connection_;
        std::shared_ptr<XFS4IoTServer::IServiceProvider> provider_;
        std::shared_ptr<ICashManagementDevice> device_;
        std::shared_ptr<ICashManagementService> cashManagement_;
        std::shared_ptr<XFS4IoTFramework::Common::ICommonService> common_;
        std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage_;
        std::shared_ptr<ILogger> logger_;
    };
}