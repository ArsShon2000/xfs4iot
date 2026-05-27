#include "SetVersionsHandler.hpp"

#include <format>
#include <ranges>
#include <stdexcept>

#include "../../../core/Exceptions.hpp"

namespace XFS4IoTFramework::Common
{
    using SetVersionsCommand =
        XFS4IoT::Common::Commands::SetVersionsCommand;

    using SetVersionsCompletion =
        XFS4IoT::Common::Completions::SetVersionsCompletion;

    SetVersionsHandler::SetVersionsHandler(
        std::shared_ptr<XFS4IoTServer::IConnection> connection,
        std::shared_ptr<XFS4IoTServer::ICommandDispatcher> dispatcher,
        std::shared_ptr<ILogger> logger)
        : connection_(std::move(connection))
        , logger_(std::move(logger))
    {
        if (!dispatcher)
        {
            throw std::invalid_argument("SetVersionsHandler: dispatcher is null");
        }

        if (!connection_)
        {
            throw std::invalid_argument("SetVersionsHandler: connection is null");
        }

        if (!logger_)
        {
            throw std::invalid_argument("SetVersionsHandler: logger is null");
        }

        provider_ = std::dynamic_pointer_cast<XFS4IoTServer::IServiceProvider>(dispatcher);
        if (!provider_)
        {
            throw std::runtime_error("SetVersionsHandler: dispatcher is not IServiceProvider");
        }

        auto deviceBase = provider_->GetDevice();
        if (!deviceBase)
        {
            throw std::runtime_error("SetVersionsHandler: provider device is null");
        }

        device_ = std::dynamic_pointer_cast<ICommonDevice>(deviceBase);
        if (!device_)
        {
            throw std::runtime_error("SetVersionsHandler: provider device is not ICommonDevice");
        }

        common_ = std::dynamic_pointer_cast<ICommonService>(provider_);
        if (!common_)
        {
            throw std::runtime_error("SetVersionsHandler: provider is not ICommonService");
        }
    }

    boost::asio::awaitable<void> SetVersionsHandler::Handle(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::stop_token cancel)
    {
        auto cmd = std::dynamic_pointer_cast<SetVersionsCommand>(command);
        if (!cmd)
        {
            throw std::invalid_argument("SetVersionsHandler::Handle: invalid SetVersionsCommand");
        }

        if (!cmd->Header().RequestId().has_value())
        {
            throw std::runtime_error("SetVersionsHandler::Handle: requestId is missing");
        }

        auto events = std::make_shared<SetVersionsEvents>(
            connection_,
            cmd->Header().RequestId().value());

        auto result = co_await HandleSetVersions(events, cmd, cancel);

        auto response = std::make_shared<SetVersionsCompletion>(
            cmd->Header().RequestId().value(),
            result.completionCode,
            result.errorDescription);

        co_await connection_->SendMessageAsync(response);
    }

    boost::asio::awaitable<void> SetVersionsHandler::HandleError(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::exception_ptr commandException)
    {
        auto cmd = std::dynamic_pointer_cast<SetVersionsCommand>(command);
        if (!cmd)
        {
            throw std::invalid_argument("SetVersionsHandler::HandleError: invalid SetVersionsCommand");
        }

        if (!cmd->Header().RequestId().has_value())
        {
            throw std::runtime_error("SetVersionsHandler::HandleError: requestId is missing");
        }

        auto errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::InternalError;
        std::string errorDescription = "Unknown error";

        try
        {
            if (commandException)
            {
                std::rethrow_exception(commandException);
            }
        }
        catch (const XFS4IoT::InvalidDataException& ex)
        {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::InvalidData;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::InternalErrorException& ex)
        {
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::UnsupportedDataException& ex)
        {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::UnsupportedData;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::SequenceErrorException& ex)
        {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::SequenceError;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::AuthorisationRequiredException& ex)
        {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::AuthorisationRequired;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::HardwareErrorException& ex)
        {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::HardwareError;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::UserErrorException& ex)
        {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::UserError;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::FraudAttemptException& ex)
        {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::FraudAttempt;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::DeviceNotReadyException& ex)
        {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::DeviceNotReady;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::InvalidCommandException& ex)
        {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::InvalidCommand;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::NotEnoughSpaceException& ex)
        {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::NotEnoughSpace;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::NotSupportedException& ex)
        {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::UnsupportedCommand;
            errorDescription = ex.what();
        }
        catch (const XFS4IoTServer::TimeoutCanceledException& ex)
        {
            errorCode = ex.IsCancelRequested()
                ? XFS4IoT::MessageHeader::CompletionCodeEnum::Canceled
                : XFS4IoT::MessageHeader::CompletionCodeEnum::TimeOut;
            errorDescription = ex.what();
        }
        catch (const std::exception& ex)
        {
            errorDescription = ex.what();
        }

        auto response = std::make_shared<SetVersionsCompletion>(
            cmd->Header().RequestId().value(),
            errorCode,
            errorDescription);

        co_await connection_->SendMessageAsync(response);
    }

