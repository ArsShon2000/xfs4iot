#include "CashInEndHandler.hpp"

#include <format>
#include <map>
#include <stdexcept>
#include <unordered_map>

#include "../../../../core/Exceptions.hpp"
#include "../../CashAcceptorServiceProvider.hpp"

namespace XFS4IoTFramework::CashAcceptor
{
    using CashInEndCommand =
        XFS4IoT::CashAcceptor::Commands::CashInEndCommand;

    using CashInEndCompletion =
        XFS4IoT::CashAcceptor::Completions::CashInEndCompletion;

    using CashInEndCompletionPayloadData =
        XFS4IoT::CashAcceptor::Completions::CashInEndCompletionPayloadData;

    namespace
    {
        std::shared_ptr<XFS4IoT::CashManagement::StorageCashCountsClass>
            ToCompletionCashCounts(
                const std::shared_ptr<XFS4IoTFramework::Storage::StorageCashCountClass>& sourceCounts)
        {
            if (!sourceCounts) {
                return nullptr;
            }

            auto resultCounts =
                std::make_shared<XFS4IoT::CashManagement::StorageCashCountsClass>(
                    sourceCounts->GetUnrecognized());

            std::unordered_map<
                std::string,
                std::shared_ptr<XFS4IoT::CashManagement::StorageCashCountClass>> itemCounts;

            for (const auto& [key, item] : sourceCounts->GetItemCounts()) {
                itemCounts.emplace(
                    key,
                    std::make_shared<XFS4IoT::CashManagement::StorageCashCountClass>(
                        item.GetFit(),
                        item.GetUnfit(),
                        item.GetSuspect(),
                        item.GetCounterfeit(),
                        item.GetInked()));
            }

            resultCounts->setExtendedProperties(std::move(itemCounts));
            return resultCounts;
        }
    }

    CashInEndHandler::CashInEndHandler(
        std::shared_ptr<XFS4IoTServer::IConnection> connection,
        std::shared_ptr<XFS4IoTServer::ICommandDispatcher> dispatcher,
        std::shared_ptr<ILogger> logger)
        : connection_(std::move(connection))
        , logger_(std::move(logger))
    {
        if (!dispatcher) {
            throw std::invalid_argument("CashInEndHandler: dispatcher is null");
        }
        if (!connection_) {
            throw std::invalid_argument("CashInEndHandler: connection is null");
        }
        if (!logger_) {
            throw std::invalid_argument("CashInEndHandler: logger is null");
        }

        provider_ = std::dynamic_pointer_cast<XFS4IoTServer::IServiceProvider>(dispatcher);
        if (!provider_) {
            throw std::runtime_error("CashInEndHandler: dispatcher is not IServiceProvider");
        }

        auto deviceBase = provider_->GetDevice();
        if (!deviceBase) {
            throw std::runtime_error("CashInEndHandler: provider device is null");
        }

        device_ = std::dynamic_pointer_cast<ICashAcceptorDevice>(deviceBase);
        if (!device_) {
            throw std::runtime_error("CashInEndHandler: device is not ICashAcceptorDevice");
        }

        cashAcceptor_ = std::dynamic_pointer_cast<XFS4IoTServer::ICashAcceptorService>(provider_);
        if (!cashAcceptor_) {
            throw std::runtime_error("CashInEndHandler: provider is not ICashAcceptorService");
        }

        common_ = std::dynamic_pointer_cast<XFS4IoTFramework::Common::ICommonService>(provider_);
        if (!common_) {
            throw std::runtime_error("CashInEndHandler: provider is not ICommonService");
        }

        cashManagement_ = std::dynamic_pointer_cast<XFS4IoTFramework::CashManagement::ICashManagementService>(provider_);
        if (!cashManagement_) {
            throw std::runtime_error("CashInEndHandler: provider is not ICashManagementService");
        }

        storage_ = std::dynamic_pointer_cast<XFS4IoTFramework::Storage::IStorageService>(provider_);
        if (!storage_) {
            throw std::runtime_error("CashInEndHandler: provider is not IStorageService");
        }
    }

    boost::asio::awaitable<void> CashInEndHandler::Handle(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::stop_token cancel)
    {
        auto cmd = std::dynamic_pointer_cast<CashInEndCommand>(command);
        if (!cmd) {
            throw std::invalid_argument("CashInEndHandler::Handle: invalid CashInEndCommand");
        }

        if (!cmd->Header().RequestId().has_value()) {
            throw std::runtime_error("CashInEndHandler::Handle: requestId is missing");
        }

        auto events = std::make_shared<CashInEndEvents>(
            connection_,
            cmd->Header().RequestId().value());

        auto result = co_await HandleCashInEnd(cmd, events, cancel);

        auto response = std::make_shared<CashInEndCompletion>(
            cmd->Header().RequestId().value(),
            result.payload,
            result.completionCode,
            result.errorDescription);

        co_await connection_->SendMessageAsync(response);
    }

