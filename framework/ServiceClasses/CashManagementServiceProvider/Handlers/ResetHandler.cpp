#include "ResetHandler.hpp"

#include <format>
#include <ranges>
#include <stdexcept>
#include "../Events/ResetEvents.hpp"
#include "../../../core/Exceptions.hpp"
#include "../../HasFlagHelper.hpp"
#include "../../../server/ClientConnection.hpp"

namespace XFS4IoTFramework::CashManagement
{
    using ResetCommand = XFS4IoT::CashManagement::Commands::ResetCommand;
    using ResetCompletion = XFS4IoT::CashManagement::Completions::ResetCompletion;
    using ResetCompletionPayloadData = XFS4IoT::CashManagement::Completions::ResetCompletionPayloadData;
    using ItemTargetEnumEnum = XFS4IoT::CashManagement::ItemTargetEnumEnum;
    using ItemTargetEnum = XFS4IoTFramework::CashManagement::ItemTargetEnum;

    ResetHandler::ResetHandler(
        std::shared_ptr<XFS4IoTServer::IConnection> connection,
        std::shared_ptr<XFS4IoTServer::ICommandDispatcher> dispatcher,
        std::shared_ptr<ILogger> logger)
        : connection_(std::move(connection))
        , logger_(std::move(logger))
    {
        if (!dispatcher)
        {
            throw std::invalid_argument("ResetHandler: dispatcher is null");
        }
        if (!connection_)
        {
            throw std::invalid_argument("ResetHandler: connection is null");
        }
        if (!logger_)
        {
            throw std::invalid_argument("ResetHandler: logger is null");
        }

        provider_ = std::dynamic_pointer_cast<XFS4IoTServer::IServiceProvider>(dispatcher);
        if (!provider_)
        {
            throw std::runtime_error("ResetHandler: dispatcher is not IServiceProvider");
        }

        auto deviceBase = provider_->GetDevice();
        if (!deviceBase)
        {
            throw std::runtime_error("ResetHandler: provider device is null");
        }

        device_ = std::dynamic_pointer_cast<ICashManagementDevice>(deviceBase);
        if (!device_)
        {
            throw std::runtime_error("ResetHandler: provider device is not ICashManagementDevice");
        }

        cashManagement_ = std::dynamic_pointer_cast<ICashManagementService>(provider_);
        if (!cashManagement_)
        {
            throw std::runtime_error("ResetHandler: provider is not ICashManagementService");
        }

        common_ = std::dynamic_pointer_cast<XFS4IoTFramework::Common::ICommonService>(provider_);
        if (!common_)
        {
            throw std::runtime_error("ResetHandler: provider is not ICommonService");
        }

        storage_ = std::dynamic_pointer_cast<XFS4IoTFramework::Storage::IStorageService>(provider_);
        if (!storage_)
        {
            throw std::runtime_error("ResetHandler: provider is not IStorageService");
        }
    }

    boost::asio::awaitable<void> ResetHandler::Handle(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::stop_token cancel)
    {
        auto resetCmd = std::dynamic_pointer_cast<ResetCommand>(command);
        if (!resetCmd)
        {
            throw std::invalid_argument("ResetHandler::Handle: некорректная команда Reset");
        }

        if (!resetCmd->Header().RequestId().has_value())
        {
            throw std::runtime_error("ResetHandler::Handle: Отсутствует requestId");
        }

        auto events = std::make_shared<ResetEvents>(
            connection_,
            resetCmd->Header().RequestId().value());

        auto result = co_await HandleReset(events, resetCmd, cancel);

        auto response =
            std::make_shared<ResetCompletion>(
                resetCmd->Header().RequestId().value(),
                result.payload,
                result.completionCode,
                result.errorDescription);

        co_await connection_->SendMessageAsync(response);
    }

