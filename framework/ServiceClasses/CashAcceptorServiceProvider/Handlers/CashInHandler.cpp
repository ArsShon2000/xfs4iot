#include "CashInHandler.hpp"

#include <format>
#include <stdexcept>
#include <unordered_map>
#include "../../../core/Exceptions.hpp"

namespace XFS4IoTFramework::CashAcceptor
{
    namespace
    {
        void AddCashItemCounts(
            XFS4IoTFramework::Storage::CashItemCountClass& target,
            const XFS4IoTFramework::Storage::CashItemCountClass& source)
        {
            target.SetCounterfeit(target.GetCounterfeit() + source.GetCounterfeit());
            target.SetFit(target.GetFit() + source.GetFit());
            target.SetInked(target.GetInked() + source.GetInked());
            target.SetSuspect(target.GetSuspect() + source.GetSuspect());
            target.SetUnfit(target.GetUnfit() + source.GetUnfit());
        }
    }

    using CashInCommand =
        XFS4IoT::CashAcceptor::Commands::CashInCommand;

    using CashInCompletion =
        XFS4IoT::CashAcceptor::Completions::CashInCompletion;

    using CashInCompletionPayloadData =
        XFS4IoT::CashAcceptor::Completions::CashInCompletionPayloadData;

    CashInHandler::CashInHandler(
        std::shared_ptr<XFS4IoTServer::IConnection> connection,
        std::shared_ptr<XFS4IoTServer::ICommandDispatcher> dispatcher,
        std::shared_ptr<ILogger> logger)
        : connection_(std::move(connection))
        , logger_(std::move(logger))
    {
        if (!dispatcher) throw std::invalid_argument("CashInHandler: dispatcher не может быть null");
        if (!connection_) throw std::invalid_argument("CashInHandler: connection не может быть null");
        if (!logger_) throw std::invalid_argument("CashInHandler: logger не может быть null");

        provider_ = std::dynamic_pointer_cast<XFS4IoTServer::IServiceProvider>(dispatcher);
        if (!provider_) throw std::runtime_error("CashInHandler: dispatcher не является IServiceProvider");

        auto deviceBase = provider_->GetDevice();
        if (!deviceBase) throw std::runtime_error("CashInHandler: provider device не может быть null");

        device_ = std::dynamic_pointer_cast<ICashAcceptorDevice>(deviceBase);
        if (!device_) throw std::runtime_error("CashInHandler: device не является ICashAcceptorDevice");

        cashAcceptor_ = std::dynamic_pointer_cast<XFS4IoTServer::ICashAcceptorService>(provider_);
        if (!cashAcceptor_) throw std::runtime_error("CashInHandler: provider не является ICashAcceptorService");

        common_ = std::dynamic_pointer_cast<XFS4IoTFramework::Common::ICommonService>(provider_);
        if (!common_) throw std::runtime_error("CashInHandler: provider не является ICommonService");

        cashManagement_ = std::dynamic_pointer_cast<XFS4IoTFramework::CashManagement::ICashManagementService>(provider_);
        if (!cashManagement_) throw std::runtime_error("CashInHandler: provider не является ICashManagementService");

        storage_ = std::dynamic_pointer_cast<XFS4IoTFramework::Storage::IStorageService>(provider_);
        if (!storage_) throw std::runtime_error("CashInHandler: provider не является IStorageService");
    }

    boost::asio::awaitable<void> CashInHandler::Handle(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::stop_token cancel)
    {
        auto cmd = std::dynamic_pointer_cast<CashInCommand>(command);
        if (!cmd)
            throw std::invalid_argument("CashInHandler::Handle: invalid CashInCommand");

        if (!cmd->Header().RequestId().has_value())
            throw std::runtime_error("CashInHandler::Handle: requestId отсутствует");

        auto events = std::make_shared<CashInEvents>(
            connection_,
            cmd->Header().RequestId().value_or(0));

        auto result = co_await HandleCashIn(cmd, events, cancel);

        auto response = std::make_shared<CashInCompletion>(
            cmd->Header().RequestId().value_or(0),
            result.payload,
            result.completionCode,
            result.errorDescription);

        co_await connection_->SendMessageAsync(response);
    }

    boost::asio::awaitable<void> CashInHandler::HandleError(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::exception_ptr commandException)
    {
        auto cmd = std::dynamic_pointer_cast<CashInCommand>(command);
        if (!cmd)
            throw std::invalid_argument("CashInHandler::HandleError: invalid CashInCommand");

        if (!cmd->Header().RequestId().has_value())
            throw std::runtime_error("CashInHandler::HandleError: requestId отсутствует");

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

        auto response = std::make_shared<CashInCompletion>(
            cmd->Header().RequestId().value(),
            nullptr,
            errorCode,
            errorDescription);

        co_await connection_->SendMessageAsync(response);
    }

