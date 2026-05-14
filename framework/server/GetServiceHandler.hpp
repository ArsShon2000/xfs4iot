#pragma once

#include <memory>
#include <stop_token>
#include <boost/asio/awaitable.hpp>

#include "./ICommandHandler.hpp"
#include "./IConnection.hpp"
#include "./ServicePublisher.hpp"
#include "../core/Logger/ILogger.hpp"
#include "../core/ServicePublisher/Commands/GetServicesCommand.hpp"
#include "../core/ServicePublisher/Completions/GetServicesCompletion.hpp"
#include "../core/common/MessageHeader.hpp"

namespace XFS4IoTServer
{
    class GetServiceHandler final : public ICommandHandler
    {
    public:
        GetServiceHandler(std::shared_ptr<IConnection> connection,
            std::shared_ptr<ICommandDispatcher> dispatcher,
            std::shared_ptr<ILogger> logger);

        boost::asio::awaitable<void> Handle(
            std::shared_ptr<XFS4IoT::MessageBase> command,
            std::stop_token cancel) override;

        boost::asio::awaitable<void> HandleError(
            std::shared_ptr<XFS4IoT::MessageBase> command,
            std::exception_ptr commandErrorException) override;

    private:
        std::shared_ptr<IConnection> connection_;
        std::shared_ptr<ServicePublisher> servicePublisher_;
        std::shared_ptr<ILogger> logger_;
    };
}