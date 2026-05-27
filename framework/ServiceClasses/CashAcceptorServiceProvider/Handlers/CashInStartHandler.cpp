#include "CashInStartHandler.hpp"

#include <format>
#include <regex>
#include <stdexcept>
#include <unordered_map>

#include "../../../core/Exceptions.hpp"

namespace XFS4IoTFramework::CashAcceptor
{
    using CashInStartCommand =
        XFS4IoT::CashAcceptor::Commands::CashInStartCommand;

    using CashInStartCompletion =
        XFS4IoT::CashAcceptor::Completions::CashInStartCompletion;

    using CashInStartCompletionPayloadData =
        XFS4IoT::CashAcceptor::Completions::CashInStartCompletionPayloadData;

    CashInStartHandler::CashInStartHandler(
        std::shared_ptr<XFS4IoTServer::IConnection> connection,
        std::shared_ptr<XFS4IoTServer::ICommandDispatcher> dispatcher,
        std::shared_ptr<ILogger> logger)
        : connection_(std::move(connection))
        , logger_(std::move(logger))
    {
        if (!dispatcher) throw std::invalid_argument("CashInStartHandler: dispatcher is null");
        if (!connection_) throw std::invalid_argument("CashInStartHandler: connection is null");
        if (!logger_) throw std::invalid_argument("CashInStartHandler: logger is null");

        provider_ = std::dynamic_pointer_cast<XFS4IoTServer::IServiceProvider>(dispatcher);
        if (!provider_) throw std::runtime_error("CashInStartHandler: dispatcher is not IServiceProvider");

        auto deviceBase = provider_->GetDevice();
        if (!deviceBase) throw std::runtime_error("CashInStartHandler: provider device is null");

        device_ = std::dynamic_pointer_cast<ICashAcceptorDevice>(deviceBase);
        if (!device_) throw std::runtime_error("CashInStartHandler: device is not ICashAcceptorDevice");

        cashAcceptor_ = std::dynamic_pointer_cast<XFS4IoTServer::ICashAcceptorService>(provider_);
        if (!cashAcceptor_) throw std::runtime_error("CashInStartHandler: provider is not ICashAcceptorService");

        common_ = std::dynamic_pointer_cast<XFS4IoTFramework::Common::ICommonService>(provider_);
        if (!common_) throw std::runtime_error("CashInStartHandler: provider is not ICommonService");

        cashManagement_ = std::dynamic_pointer_cast<XFS4IoTFramework::CashManagement::ICashManagementService>(provider_);
        if (!cashManagement_) throw std::runtime_error("CashInStartHandler: provider is not ICashManagementService");
    }

    boost::asio::awaitable<void> CashInStartHandler::Handle(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::stop_token cancel)
    {
        auto cmd = std::dynamic_pointer_cast<CashInStartCommand>(command);
        if (!cmd) throw std::invalid_argument("CashInStartHandler::Handle: invalid command");

        if (!cmd->Header().RequestId().has_value())
            throw std::runtime_error("CashInStartHandler::Handle: requestId is missing");

        auto result = co_await HandleCashInStart(cmd, cancel);

        auto response = std::make_shared<CashInStartCompletion>(
            cmd->Header().RequestId().value(),
            result.payload,
            result.completionCode,
            result.errorDescription);

        co_await connection_->SendMessageAsync(response);
    }

    boost::asio::awaitable<void> CashInStartHandler::HandleError(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::exception_ptr commandException)
    {
        auto cmd = std::dynamic_pointer_cast<CashInStartCommand>(command);
        if (!cmd) throw std::invalid_argument("CashInStartHandler::HandleError: invalid command");

        if (!cmd->Header().RequestId().has_value())
            throw std::runtime_error("CashInStartHandler::HandleError: requestId is missing");

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

        auto response = std::make_shared<CashInStartCompletion>(
            cmd->Header().RequestId().value(),
            nullptr,
            errorCode,
            errorDescription);

        co_await connection_->SendMessageAsync(response);
    }

