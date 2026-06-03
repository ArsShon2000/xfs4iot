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
#include "../../../core/Common/Commands/SetVersionsCommand.hpp"
#include "../../../core/Common/Completions/SetVersionsCompletion.hpp"

#include "../ICommonDevice.hpp"
#include "../ICommonService.hpp"
#include "../Events/SetVersionsEvents.hpp"

namespace XFS4IoTFramework::Common
{
    class SetVersionsHandler final : public ICommandHandler
    {
    public:
        SetVersionsHandler(
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
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode;
            std::string errorDescription;
        };

        boost::asio::awaitable<CommandResult> HandleSetVersions(
            std::shared_ptr<ISetVersionsEvents> events,
            std::shared_ptr<XFS4IoT::Common::Commands::SetVersionsCommand> setVersions,
            std::stop_token cancel);

    private:
        std::shared_ptr<XFS4IoTServer::IConnection> connection_;
        std::shared_ptr<XFS4IoTServer::IServiceProvider> provider_;
        std::shared_ptr<ICommonDevice> device_;
        std::shared_ptr<ICommonService> common_;
        std::shared_ptr<ILogger> logger_;
    };
}