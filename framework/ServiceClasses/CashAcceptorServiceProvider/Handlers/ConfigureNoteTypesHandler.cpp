#include "ConfigureNoteTypesHandler.hpp"

#include <format>
#include <regex>
#include <stdexcept>
#include <unordered_map>

#include "../../../core/Exceptions.hpp"

namespace XFS4IoTFramework::CashAcceptor
{
    using ConfigureNoteTypesCommand =
        XFS4IoT::CashAcceptor::Commands::ConfigureNoteTypesCommand;

    using ConfigureNoteTypesCompletion =
        XFS4IoT::CashAcceptor::Completions::ConfigureNoteTypesCompletion;

    using ConfigureNoteTypesCompletionPayloadData =
        XFS4IoT::CashAcceptor::Completions::ConfigureNoteTypesCompletionPayloadData;

    ConfigureNoteTypesHandler::ConfigureNoteTypesHandler(
        std::shared_ptr<XFS4IoTServer::IConnection> connection,
        std::shared_ptr<XFS4IoTServer::ICommandDispatcher> dispatcher,
        std::shared_ptr<ILogger> logger)
        : connection_(std::move(connection))
        , logger_(std::move(logger))
    {
        if (!dispatcher) throw std::invalid_argument("ConfigureNoteTypesHandler: dispatcher не может быть null");
        if (!connection_) throw std::invalid_argument("ConfigureNoteTypesHandler: connection не может быть null");
        if (!logger_) throw std::invalid_argument("ConfigureNoteTypesHandler: logger не может быть null");

        provider_ = std::dynamic_pointer_cast<XFS4IoTServer::IServiceProvider>(dispatcher);
        if (!provider_) throw std::runtime_error("ConfigureNoteTypesHandler: dispatcher не IServiceProvider");

        auto deviceBase = provider_->GetDevice();
        if (!deviceBase) throw std::runtime_error("ConfigureNoteTypesHandler: provider device не может быть null");

        device_ = std::dynamic_pointer_cast<ICashAcceptorDevice>(deviceBase);
        if (!device_) throw std::runtime_error("ConfigureNoteTypesHandler: device не ICashAcceptorDevice");

        cashAcceptor_ = std::dynamic_pointer_cast<XFS4IoTServer::ICashAcceptorService>(provider_);
        if (!cashAcceptor_) throw std::runtime_error("ConfigureNoteTypesHandler: provider не ICashAcceptorService");

        common_ = std::dynamic_pointer_cast<XFS4IoTFramework::Common::ICommonService>(provider_);
        if (!common_) throw std::runtime_error("ConfigureNoteTypesHandler: provider не ICommonService");

        cashManagement_ = std::dynamic_pointer_cast<XFS4IoTFramework::CashManagement::ICashManagementService>(provider_);
        if (!cashManagement_) throw std::runtime_error("ConfigureNoteTypesHandler: provider не ICashManagementService");
    }

    boost::asio::awaitable<void> ConfigureNoteTypesHandler::Handle(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::stop_token cancel)
    {
        auto cmd = std::dynamic_pointer_cast<ConfigureNoteTypesCommand>(command);
        if (!cmd) throw std::invalid_argument("ConfigureNoteTypesHandler::Handle: Неверная команда");

        if (!cmd->Header().RequestId().has_value())
            throw std::runtime_error("ConfigureNoteTypesHandler::Handle: requestId отсутствует");

        auto result = co_await HandleConfigureNoteTypes(cmd, cancel);

        auto response = std::make_shared<ConfigureNoteTypesCompletion>(
            cmd->Header().RequestId().value(),
            result.payload,
            result.completionCode,
            result.errorDescription);

        co_await connection_->SendMessageAsync(response);
    }

    boost::asio::awaitable<void> ConfigureNoteTypesHandler::HandleError(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::exception_ptr commandException)
    {
        auto cmd = std::dynamic_pointer_cast<ConfigureNoteTypesCommand>(command);
        if (!cmd) throw std::invalid_argument("ConfigureNoteTypesHandler::HandleError: Неверная команда");

        if (!cmd->Header().RequestId().has_value())
            throw std::runtime_error("ConfigureNoteTypesHandler::HandleError: requestId отсутствует");

        auto errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::InternalError;
        std::string errorDescription = "Unknown error";

        try
        {
            if (commandException)
                std::rethrow_exception(commandException);
        }
        catch (const XFS4IoT::InvalidDataException& ex)
        {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::InvalidData;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::InternalErrorException& ex)
        {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::InternalError;
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
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::InternalError;
            errorDescription = ex.what();
        }

        auto response = std::make_shared<ConfigureNoteTypesCompletion>(
            cmd->Header().RequestId().value(),
            nullptr,
            errorCode,
            errorDescription);

        co_await connection_->SendMessageAsync(response);
    }

