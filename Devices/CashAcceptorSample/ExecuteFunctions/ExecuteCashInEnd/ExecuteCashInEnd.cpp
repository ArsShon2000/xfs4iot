#include "ExecuteCashInEnd.hpp"

#include "../../Managers/EscrowManager/EscrowManager.hpp"
#include "../../Managers/NotesInhibitManager/NotesInhibitManager.hpp"
#include "../../../../framework/core/Persistent/PersistentDatasHandler.hpp"

#include <chrono>
#include <format>

namespace XFS4IoTSP::CashAcceptor::Sample
{
    namespace
    {
        using DorsHW = FS365::HW::Dors::DorsHW;
        using CompletionCodeEnum = XFS4IoT::MessageHeader::CompletionCodeEnum;
        using CashInEndErrorCodeEnum =
            XFS4IoT::CashAcceptor::Completions::CashInEndCompletionPayloadData::ErrorCodeEnum;

        const StateMachine::EventTo hardwareErrorOccurred{
            {
                DorsHW::POLL_RES::DropCassetteOutOfPosition,
                DorsHW::POLL_RES::StackMotorFail,
                DorsHW::POLL_RES::TransportMotorFail,
                DorsHW::POLL_RES::InitialCassetteStatusFail,
                DorsHW::POLL_RES::OpticCanalFail,
                DorsHW::POLL_RES::MagneticCanalFail,
                DorsHW::POLL_RES::StartTrayFailure,
                DorsHW::POLL_RES::Group47UnknownFailure,
                DorsHW::POLL_RES::DropCassetteJammed,
                DorsHW::POLL_RES::ValidatorJammed,
                DorsHW::POLL_RES::PortError
            }
        };
    }

    ExecuteCashInEnd::ExecuteCashInEnd(
        std::shared_ptr<CashAcceptorSample> handler,
        std::shared_ptr<XFS4IoTFramework::CashAcceptor::CashInEndCommandEvents> events,
        std::stop_token cancellation)
        : handler_(std::move(handler))
        , events_(std::move(events))
        , cancellation_(cancellation)
    {
    }

    void ExecuteCashInEnd::InterruptRequest()
    {
        if (auto terminator = asyncTerminator_.lock())
        {
            terminator->AsyncCancel();
        }
    }

    boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashInEndResult> ExecuteCashInEnd::Execute()
    {
        if (handler_->logger_)
        {
            handler_->logger_->trace(
                std::format("{} ------------------- запуск CashAcceptor.CashInEnd -------------------", __FUNCTION__),
                LOGLEVEL1);
        }

        std::optional<CashInEndErrorCodeEnum> errorCode;
        std::string errorDescription;
        auto validation = ValidateProcessingConditions(errorCode, errorDescription);
        if (validation != CompletionCodeEnum::Success)
        {
            if (handler_->escrowManager_ &&
                errorCode != CashInEndErrorCodeEnum::NoCashInActive)
            {
                handler_->escrowManager_->CloseTransaction(
                    XFS4IoTFramework::CashManagement::CashInStatusClass::StatusEnum::Ok);
            }

            co_return XFS4IoTFramework::CashAcceptor::CashInEndResult(
                validation,
                errorDescription,
                errorCode);
        }

        auto terminator = std::make_shared<StateMachine::BlockedWaitTermination>();
        asyncTerminator_ = terminator;

        std::stop_callback cancelCallback(cancellation_, [this]()
            {
                InterruptRequest();
            });

        if (handler_->m_State == DorsHW::POLL_RES::BillStacked)
        {
            auto movement = BuildMovementAndCommit();
            UpdateStatusesAfterSuccess();

            if (handler_->m_pNotesInhibitManager_)
            {
                handler_->m_pNotesInhibitManager_->DelayInhibitAccept();
            }

            if (handler_->escrowManager_)
            {
                handler_->escrowManager_->CloseTransaction(
                    XFS4IoTFramework::CashManagement::CashInStatusClass::StatusEnum::Ok);
            }

            co_return XFS4IoTFramework::CashAcceptor::CashInEndResult(
                CompletionCodeEnum::Success,
                std::move(movement));
        }

        StateMachine::CEventsSequenceList outcomes;
        outcomes.push_back({ StateMachine::EventTo{ DorsHW::POLL_RES::BillStacked } });
        outcomes.push_back({ StateMachine::EventTo{ DorsHW::POLL_RES::Returning } });
        outcomes.push_back({ hardwareErrorOccurred });

        const auto stackResult = handler_->m_pDevice->Stack();
        if (stackResult != DorsHW::RESULT::Ok)
        {
            co_return XFS4IoTFramework::CashAcceptor::CashInEndResult(
                CompletionCodeEnum::HardwareError,
                std::format("Stack() вернул ошибку {}", static_cast<int>(stackResult)));
        }

        const auto waitResult = handler_->m_stateMachine.BlockedWait(
            outcomes,
            std::chrono::seconds(30),
            terminator);

        if (cancellation_.stop_requested())
        {
            co_return XFS4IoTFramework::CashAcceptor::CashInEndResult(
                CompletionCodeEnum::Canceled,
                "Операция CashInEnd отменена.");
        }

        switch (waitResult)
        {
        case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_0:
        {
            auto movement = BuildMovementAndCommit();
            UpdateStatusesAfterSuccess();

            if (handler_->m_pNotesInhibitManager_)
            {
                handler_->m_pNotesInhibitManager_->DelayInhibitAccept();
            }

            if (handler_->escrowManager_)
            {
                handler_->escrowManager_->CloseTransaction(
                    XFS4IoTFramework::CashManagement::CashInStatusClass::StatusEnum::Ok);
            }

            co_return XFS4IoTFramework::CashAcceptor::CashInEndResult(
                CompletionCodeEnum::Success,
                std::move(movement));
        }

        case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_1:
            co_return XFS4IoTFramework::CashAcceptor::CashInEndResult(
                CompletionCodeEnum::CommandErrorCode,
                "Устройство вернуло банкноту во время CashInEnd.",
                CashInEndErrorCodeEnum::NoItems);

        case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_2:
            if (events_)
            {
                co_await events_->storageErrorEvent(
                    XFS4IoTFramework::Storage::FailureEnum::Error,
                    { CashInStorageId() });
            }

            co_return XFS4IoTFramework::CashAcceptor::CashInEndResult(
                CompletionCodeEnum::CommandErrorCode,
                "Ошибка кассеты или замятие при CashInEnd.",
                CashInEndErrorCodeEnum::CashUnitError);

        case StateMachine::BlockedWaitResult::BWR_TIMEOUT:
            co_return XFS4IoTFramework::CashAcceptor::CashInEndResult(
                CompletionCodeEnum::TimeOut,
                "Таймаут ожидания складирования банкноты.");

        case StateMachine::BlockedWaitResult::BWR_CANCELLED:
            co_return XFS4IoTFramework::CashAcceptor::CashInEndResult(
                CompletionCodeEnum::Canceled,
                "Операция CashInEnd прервана.");

        default:
            co_return XFS4IoTFramework::CashAcceptor::CashInEndResult(
                CompletionCodeEnum::HardwareError,
                "Не удалось дождаться ожидаемого состояния CashInEnd.");
        }
    }

    CompletionCodeEnum ExecuteCashInEnd::ValidateProcessingConditions(
        std::optional<CashInEndErrorCodeEnum>& errorCode,
        std::string& errorDescription) const
    {
        if (!handler_ || !handler_->m_pDevice || !handler_->m_pDevice->IsDeviceInitialized())
        {
            errorDescription = "Устройство не инициализировано.";
            return CompletionCodeEnum::HardwareError;
        }

        if (cancellation_.stop_requested())
        {
            errorDescription = "Операция CashInEnd отменена.";
            return CompletionCodeEnum::Canceled;
        }

        if (handler_->IsCassetteMissing())
        {
            errorDescription = "Кассета приема отсутствует.";
            errorCode = CashInEndErrorCodeEnum::CashUnitError;
            return CompletionCodeEnum::CommandErrorCode;
        }

        if (!handler_->escrowManager_ || !handler_->escrowManager_->IsCashInActive())
        {
            errorDescription = "Нет активной cash-in транзакции.";
            errorCode = CashInEndErrorCodeEnum::NoCashInActive;
            return CompletionCodeEnum::CommandErrorCode;
        }

        if (handler_->acceptedItems_->empty())
        {
            errorDescription = "В cash-in транзакции нет принятых банкнот.";
            errorCode = CashInEndErrorCodeEnum::NoItems;
            return CompletionCodeEnum::CommandErrorCode;
        }

        if (handler_->m_State != DorsHW::POLL_RES::EscrowPos &&
            handler_->m_State != DorsHW::POLL_RES::Holding &&
            handler_->m_State != DorsHW::POLL_RES::BillStacked)
        {
            errorDescription = "Банкнота не находится в escrow/holding для CashInEnd.";
            errorCode = CashInEndErrorCodeEnum::NoItems;
            return CompletionCodeEnum::CommandErrorCode;
        }

        return CompletionCodeEnum::Success;
    }