    boost::asio::awaitable<CashInStartHandler::CommandResult>
        CashInStartHandler::HandleCashInStart(
            std::shared_ptr<CashInStartCommand> cashInStart,
            std::stop_token cancel)
    {
        auto commonStatus = common_->GetCommonStatus();
        if (commonStatus &&
            commonStatus->GetExchange() ==
            XFS4IoTFramework::Common::CommonStatusClass::ExchangeEnum::Active)
        {
            auto payload = std::make_shared<CashInStartCompletionPayloadData>(
                CashInStartCompletionPayloadData::ErrorCodeEnum::ExchangeActive);

            co_return CommandResult{
                payload,
                XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
                "The exchange state is already in active."
            };
        }

        auto cashInStatus = cashAcceptor_->GetCashInStatus();
        if (cashInStatus &&
            cashInStatus->GetStatus() ==
            XFS4IoTFramework::CashManagement::CashInStatusClass::StatusEnum::Active)
        {
            auto payload = std::make_shared<CashInStartCompletionPayloadData>(
                CashInStartCompletionPayloadData::ErrorCodeEnum::CashInActive);

            co_return CommandResult{
                payload,
                XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
                "The cash-in state is already in active."
            };
        }

        auto payload = cashInStart->Payload;
        if (!payload)
        {
            payload = std::make_shared<XFS4IoT::CashAcceptor::Commands::CashInStartCommandPayloadData>();
        }

        using CmPosition =
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum;

        CmPosition outputPosition = CmPosition::OutDefault;

        if (payload->GetOutputPosition().has_value())
        {
            switch (payload->GetOutputPosition().value())
            {
            case XFS4IoT::CashManagement::OutputPositionEnum::outDefault:
                outputPosition = CmPosition::OutDefault; break;
            case XFS4IoT::CashManagement::OutputPositionEnum::outBottom:
                outputPosition = CmPosition::OutBottom; break;
            case XFS4IoT::CashManagement::OutputPositionEnum::outCenter:
                outputPosition = CmPosition::OutCenter; break;
            case XFS4IoT::CashManagement::OutputPositionEnum::outFront:
                outputPosition = CmPosition::OutFront; break;
            case XFS4IoT::CashManagement::OutputPositionEnum::outLeft:
                outputPosition = CmPosition::OutLeft; break;
            case XFS4IoT::CashManagement::OutputPositionEnum::outRear:
                outputPosition = CmPosition::OutRear; break;
            case XFS4IoT::CashManagement::OutputPositionEnum::outRight:
                outputPosition = CmPosition::OutRight; break;
            case XFS4IoT::CashManagement::OutputPositionEnum::outTop:
                outputPosition = CmPosition::OutTop; break;
            default:
                throw XFS4IoT::InternalErrorException("Unexpected output position specified.");
            }
        }

        auto cashAcceptorCaps = common_->GetCashAcceptorCapabilities();
        auto cashAcceptorStatus = common_->GetCashAcceptorStatus();

        if (!cashAcceptorCaps ||
            !cashAcceptorCaps->GetPositions().has_value())
        {
            co_return CommandResult{
                nullptr,
                XFS4IoT::MessageHeader::CompletionCodeEnum::InvalidData,
                std::format("Unsupported output position. {}", static_cast<int>(outputPosition))
            };
        }

        {
            auto& positions = cashAcceptorStatus->GetPositions();
            auto it = positions.find(outputPosition);

            if (it == positions.end() ||
                it->second.GetPositionStatus() ==
                XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::NotSupported)
            {
                auto responsePayload = std::make_shared<CashInStartCompletionPayloadData>(
                    CashInStartCompletionPayloadData::ErrorCodeEnum::UnsupportedPosition);

                co_return CommandResult{
                    responsePayload,
                    XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
                    std::format("The specified output position is not supported. {}", static_cast<int>(outputPosition))
                };
            }
        }

        CmPosition inputPosition = CmPosition::InDefault;

        if (payload->GetInputPosition().has_value())
        {
            switch (payload->GetInputPosition().value())
            {
            case XFS4IoT::CashManagement::InputPositionEnum::InDefault:
                inputPosition = CmPosition::InDefault; break;
            case XFS4IoT::CashManagement::InputPositionEnum::InBottom:
                inputPosition = CmPosition::InBottom; break;
            case XFS4IoT::CashManagement::InputPositionEnum::InCenter:
                inputPosition = CmPosition::InCenter; break;
            case XFS4IoT::CashManagement::InputPositionEnum::InFront:
                inputPosition = CmPosition::InFront; break;
            case XFS4IoT::CashManagement::InputPositionEnum::InLeft:
                inputPosition = CmPosition::InLeft; break;
            case XFS4IoT::CashManagement::InputPositionEnum::InRear:
                inputPosition = CmPosition::InRear; break;
            case XFS4IoT::CashManagement::InputPositionEnum::InRight:
                inputPosition = CmPosition::InRight; break;
            case XFS4IoT::CashManagement::InputPositionEnum::InTop:
                inputPosition = CmPosition::InTop; break;
            default:
                throw XFS4IoT::InternalErrorException("Unexpected input position specified.");
            }
        }

        if (!cashAcceptorCaps->GetPositions().has_value())
        {
            co_return CommandResult{
                nullptr,
                XFS4IoT::MessageHeader::CompletionCodeEnum::InvalidData,
                std::format("Unsupported input position. {}", static_cast<int>(inputPosition))
            };
        }

        {
            auto& positions = cashAcceptorStatus->GetPositions();
            auto it = positions.find(inputPosition);

            if (it == positions.end() ||
                it->second.GetPositionStatus() ==
                XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::NotSupported)
            {
                auto responsePayload = std::make_shared<CashInStartCompletionPayloadData>(
                    CashInStartCompletionPayloadData::ErrorCodeEnum::UnsupportedPosition);

                co_return CommandResult{
                    responsePayload,
                    XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
                    std::format("The specified input position is not supported. {}", static_cast<int>(inputPosition))
                };
            }
        }

		for (auto& [pos, presentStatus] : cashManagement_->GetLastCashManagementPresentStatus()) // Проверяем все позиции на наличие не возвращенных купюр, так как после начала операции все позиции должны быть пустыми
        {
            if (presentStatus) 
            {
                presentStatus->SetTotalReturnedItems(
                    XFS4IoTFramework::Storage::StorageCashCountClass{});
            }
        }

		std::optional<std::unordered_map<std::string, double>> amountLimit = std::nullopt; // По умолчанию лимит по сумме не установлен

		if (payload->GetAmountLimit().has_value()) // Если лимит по сумме установлен в запросе, то проверяем его на валидность и преобразуем в нужный формат
        {
            static const std::regex currencyRegex(R"(^[A-Z]{3}$)");

            std::unordered_map<std::string, double> limits;

            for (const auto& limit : payload->GetAmountLimit().value())
            {
                if (limit.GetValue().has_value() &&
                    limit.GetValue().value() != 0.0 &&
                    limit.GetCurrency().has_value() &&
                    !limit.GetCurrency()->empty() &&
                    std::regex_match(limit.GetCurrency().value(), currencyRegex))
                {
                    limits.emplace(limit.GetCurrency().value(), limit.GetValue().value());
                }
            }

            if (!limits.empty())
            {
                amountLimit = std::move(limits);
            }
        }

        logger_->trace("CashAcceptorDev.CashInStart()");

        auto result = co_await device_->CashInStart(
            CashInStartRequest(
                payload->GetTellerID(),
                payload->GetUseRecycleUnits().value_or(false),
                outputPosition,
                inputPosition,
                payload->GetTotalItemsLimit().value_or(0),
                amountLimit),
            cancel);

        logger_->trace(std::format(
            "CashAcceptorDev.CashInStart() -> {}, error_code_present={}",
            static_cast<int>(result.completionCode),
            result.errorCode.has_value() ? "true" : "false"));

        if (result.completionCode == XFS4IoT::MessageHeader::CompletionCodeEnum::Success)
        {
            auto managed = cashManagement_->GetCashInStatusManaged();

            managed->SetNumOfRefusedItems(0);
            managed->SetCashCounts(std::make_shared<XFS4IoTFramework::Storage::StorageCashCountClass>());
            managed->SetStatus(XFS4IoTFramework::CashManagement::CashInStatusClass::StatusEnum::Active);

            cashManagement_->StoreCashInStatus();
        }

        std::shared_ptr<CashInStartCompletionPayloadData> responsePayload = nullptr;

        if (result.errorCode.has_value())
        {
            responsePayload = std::make_shared<CashInStartCompletionPayloadData>(
                result.errorCode.value());
        }

        co_return CommandResult{
            responsePayload,
            result.completionCode,
            result.errorDescription.value_or("")
        };
    }
}