    boost::asio::awaitable<ConfigureNoteTypesHandler::CommandResult>
        ConfigureNoteTypesHandler::HandleConfigureNoteTypes(
            std::shared_ptr<ConfigureNoteTypesCommand> configureNoteTypes,
            std::stop_token cancel)
    {
        auto payload = configureNoteTypes->Payload;
        const auto& itemsOpt = payload ? payload->GetItems() : std::nullopt;

        if (!itemsOpt.has_value() || itemsOpt->empty())
        {
            co_return CommandResult{
                nullptr,
                XFS4IoT::MessageHeader::CompletionCodeEnum::Success,
                "Нет элементов банкнот для включения или отключения."
            };
        }

        static const std::regex itemRegex(R"(^type[0-9A-Z]+$)");

        for (const auto& item : itemsOpt.value())
        {
            if (!item.GetItem().has_value() ||
                item.GetItem()->empty() ||
                !std::regex_match(item.GetItem().value(), itemRegex))
            {
                co_return CommandResult{
                    nullptr,
                    XFS4IoT::MessageHeader::CompletionCodeEnum::InvalidData,
                    "Указано недопустимое имя элемента."
                };
            }
        }

        auto commonStatus = common_->GetCommonStatus();
        if (commonStatus &&
            commonStatus->GetExchange() ==
            XFS4IoTFramework::Common::CommonStatusClass::ExchangeEnum::Active)
        {
            auto responsePayload = std::make_shared<ConfigureNoteTypesCompletionPayloadData>(
                ConfigureNoteTypesCompletionPayloadData::ErrorCodeEnum::ExchangeActive);

            co_return CommandResult{
                responsePayload,
                XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
                "Состояние обмена уже активно."
            };
        }

        auto cashInStatus = cashAcceptor_->GetCashInStatus();
        if (cashInStatus &&
            cashInStatus->GetStatus() ==
            XFS4IoTFramework::CashManagement::CashInStatusClass::StatusEnum::Active)
        {
            auto responsePayload = std::make_shared<ConfigureNoteTypesCompletionPayloadData>(
                ConfigureNoteTypesCompletionPayloadData::ErrorCodeEnum::CashInActive);

            co_return CommandResult{
                responsePayload,
                XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
                std::format(
                    "Состояние cash-in уже активно. {}",
                    static_cast<int>(cashInStatus->GetStatus()))
            };
        }

        auto cashMgmtCaps = common_->GetCashManagementCapabilities();
        if (!cashMgmtCaps || !cashMgmtCaps->GetAllBanknoteItems().has_value())
        {
            co_return CommandResult{
                nullptr,
                XFS4IoT::MessageHeader::CompletionCodeEnum::InvalidData,
                "Нет конфигурации элементов банкнот."
            };
        }

        for (const auto& item : itemsOpt.value())
        {
            if (!cashMgmtCaps->GetAllBanknoteItems()->contains(item.GetItem().value()))
            {
                co_return CommandResult{
                    nullptr,
                    XFS4IoT::MessageHeader::CompletionCodeEnum::UnsupportedData,
                    std::format("Указанное имя элемента не существует. {}", item.GetItem().value())
                };
            }
        }

        std::unordered_map<std::string, bool> requestItems;
        for (const auto& item : itemsOpt.value())
        {
            requestItems.emplace(
                item.GetItem().value(),
                item.GetEnabled().value_or(false));
        }

        logger_->trace("PSHANDLER.ConfigureNoteTypes()");

        auto result = co_await device_->ConfigureNoteTypes(
            ConfigureNoteTypesRequest(std::move(requestItems)),
            cancel);

        logger_->trace(std::format(
            "PSHANDLER.ConfigureNoteTypes() -> {}, errorCode_present={}",
            static_cast<int>(result.completionCode),
            result.errorCode.has_value() ? "true" : "false"));

        std::shared_ptr<ConfigureNoteTypesCompletionPayloadData> responsePayload = nullptr;

        if (result.errorCode.has_value())
        {
            responsePayload = std::make_shared<ConfigureNoteTypesCompletionPayloadData>(
                result.errorCode.value());
        }

        co_return CommandResult{
            responsePayload,
            result.completionCode,
            result.errorDescription.value_or("")
        };
    }
}