    std::unordered_map<std::string, std::shared_ptr<XFS4IoTFramework::Storage::CashUnitCountClass>>
        ExecuteCashInEnd::BuildMovementAndCommit()
    {
        const auto storageId = CashInStorageId();
        auto deposited = std::make_shared<XFS4IoTFramework::Storage::StorageCashCountClass>(
            AcceptedStorageCount());

        auto cashInCount = std::make_shared<XFS4IoTFramework::Storage::StorageCashInCountClass>();
        cashInCount->SetDeposited(deposited);

        auto movementCount = std::make_shared<XFS4IoTFramework::Storage::CashUnitCountClass>(
            nullptr,
            cashInCount,
            deposited->GetTotal());

        {
            std::lock_guard lock(handler_->m_mtx);

            auto it = handler_->cashUnitInfo_.find(storageId);
            if (it != handler_->cashUnitInfo_.end())
            {
                auto existingCashIn = it->second.unitCount_.GetStorageCashInCount();
                if (!existingCashIn)
                {
                    existingCashIn = std::make_shared<XFS4IoTFramework::Storage::StorageCashInCountClass>();
                }

                auto existingDeposited = existingCashIn->GetDeposited();
                *existingDeposited += *deposited;
                existingCashIn->SetDeposited(existingDeposited);

                it->second.unitCount_.SetStorageCashInCount(existingCashIn);
                it->second.unitCount_.SetCount(it->second.unitCount_.GetCount() + deposited->GetTotal());
            }
        }

        PersistentDatasHandler::GetInstance()->addCashUnitNotes(handler_->AcceptedNotesByNoteId());
        PersistentDatasHandler::GetInstance()->resetCashInStatus();

        handler_->acceptedItems_->clear();
        handler_->currentCashInItems_.clear();
        handler_->ClearCashInLimits();

        return { { storageId, movementCount } };
    }

    XFS4IoTFramework::Storage::StorageCashCountClass ExecuteCashInEnd::AcceptedStorageCount() const
    {
        return XFS4IoTFramework::Storage::StorageCashCountClass(0, *handler_->acceptedItems_);
    }

    std::string ExecuteCashInEnd::CashInStorageId() const
    {
        if (!handler_->cashUnitInfo_.empty())
        {
            return handler_->cashUnitInfo_.begin()->first;
        }

        return "1";
    }

    void ExecuteCashInEnd::UpdateStatusesAfterSuccess()
    {
        handler_->cashAcceptorStatus_->SetIntermediateStacker(
            XFS4IoTFramework::Common::CashAcceptorStatusClass::IntermediateStackerEnum::Empty);
        handler_->cashAcceptorStatus_->SetStackerItems(
            XFS4IoTFramework::Common::CashAcceptorStatusClass::StackerItemsEnum::NoItems);

        handler_->positionStatus_.SetPositionStatus(
            XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::Empty);
        handler_->positionStatus_.SetTransportStatus(
            XFS4IoTFramework::Common::CashManagementStatusClass::TransportStatusEnum::Empty);

        auto& positions = handler_->cashAcceptorStatus_->GetPositions();
        auto position = positions.find(
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::InCenter);
        if (position != positions.end())
        {
            position->second = handler_->positionStatus_;
        }
    }
}