    boost::asio::awaitable<CashInHandler::CommandResult>
        CashInHandler::HandleCashIn(
            std::shared_ptr<CashInCommand> cashIn,
            std::shared_ptr<ICashInEvents> events,
            std::stop_token cancel)
    {
        auto commonStatus = common_->GetCommonStatus();
        if (commonStatus &&
            commonStatus->GetExchange() ==
            XFS4IoTFramework::Common::CommonStatusClass::ExchangeEnum::Active)
        {
            auto payload = std::make_shared<CashInCompletionPayloadData>(
                CashInCompletionPayloadData::ErrorCodeEnum::ExchangeActive);

            co_return CommandResult{
                payload,
                XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
                "Включен режим инкассации."
            };
        }

        auto cashInStatus = cashAcceptor_->GetCashInStatus();
        if (!cashInStatus ||
            cashInStatus->GetStatus() !=
            XFS4IoTFramework::CashManagement::CashInStatusClass::StatusEnum::Active)
        {
            auto payload = std::make_shared<CashInCompletionPayloadData>(
                CashInCompletionPayloadData::ErrorCodeEnum::NoCashInActive);

            co_return CommandResult{
                payload,
                XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
                std::format(
                    "Состояние cash-in не активно. {}",
                    cashInStatus ? (ToString(cashInStatus->GetStatus())) : "Unknown")
            };
        }

        for (auto& [pos, presentStatus] : cashManagement_->GetLastCashManagementPresentStatus())
        {
            if (presentStatus)
            {
                presentStatus->SetTotalReturnedItems(
                    XFS4IoTFramework::Storage::StorageCashCountClass{});
            }
        }

        logger_->trace("CashAcceptorDev.CashIn()");

        const int timeout = cashIn->Header().Timeout().value_or(0);

        auto result = co_await device_->CashIn(
            std::make_shared<CashInCommandEvents>(storage_, events),
            CashInRequest(timeout),
            cancel);

        logger_->trace(std::format(
            "CashAcceptorDev.CashIn() -> {}, error_code_present={}",
            static_cast<int>(result.completionCode),
            result.errorCode.has_value() ? "true" : "false"));

        if (result.movementResult.has_value())
        {
            co_await storage_->UpdateCashAccounting(result.movementResult);
        }

		std::shared_ptr<XFS4IoT::CashManagement::StorageCashCountsClass> movement = nullptr; // это для возврата в ответе, не для хранения в статусе, так как это не cumulative, а только за эту операцию

        if (result.itemCounts.has_value() || result.unrecognized > 0)
        {
            movement =
                std::make_shared<XFS4IoT::CashManagement::StorageCashCountsClass>(
                    result.unrecognized);

            std::unordered_map<
                std::string,
                std::shared_ptr<XFS4IoT::CashManagement::StorageCashCountClass>> itemAcceptedResult;

            auto managed = cashManagement_->GetCashInStatusManaged();
            auto managedCounts = managed->GetCashCounts();
            if (!managedCounts)
            {
                managedCounts = std::make_shared<XFS4IoTFramework::Storage::StorageCashCountClass>();
                managed->SetCashCounts(managedCounts);
            }

            managedCounts->SetUnrecognized(managedCounts->GetUnrecognized() + result.unrecognized);

            for (const auto& [key, itemCount] : result.itemCounts.value_or(std::map<std::string, XFS4IoTFramework::Storage::CashItemCountClass>{}))
            {
                itemAcceptedResult.emplace(
                    key,
                    std::make_shared<XFS4IoT::CashManagement::StorageCashCountClass>(
                        itemCount.GetFit(),
                        itemCount.GetUnfit(),
                        itemCount.GetSuspect(),
                        itemCount.GetCounterfeit(),
                        itemCount.GetInked()));

                auto& managedItemCounts = managedCounts->GetItemCounts();
                auto it = managedItemCounts.find(key);

                if (it != managedItemCounts.end())
                {
                    AddCashItemCounts(it->second, itemCount);
                }
                else
                {
                    managedItemCounts.emplace(key, itemCount);
                }
            }

            movement->setExtendedProperties(std::move(itemAcceptedResult));
        }

        std::shared_ptr<CashInCompletionPayloadData> responsePayload = nullptr;

        if (result.errorCode.has_value())
        {
            responsePayload = std::make_shared<CashInCompletionPayloadData>(
                result.errorCode.value(),
                movement);
        }
        else if (movement)
        {
            responsePayload = std::make_shared<CashInCompletionPayloadData>(
                std::nullopt,
                movement);
        }

        auto managed = cashManagement_->GetCashInStatusManaged();
        cashManagement_->StoreCashInStatus();

        co_return CommandResult{
            responsePayload,
            result.completionCode,
            result.errorDescription.value_or("")
        };
    }
}
