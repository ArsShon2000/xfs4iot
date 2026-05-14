#include "GetBankNoteTypesHandler.hpp"
#include <format>
#include <stdexcept>

#include "../../../core/Exceptions.hpp"

namespace XFS4IoTFramework::CashManagement
{
    using GetBankNoteTypesCommand =
        XFS4IoT::CashManagement::Commands::GetBankNoteTypesCommand;

    using GetBankNoteTypesCompletion =
        XFS4IoT::CashManagement::Completions::GetBankNoteTypesCompletion;

    using GetBankNoteTypesCompletionPayloadData =
        XFS4IoT::CashManagement::Completions::GetBankNoteTypesCompletionPayloadData;

    GetBankNoteTypesHandler::GetBankNoteTypesHandler(
        std::shared_ptr<XFS4IoTServer::IConnection> connection,
        std::shared_ptr<XFS4IoTServer::ICommandDispatcher> dispatcher,
        std::shared_ptr<ILogger> logger)
        : connection_(std::move(connection))
        , logger_(std::move(logger))
    {
        if (!dispatcher)
        {
            throw std::invalid_argument("GetBankNoteTypesHandler: dispatcher is null");
        }

        if (!connection_)
        {
            throw std::invalid_argument("GetBankNoteTypesHandler: connection is null");
        }

        if (!logger_)
        {
            throw std::invalid_argument("GetBankNoteTypesHandler: logger is null");
        }

        provider_ = std::dynamic_pointer_cast<XFS4IoTServer::IServiceProvider>(dispatcher);
        if (!provider_)
        {
            throw std::runtime_error("GetBankNoteTypesHandler: dispatcher is not IServiceProvider");
        }

        auto deviceBase = provider_->GetDevice();
        if (!deviceBase)
        {
            throw std::runtime_error("GetBankNoteTypesHandler: provider device is null");
        }

        device_ = std::dynamic_pointer_cast<ICashManagementDevice>(deviceBase);
        if (!device_)
        {
            throw std::runtime_error("GetBankNoteTypesHandler: provider device is not ICashManagementDevice");
        }

        cashManagement_ = std::dynamic_pointer_cast<ICashManagementService>(provider_);
        if (!cashManagement_)
        {
            throw std::runtime_error("GetBankNoteTypesHandler: provider is not ICashManagementService");
        }

        common_ = std::dynamic_pointer_cast<XFS4IoTFramework::Common::ICommonService>(provider_);
        if (!common_)
        {
            throw std::runtime_error("GetBankNoteTypesHandler: provider is not ICommonService");
        }
    }

    boost::asio::awaitable<void> GetBankNoteTypesHandler::Handle(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::stop_token cancel)
    {
        auto getBankNoteTypesCmd =
            std::dynamic_pointer_cast<GetBankNoteTypesCommand>(command);

        if (!getBankNoteTypesCmd)
        {
            throw std::invalid_argument(
                "GetBankNoteTypesHandler::Handle: invalid GetBankNoteTypesCommand");
        }

        if (!getBankNoteTypesCmd->Header().RequestId().has_value())
        {
            throw std::runtime_error(
                "GetBankNoteTypesHandler::Handle: requestId is missing");
        }

        auto result = co_await HandleGetBankNoteTypes(
            getBankNoteTypesCmd,
            cancel);

        auto response =
            std::make_shared<GetBankNoteTypesCompletion>(
                getBankNoteTypesCmd->Header().RequestId().value(),
                result.payload,
                result.completionCode,
                result.errorDescription);

        co_await connection_->SendMessageAsync(response);
    }

    boost::asio::awaitable<void> GetBankNoteTypesHandler::HandleError(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::exception_ptr commandException)
    {
        auto getBankNoteTypesCmd =
            std::dynamic_pointer_cast<GetBankNoteTypesCommand>(command);

        if (!getBankNoteTypesCmd)
        {
            throw std::invalid_argument(
                "GetBankNoteTypesHandler::HandleError: invalid GetBankNoteTypesCommand");
        }

        if (!getBankNoteTypesCmd->Header().RequestId().has_value())
        {
            throw std::runtime_error(
                "GetBankNoteTypesHandler::HandleError: requestId is missing");
        }

        XFS4IoT::MessageHeader::CompletionCodeEnum errorCode =
            XFS4IoT::MessageHeader::CompletionCodeEnum::InternalError;

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

        auto response =
            std::make_shared<GetBankNoteTypesCompletion>(
                getBankNoteTypesCmd->Header().RequestId().value(),
                nullptr,
                errorCode,
                errorDescription);

        co_await connection_->SendMessageAsync(response);
    }

    boost::asio::awaitable<GetBankNoteTypesHandler::CommandResult>
        GetBankNoteTypesHandler::HandleGetBankNoteTypes(
            std::shared_ptr<GetBankNoteTypesCommand> getBankNoteTypes,
            std::stop_token cancel)
    {
        (void)getBankNoteTypes;
        (void)cancel;

        std::optional<std::unordered_map<
            std::string,
            std::shared_ptr<XFS4IoT::CashManagement::BankNoteClass>>> items = std::nullopt;

        auto cashManagementCaps = common_->GetCashManagementCapabilities();

        if (cashManagementCaps &&
            cashManagementCaps->GetAllBanknoteItems().has_value() &&
            !cashManagementCaps->GetAllBanknoteItems()->empty())
        {
            std::unordered_map<
                std::string,
                std::shared_ptr<XFS4IoT::CashManagement::BankNoteClass>> resultItems;

            for (const auto& [key, item] : cashManagementCaps->GetAllBanknoteItems().value())
            {
                if (!item.IsEnabled())  // пока буду отображать все банкноты, если нгужно толькко разрешенных раскомментирую
                {
                    //continue;
                }

                auto cashItem =
                    std::make_shared<XFS4IoT::CashManagement::CashItemClass>(
                        item.GetNoteId(),
                        item.GetCurrency(),
                        item.GetValue(),
                        item.GetRelease());

                resultItems.emplace(
                    key,
                    std::make_shared<XFS4IoT::CashManagement::BankNoteClass>(
                        cashItem,
                        item.IsEnabled()));
            }

            if (!resultItems.empty())
            {
                items = std::move(resultItems);
            }
        }

        std::shared_ptr<GetBankNoteTypesCompletionPayloadData> payload = nullptr;

        if (items.has_value())
        {
            payload =
                std::make_shared<GetBankNoteTypesCompletionPayloadData>(
                    std::move(items));
        }

        co_return CommandResult{
            payload,
            XFS4IoT::MessageHeader::CompletionCodeEnum::Success,
            ""
        };
    }
}