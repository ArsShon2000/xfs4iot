#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <unordered_map>

#include <boost/asio/awaitable.hpp>

#include "../../../server/ICommandHandler.hpp"
#include "../../../core/Logger/ILogger.hpp"

#include "../../../core/common/XFSConstants.hpp"
#include "../../../core/common/MessageHeader.hpp"
//#include "../../../core/common/DeviceInformationClass.hpp"
//#include "../../../core/common/CapabilityPropertiesClass.hpp"

#include "../../../core/common/Commands/CapabilitiesCommand.hpp"
#include "../../../core/common/Completions/CapabilitiesCompletion.hpp"
#include "../../../server/IConnection.hpp"
#include "../../../server/ICommandDispatcher.hpp"
#include "../ICommonService.hpp"
#include "../ICommonDevice.hpp"
#include "../../../core/common/CommonSchemas.hpp"
#include "../../../ServiceClasses/CommonServiceProvider/CommonCapabilitiesClass.hpp"

namespace XFS4IoTServer::Common
{
    //class ICommonDevice;

    class CapabilitiesHandler final : public ICommandHandler
    {
    public:
        CapabilitiesHandler(
            std::shared_ptr<IConnection> connection,
            std::shared_ptr<ICommandDispatcher> dispatcher,
            std::shared_ptr<ILogger> logger);

        boost::asio::awaitable<void> Handle(
            std::shared_ptr<XFS4IoT::MessageBase> command,
            std::stop_token cancel) override;

        boost::asio::awaitable<void> HandleError(
            std::shared_ptr<XFS4IoT::MessageBase> command,
            std::exception_ptr commandErrorException) override;

    private:
        struct CapabilitiesResult
        {
            std::shared_ptr<XFS4IoT::Common::Completions::CapabilitiesCompletionPayloadData> payload;
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode;
            std::string errorDescription;
        };

    private:
        boost::asio::awaitable<CapabilitiesResult> HandleCapabilities(
            std::shared_ptr<XFS4IoT::Common::Commands::CapabilitiesCommand> capabilities,
            std::stop_token cancel);

        std::shared_ptr<XFS4IoT::Common::InterfaceClass> GetDeviceInterface(
            XFS4IoT::Common::InterfaceClass::NameEnum interfaceName) const;

        XFS4IoT::MessageHeader::CompletionCodeEnum MapExceptionToCompletionCode(
            const std::exception& ex) const;

    private:
        std::shared_ptr<XFS4IoTServer::IConnection> connection_;
        std::shared_ptr<XFS4IoTServer::IServiceProvider> provider_;
        std::shared_ptr< XFS4IoTFramework::Common::ICommonService> common_;
        std::shared_ptr<ILogger> logger_;
    };
}