    boost::asio::awaitable<SetVersionsHandler::CommandResult>
        SetVersionsHandler::HandleSetVersions(
            std::shared_ptr<ISetVersionsEvents> events,
            std::shared_ptr<SetVersionsCommand> setVersions,
            std::stop_token cancel)
    {
        (void)events;
        (void)cancel;

        auto payload = setVersions->Payload;

        if (!payload)
        {
            co_return CommandResult{
                XFS4IoT::MessageHeader::CompletionCodeEnum::InvalidData,
                "Не указаны команды."
            };
        }

        const bool noCommands =
            !payload->GetCommands().has_value() ||
            payload->GetCommands()->empty();

        const bool noEvents =
            !payload->GetEvents().has_value() ||
            payload->GetEvents()->empty();

        if (noCommands && noEvents)
        {
            co_return CommandResult{
                XFS4IoT::MessageHeader::CompletionCodeEnum::InvalidData,
                "Не указаны команды и события."
            };
        }

        const auto supported = provider_->GetMessagesSupported();

        if (payload->GetCommands().has_value())
        {
            for (const auto& [name, majorVersion] : payload->GetCommands().value())
            {
                auto it = supported.find(name);
                if (it == supported.end())
                {
                    co_return CommandResult{
                        XFS4IoT::MessageHeader::CompletionCodeEnum::UnsupportedData,
                        std::format("Указана неподдерживаемая команда. {}", name)
                    };
                }

                const std::string requestedVersion = std::format("{}.0", majorVersion);

                if (std::ranges::find(it->second.Versions, requestedVersion) == it->second.Versions.end())
                {
                    const std::string supportedVersion =
                        it->second.Versions.empty() ? "" : it->second.Versions.front();

                    co_return CommandResult{
                        XFS4IoT::MessageHeader::CompletionCodeEnum::UnsupportedData,
                        std::format(
                            "Неподдерживаемая версия указана. {}, Сервис поддерживает исключительно эти версии {}",
                            majorVersion,
                            supportedVersion)
                    };
                }
            }
        }

        if (payload->GetEvents().has_value())
        {
            for (const auto& [name, majorVersion] : payload->GetEvents().value())
            {
                auto it = supported.find(name);
                if (it == supported.end())
                {
                    co_return CommandResult{
                        XFS4IoT::MessageHeader::CompletionCodeEnum::UnsupportedData,
                        std::format("Неподдерживаемое событие указано. {}", name)
                    };
                }

                const std::string requestedVersion = std::format("{}.0", majorVersion);

                if (std::ranges::find(it->second.Versions, requestedVersion) == it->second.Versions.end())
                {
                    const std::string supportedVersion =
                        it->second.Versions.empty() ? "" : it->second.Versions.front();

                    co_return CommandResult{
                        XFS4IoT::MessageHeader::CompletionCodeEnum::UnsupportedData,
                        std::format(
                            "Неподдерживаемая версия указана. {}, Сервис поддерживает исключительно эти версии {}",
                            majorVersion,
                            supportedVersion)
                    };
                }
            }
        }

        co_return CommandResult{
            XFS4IoT::MessageHeader::CompletionCodeEnum::Success,
            ""
        };
    }
}