    boost::asio::awaitable<void> CashInEndHandler::HandleError(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::exception_ptr commandException)
    {
        auto cmd = std::dynamic_pointer_cast<CashInEndCommand>(command);
        if (!cmd) {
            throw std::invalid_argument("CashInEndHandler::HandleError: invalid CashInEndCommand");
        }

        if (!cmd->Header().RequestId().has_value()) {
            throw std::runtime_error("CashInEndHandler::HandleError: requestId is missing");
        }

        auto errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::InternalError;
        std::string errorDescription = "Unknown error";

        try {
            if (commandException) {
                std::rethrow_exception(commandException);
            }
        }
        catch (const XFS4IoT::InvalidDataException& ex) {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::InvalidData;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::InternalErrorException& ex) {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::InternalError;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::UnsupportedDataException& ex) {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::UnsupportedData;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::SequenceErrorException& ex) {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::SequenceError;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::AuthorisationRequiredException& ex) {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::AuthorisationRequired;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::HardwareErrorException& ex) {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::HardwareError;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::UserErrorException& ex) {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::UserError;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::FraudAttemptException& ex) {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::FraudAttempt;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::DeviceNotReadyException& ex) {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::DeviceNotReady;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::InvalidCommandException& ex) {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::InvalidCommand;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::NotEnoughSpaceException& ex) {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::NotEnoughSpace;
            errorDescription = ex.what();
        }
        catch (const XFS4IoT::NotSupportedException& ex) {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::UnsupportedCommand;
            errorDescription = ex.what();
        }
        catch (const XFS4IoTServer::TimeoutCanceledException& ex) {
            errorCode = ex.IsCancelRequested()
                ? XFS4IoT::MessageHeader::CompletionCodeEnum::Canceled
                : XFS4IoT::MessageHeader::CompletionCodeEnum::TimeOut;
            errorDescription = ex.what();
        }
        catch (const std::exception& ex) {
            errorCode = XFS4IoT::MessageHeader::CompletionCodeEnum::InternalError;
            errorDescription = ex.what();
        }

        auto response = std::make_shared<CashInEndCompletion>(
            cmd->Header().RequestId().value(),
            nullptr,
            errorCode,
            errorDescription);

        co_await connection_->SendMessageAsync(response);
    }

    boost::asio::awaitable<CashInEndHandler::CommandResult>
        CashInEndHandler::HandleCashInEnd(
            std::shared_ptr<CashInEndCommand> cashInEnd,
            std::shared_ptr<ICashInEndEvents> events,
            std::stop_token cancel)
    {
        (void)cashInEnd;

        using Payload = CashInEndCompletionPayloadData;

        auto commonStatus = common_->GetCommonStatus();
        if (commonStatus &&
            commonStatus->GetExchange() ==
            XFS4IoTFramework::Common::CommonStatusClass::ExchangeEnum::Active)
        {
            co_return CommandResult{
                std::make_shared<Payload>(Payload::ErrorCodeEnum::ExchangeActive),
                XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
                "The exchange state is already in active."
            };
        }

        auto cashInStatus = cashAcceptor_->GetCashInStatus();
        if (!cashInStatus ||
            cashInStatus->GetStatus() !=
            XFS4IoTFramework::CashManagement::CashInStatusClass::StatusEnum::Active)
        {
            co_return CommandResult{
                std::make_shared<Payload>(Payload::ErrorCodeEnum::NoCashInActive),
                XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
                std::format(
                    "The cash-in state is not in active. {}",
                    cashInStatus ? static_cast<int>(cashInStatus->GetStatus()) : -1)
            };
        }

        for (auto& [pos, presentStatus] : cashManagement_->GetLastCashManagementPresentStatus()) {
            if (presentStatus) {
                presentStatus->SetTotalReturnedItems(
                    XFS4IoTFramework::Storage::StorageCashCountClass{});
            }
        }

        logger_->trace("CashAcceptorDev.CashInEnd()");

        auto result = co_await device_->CashInEnd(
            std::make_shared<CashInEndCommandEvents>(storage_, events),
            cancel);

        logger_->trace(std::format(
            "CashAcceptorDev.CashInEnd() -> {}, error_code_present={}",
            static_cast<int>(result.completionCode),
            result.errorCode.has_value() ? "true" : "false"));

        cashManagement_->GetCashInStatusManaged()->SetStatus(
            XFS4IoTFramework::CashManagement::CashInStatusClass::StatusEnum::Ok);
        cashManagement_->StoreCashInStatus();

        std::optional<std::map<
            std::string,
            std::shared_ptr<XFS4IoT::CashManagement::StorageCashInClass>>> itemMovementResult = std::nullopt;

        if (result.movementResult.has_value() && !result.movementResult->empty()) {
            std::map<
                std::string,
                std::shared_ptr<XFS4IoT::CashManagement::StorageCashInClass>> storageMap;

            for (const auto& [unitId, movement] : result.movementResult.value()) {
                auto storageCashInCount = movement->GetStorageCashInCount();

                if (!storageCashInCount) {
                    continue;
                }

                storageMap.emplace(
                    unitId,
                    std::make_shared<XFS4IoT::CashManagement::StorageCashInClass>(
                        storageCashInCount->GetRetractOperations(),
                        ToCompletionCashCounts(storageCashInCount->GetDeposited()),
                        ToCompletionCashCounts(storageCashInCount->GetRetracted()),
                        ToCompletionCashCounts(storageCashInCount->GetRejected()),
                        ToCompletionCashCounts(storageCashInCount->GetDistributed()),
                        ToCompletionCashCounts(storageCashInCount->GetTransport())));
            }

            if (!storageMap.empty()) {
                itemMovementResult = std::move(storageMap);
            }
        }

        if (result.movementResult.has_value()) {
            co_await storage_->UpdateCashAccounting(result.movementResult);
        }

        std::shared_ptr<Payload> responsePayload = nullptr;

        if (result.errorCode.has_value() || itemMovementResult.has_value()) {
            responsePayload = std::make_shared<Payload>(
                result.errorCode,
                itemMovementResult);
        }

        co_return CommandResult{
            responsePayload,
            result.completionCode,
            result.errorDescription.value_or("")
        };
    }
}