    boost::asio::awaitable<void> ResetHandler::HandleError(
        std::shared_ptr<XFS4IoT::MessageBase> command,
        std::exception_ptr commandException)
    {
        auto resetCmd = std::dynamic_pointer_cast<ResetCommand>(command);
        if (!resetCmd)
        {
            throw std::invalid_argument("ResetHandler::HandleError: некорректная команда Reset");
        }

        if (!resetCmd->Header().RequestId().has_value())
        {
            throw std::runtime_error("ResetHandler::HandleError: Отсутствует requestId");
        }

        XFS4IoT::MessageHeader::CompletionCodeEnum errorCode =
            XFS4IoT::MessageHeader::CompletionCodeEnum::InternalError;
        std::string errorDescription = "Неизвестная ошибка";

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
            std::make_shared<ResetCompletion>(
                resetCmd->Header().RequestId().value(),
                nullptr,
                errorCode,
                errorDescription);

        co_await connection_->SendMessageAsync(response);
    }

    boost::asio::awaitable<ResetHandler::CommandResult> ResetHandler::HandleReset(
        std::shared_ptr<IResetEvents> events,
        std::shared_ptr<ResetCommand> reset,
        std::stop_token cancel)
    {
        ItemDestination destination{};

        auto payload = reset->Payload;
        auto position = payload ? payload->GetPosition() : nullptr;

        if (position && position->getTarget().has_value())
        {
            const auto target = position->getTarget().value();

            if (target == ItemTargetEnumEnum::SingleUnit &&
                (!position->getUnit().has_value() || position->getUnit()->empty()))
            {
                co_return CommandResult{
                    nullptr,
                    XFS4IoT::MessageHeader::CompletionCodeEnum::InvalidData,
                    std::format(
                        "Указано местоположение singleUnit, но свойство целевого устройства пустое (пустая строка).")
                };
            }

            if (target == ItemTargetEnumEnum::SingleUnit &&
                position->getUnit().has_value() &&
                !storage_->GetCashUnits().contains(position->getUnit().value()))
            {
                co_return CommandResult{
                    nullptr,
                    XFS4IoT::MessageHeader::CompletionCodeEnum::InvalidData,
                    std::format("Указанное местоположение CashUnit неизвестно. {}",
                        position->getUnit().value_or(""))
                };
            }

            if (target == ItemTargetEnumEnum::SingleUnit)
            {
                destination = ItemDestination(position->getUnit().value());
            }
            else
            {
                const bool isRetractArea =
                    target == ItemTargetEnumEnum::Retract ||
                    target == ItemTargetEnumEnum::Transport ||
                    target == ItemTargetEnumEnum::Stacker ||
                    target == ItemTargetEnumEnum::ItemCassette ||
                    target == ItemTargetEnumEnum::CashIn ||
                    target == ItemTargetEnumEnum::Reject;

                if (isRetractArea)
                {
                    using RetractAreaEnum =
                        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractAreaEnum;

                    RetractAreaEnum retractArea =
                        target == ItemTargetEnumEnum::ItemCassette ? RetractAreaEnum::ItemCassette :
                        target == ItemTargetEnumEnum::CashIn ? RetractAreaEnum::CashIn :
                        target == ItemTargetEnumEnum::Retract ? RetractAreaEnum::Retract :
                        target == ItemTargetEnumEnum::Stacker ? RetractAreaEnum::Stacker :
                        target == ItemTargetEnumEnum::Reject ? RetractAreaEnum::Reject :
                        RetractAreaEnum::Transport;

                    auto cashMgmtCaps = common_->GetCashManagementCapabilities();
                    if (!cashMgmtCaps ||
                        !XFS4IoT::HasFlag(cashMgmtCaps->GetRetractAreas().value_or(XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractAreaEnum::NotSupported), retractArea))
                    {
                        auto completionPayload =
                            std::make_shared<ResetCompletionPayloadData>(
                                ResetCompletionPayloadData::ErrorCodeEnum::InvalidRetractPosition);

                        co_return CommandResult{
                            completionPayload,
                            XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
                            std::format("Указана неподдерживаемая область возврата (retract area). {}",
                                static_cast<int>(retractArea))
                        };
                    }

                    if (retractArea == RetractAreaEnum::Retract)
                    {
                        int index = 0;
                        if (!position->getIndex().has_value())
                        {
                            logger_->warn("Свойство Index равно null для назначения возврата. Используется значение по умолчанию: 0.");
                        }
                        else
                        {
                            index = position->getIndex().value();
                        }

                        if (index < 0)
                        {
                            logger_->warn(std::format(
                                "Свойство Index имеет отрицательное значение {}. Используется значение по умолчанию: 0.", index));
                            index = 0;
                        }

                        int totalRetractUnits = 0;
                        for (const auto& [unitId, unitStorage] : storage_->GetCashUnits())
                        {
                            if (!unitStorage || !unitStorage->GetUnit())
                            {
                                continue;
                            }

                            const auto types = unitStorage->GetUnit()->GetConfiguration()->GetTypes();
                            using TypesEnum = XFS4IoTFramework::Storage::CashCapabilitiesClass::TypesEnum;

                            if (XFS4IoT::HasFlag(types, TypesEnum::CashOutRetract) ||
                                XFS4IoT::HasFlag(types, TypesEnum::CashInRetract))
                            {
                                ++totalRetractUnits;
                            }
                        }

                        if (index > totalRetractUnits)
                        {
                            co_return CommandResult{
                                nullptr,
                                XFS4IoT::MessageHeader::CompletionCodeEnum::InvalidData,
                                std::format(
                                    "Неожиданное значение свойства Index для позиции возврата. {}",
                                    index)
                            };
                        }

                        destination = ItemDestination(ItemTargetEnum::Retract, index);
                    }
                    else
                    {
                        destination = ItemDestination(
                            target == ItemTargetEnumEnum::ItemCassette ? ItemTargetEnum::ItemCassette :
                            target == ItemTargetEnumEnum::CashIn ? ItemTargetEnum::CashIn :
                            target == ItemTargetEnumEnum::Stacker ? ItemTargetEnum::Stacker :
                            target == ItemTargetEnumEnum::Reject ? ItemTargetEnum::Reject :
                            ItemTargetEnum::Transport);
                    }
                }
                else
                {
                    auto cashMgmtCaps = common_->GetCashManagementCapabilities();

                    using PositionEnum =
                        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum;

                    PositionEnum positionEnum =
                        target == ItemTargetEnumEnum::OutBottom ? PositionEnum::OutBottom :
                        target == ItemTargetEnumEnum::OutCenter ? PositionEnum::OutCenter :
                        target == ItemTargetEnumEnum::OutDefault ? PositionEnum::OutDefault :
                        target == ItemTargetEnumEnum::OutFront ? PositionEnum::OutFront :
                        target == ItemTargetEnumEnum::OutLeft ? PositionEnum::OutLeft :
                        target == ItemTargetEnumEnum::OutRear ? PositionEnum::OutRear :
                        target == ItemTargetEnumEnum::OutRight ? PositionEnum::OutRight :
                        target == ItemTargetEnumEnum::OutTop ? PositionEnum::OutTop :
                        throw XFS4IoT::InvalidDataException(
                            std::format("Указана неподдерживаемая выходная позиция. {}",
                                static_cast<int>(target)));


                    destination = ItemDestination(
                        target == ItemTargetEnumEnum::OutBottom ? ItemTargetEnum::OutBottom :
                        target == ItemTargetEnumEnum::OutCenter ? ItemTargetEnum::OutCenter :
                        target == ItemTargetEnumEnum::OutDefault ? ItemTargetEnum::OutDefault :
                        target == ItemTargetEnumEnum::OutFront ? ItemTargetEnum::OutFront :
                        target == ItemTargetEnumEnum::OutLeft ? ItemTargetEnum::OutLeft :
                        target == ItemTargetEnumEnum::OutRear ? ItemTargetEnum::OutRear :
                        target == ItemTargetEnumEnum::OutRight ? ItemTargetEnum::OutRight :
                        ItemTargetEnum::OutTop);
                }
            }
        }

        for (auto& [pos, presentStatus] : cashManagement_->GetLastCashManagementPresentStatus())
        {
            if (presentStatus)
            {
                presentStatus->SetTotalReturnedItems(
                    XFS4IoTFramework::Storage::StorageCashCountClass{});
            }
        }

        logger_->trace("CashManagementDevice.ResetDeviceAsync()");

        auto commandEvents = std::make_shared<ResetCommandEvents>(
            storage_,
            events,
            [cashManagement = cashManagement_](
                XFS4IoTFramework::CashManagement::ItemTargetEnum target,
                std::optional<std::string> storageId,
                std::optional<int> index) -> boost::asio::awaitable<void>
            {
                co_await cashManagement->MediaDetectedEvent(
                    storageId.value_or(""),
                    target,
                    index);
            });

        auto result = co_await device_->ResetDeviceAsync(
            commandEvents,
            ResetDeviceRequest(destination),
            cancel);

        logger_->trace(std::format(
            "CashManagementDevice.ResetDeviceAsync() -> {}, error_code_present = {}",
            static_cast<int>(result.completionCode),
            static_cast<int>(result.error_code().value_or(XFS4IoT::CashManagement::Completions::ResetCompletionPayloadData::ErrorCodeEnum::UnsupportedPosition))));

        cashManagement_->GetCashInStatusManaged()->SetStatus(
            XFS4IoTFramework::CashManagement::CashInStatusClass::StatusEnum::Reset);
        cashManagement_->StoreCashInStatus();

        std::optional<std::unordered_map<std::string, std::shared_ptr<XFS4IoT::CashManagement::StorageCashInClass>>> itemMovementResult =
            std::nullopt;

        if (result.movement_result().has_value() && !result.movement_result()->empty())
        {
            std::unordered_map<std::string, std::shared_ptr<XFS4IoT::CashManagement::StorageCashInClass>> storageMap;

            for (const auto& [unitId, movement] : result.movement_result().value())
            {
                if (!movement.GetStorageCashInCount())
                {
                    continue;
                }

                const auto& storageCashInCount = movement.GetStorageCashInCount();

                auto convertCounts =
                    [](const auto& sourceCounts)
                    -> std::shared_ptr<XFS4IoT::CashManagement::StorageCashCountsClass>
                    {
                        auto resultCounts =
                            std::make_shared<XFS4IoT::CashManagement::StorageCashCountsClass>(
                                sourceCounts->GetUnrecognized());

                        std::unordered_map<std::string, std::shared_ptr<XFS4IoT::CashManagement::StorageCashCountClass>> ext;

                        for (const auto& [key, item] : sourceCounts->GetItemCounts())
                        {
                            ext.emplace(
                                key,
                                std::make_shared<XFS4IoT::CashManagement::StorageCashCountClass>(
                                    item.GetFit(),
                                    item.GetUnfit(),
                                    item.GetSuspect(),
                                    item.GetCounterfeit(),
                                    item.GetInked()));
                        }

                        resultCounts->setExtendedProperties(std::move(ext));
                        return resultCounts;
                    };

                storageMap.emplace(
                    unitId,
                    std::make_shared<XFS4IoT::CashManagement::StorageCashInClass>(
                        storageCashInCount->GetRetractOperations(),
                        convertCounts(storageCashInCount->GetDeposited()),
                        convertCounts(storageCashInCount->GetRetracted()),
                        convertCounts(storageCashInCount->GetRejected()),
                        convertCounts(storageCashInCount->GetDistributed()),
                        convertCounts(storageCashInCount->GetTransport())));
            }

            if (!storageMap.empty())
            {
                itemMovementResult = std::move(storageMap);
            }
        }

        if (result.movement_result().has_value())
        {
            std::unordered_map<std::string, std::shared_ptr<XFS4IoTFramework::Storage::CashUnitCountClass>> delta;

            for (const auto& [unitId, movement] : result.movement_result().value())
            {
                delta.emplace(
                    unitId,
                    std::make_shared<XFS4IoTFramework::Storage::CashUnitCountClass>(movement));
            }

            co_await storage_->UpdateCashAccounting(std::move(delta));
        }

        std::shared_ptr<ResetCompletionPayloadData> responsePayload = nullptr;
        if (result.error_code().has_value() || itemMovementResult.has_value())
        {
            responsePayload =
                std::make_shared<ResetCompletionPayloadData>(
                    result.error_code(),
                    itemMovementResult,
                    nullptr,
                    nullptr);
        }

        co_return CommandResult{
            responsePayload,
            result.completionCode,
            result.errorDescription.value_or("")
        };
    }
}