#include "ExecuteCashIn.hpp"

#include "../../Managers/EscrowManager/EscrowManager.hpp"
#include "../../Managers/NotesInhibitManager/NotesInhibitManager.hpp"
#include "../../../../framework/ServiceClasses/CashAcceptorServiceProvider/CashAcceptorServiceProvider.hpp"

#include <boost/scope_exit.hpp>
#include <format>
#include <thread>

namespace XFS4IoTSP::CashAcceptor::Sample
{
    namespace
    {
        using DorsHW = FS365::HW::Dors::DorsHW;
        using CompletionCodeEnum = XFS4IoT::MessageHeader::CompletionCodeEnum;
        using CashInErrorCodeEnum =
            XFS4IoT::CashAcceptor::Completions::CashInCompletionPayloadData::ErrorCodeEnum;

        const StateMachine::EventTo hardwareErrorOccurred{
            {
                DorsHW::POLL_RES::StackMotorFail,
                DorsHW::POLL_RES::TransportMotorFail,
                DorsHW::POLL_RES::InitialCassetteStatusFail,
                DorsHW::POLL_RES::OpticCanalFail,
                DorsHW::POLL_RES::MagneticCanalFail,
                DorsHW::POLL_RES::StartTrayFailure,
                DorsHW::POLL_RES::Group47UnknownFailure,
                DorsHW::POLL_RES::PortError
            }
        };
    }

    ExecuteCashIn::ExecuteCashIn(
        std::shared_ptr<CashAcceptorSample> handler,
        std::shared_ptr<XFS4IoTFramework::CashAcceptor::CashInCommandEvents> events,
        const XFS4IoTFramework::CashAcceptor::CashInRequest& request,
        std::stop_token cancellation)
        : handler_(std::move(handler))
        , events_(std::move(events))
        , request_(request)
        , cancellation_(cancellation)
        , guard_(handler_->m_stateMachine)
    {
    }

    void ExecuteCashIn::InterruptRequest()
    {
        if (auto terminator = asyncTerminator_.lock())
        {
            terminator->AsyncCancel();
        }
    }

    boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashInResult> ExecuteCashIn::Execute()
    {
        ThreadPriorityGuard priorityGuard;

        if (handler_->logger_)
        {
            handler_->logger_->trace(
                std::format("{} ------------------- запуск CashAcceptor.CashIn -------------------", __FUNCTION__),
                LOGLEVEL1);
        }

        std::optional<CashInErrorCodeEnum> errorCode;
        std::string errorDescription;
        auto validation = ValidateProcessingConditions(errorCode, errorDescription);
        if (validation != CompletionCodeEnum::Success)
        {
            co_return XFS4IoTFramework::CashAcceptor::CashInResult(
                validation,
                errorDescription,
                errorCode);
        }

        if (events_)
        {
            co_await events_->InsertItemsEvent();
        }

        if (!handler_->m_pNotesInhibitManager_ || !handler_->m_pNotesInhibitManager_->AllowAccept())
        {
            co_return XFS4IoTFramework::CashAcceptor::CashInResult(
                CompletionCodeEnum::HardwareError,
                "Не удалось разрешить прием банкнот на устройстве.");
        }

        auto terminator = std::make_shared<StateMachine::BlockedWaitTermination>();
        asyncTerminator_ = terminator;

        std::stop_callback cancelCallback(cancellation_, [this]()
            {
                InterruptRequest();
            });

        BOOST_SCOPE_EXIT_ALL(this)
        {
            const bool cashInEndedWithItemInEscrow = handler_ && accepted_ &&
                (handler_->m_State == DorsHW::POLL_RES::EscrowPos ||
                    handler_->m_State == DorsHW::POLL_RES::Holding);

            if (handler_ && handler_->m_pNotesInhibitManager_ && !cashInEndedWithItemInEscrow)
            {
                handler_->m_pNotesInhibitManager_->InhibitAccept();
            }

            if (handler_ && handler_->logger_)
            {
                handler_->logger_->trace(
                    std::format("{} ------------------- завершение CashAcceptor.CashIn -------------------", __FUNCTION__),
                    LOGLEVEL1);
            }
        };

        SubscribeForDeviceEvents(terminator);

        StateMachine::CEventsSequenceList outcomes;
        outcomes.push_back({ StateMachine::EventTo{ { DorsHW::POLL_RES::EscrowPos, DorsHW::POLL_RES::Holding } } });
        outcomes.push_back({ StateMachine::EventTo{ DorsHW::POLL_RES::BillStacked } });
        outcomes.push_back({ StateMachine::EventTo{ { DorsHW::POLL_RES::DropCassetteFull } } });
        outcomes.push_back({ hardwareErrorOccurred });
        outcomes.push_back({ StateMachine::EventTo{ { DorsHW::POLL_RES::DropCassetteOutOfPosition } } });
        outcomes.push_back({ StateMachine::EventTo{ { DorsHW::POLL_RES::DropCassetteJammed, DorsHW::POLL_RES::ValidatorJammed } } });
        outcomes.push_back({ StateMachine::EventTo{ DorsHW::GetRejectStates() } });

        const auto waitResult = handler_->m_stateMachine.BlockedWait(
            outcomes,
            CashInTimeout(),
            terminator);

        if (cancellation_.stop_requested())
        {
            co_return XFS4IoTFramework::CashAcceptor::CashInResult(
                CompletionCodeEnum::Canceled,
                "Операция CashIn отменена.");
        }

        switch (waitResult)
        {
        case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_0:
        case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_1:
            if (!accepted_)
            {
                AddAcceptedBanknote(processedNoteId_);
            }
            co_return co_await CompleteSuccess();

        case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_2:
            co_await Refuse(RefusedReasonEnum::CashInUnitFull);
            co_await SendStorageError(XFS4IoTFramework::Storage::FailureEnum::Full);
            co_return XFS4IoTFramework::CashAcceptor::CashInResult(
                CompletionCodeEnum::CommandErrorCode,
                "Кассета приема заполнена.",
                CashInErrorCodeEnum::CashUnitError);

        case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_3:
            co_return XFS4IoTFramework::CashAcceptor::CashInResult(
                CompletionCodeEnum::HardwareError,
                "Аппаратная ошибка валидатора.");

        case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_4:
        case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_5:
            co_await SendStorageError(XFS4IoTFramework::Storage::FailureEnum::Error);
            co_return XFS4IoTFramework::CashAcceptor::CashInResult(
                CompletionCodeEnum::CommandErrorCode,
                "Ошибка кассеты приема или замятие банкноты.",
                CashInErrorCodeEnum::CashUnitError);

        case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_6:
            co_await Refuse(RefusedReasonEnum::InvalidBill);
            co_return co_await CompleteSuccess();

        case StateMachine::BlockedWaitResult::BWR_TIMEOUT:
            co_await Refuse(RefusedReasonEnum::NoBillsToDeposit);
            co_return XFS4IoTFramework::CashAcceptor::CashInResult(
                CompletionCodeEnum::CommandErrorCode,
                "За время ожидания банкноты не были внесены.",
                CashInErrorCodeEnum::NoItems);

        case StateMachine::BlockedWaitResult::BWR_CANCELLED:
            if (accepted_)
            {
                co_return co_await CompleteSuccess();
            }
            co_return XFS4IoTFramework::CashAcceptor::CashInResult(
                CompletionCodeEnum::Canceled,
                "Операция CashIn прервана.");

        default:
            co_return XFS4IoTFramework::CashAcceptor::CashInResult(
                CompletionCodeEnum::HardwareError,
                "Не удалось дождаться ожидаемого состояния устройства.");
        }
    }

    CompletionCodeEnum ExecuteCashIn::ValidateProcessingConditions(
        std::optional<CashInErrorCodeEnum>& errorCode,
        std::string& errorDescription) const
    {
        if (!handler_ || !handler_->m_pDevice || !handler_->m_pDevice->IsDeviceInitialized())
        {
            errorDescription = "Устройство не инициализировано.";
            return CompletionCodeEnum::HardwareError;
        }

        if (cancellation_.stop_requested())
        {
            errorDescription = "Операция CashIn отменена.";
            return CompletionCodeEnum::Canceled;
        }

        if (handler_->IsCassetteMissing())
        {
            errorDescription = "Кассета приема отсутствует.";
            errorCode = CashInErrorCodeEnum::CashUnitError;
            return CompletionCodeEnum::CommandErrorCode;
        }

        if (!handler_->escrowManager_ || !handler_->escrowManager_->IsCashInActive())
        {
            errorDescription = "Нет активной cash-in транзакции.";
            errorCode = CashInErrorCodeEnum::NoCashInActive;
            return CompletionCodeEnum::CommandErrorCode;
        }

        if (!handler_->IsReadyForCashIn())
        {
            errorDescription = "Текущее состояние устройства не позволяет выполнить CashIn.";
            return CompletionCodeEnum::HardwareError;
        }

        return CompletionCodeEnum::Success;
    }

    boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashInResult> ExecuteCashIn::CompleteSuccess()
    {
        UpdateInputPositionStatus(accepted_ || unrecognized_ > 0);

        if (accepted_ || unrecognized_ > 0)
        {
            if (handler_->cashAcceptorService_)
            {
                co_await handler_->cashAcceptorService_->ItemsInsertedEvent(
                    XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::InCenter);
            }
        }

        co_return XFS4IoTFramework::CashAcceptor::CashInResult(
            CompletionCodeEnum::Success,
            handler_->currentCashInItems_,
            std::unordered_map<std::string, std::shared_ptr<XFS4IoTFramework::Storage::CashUnitCountClass>>{},
            unrecognized_);
    }

    boost::asio::awaitable<void> ExecuteCashIn::Refuse(RefusedReasonEnum reason)
    {
        refused_ = true;

        if (handler_->escrowManager_)
        {
            handler_->escrowManager_->AddRefusedBanknotes(1);
        }

        if (events_)
        {
            co_await events_->InputRefuseEvent(reason);
        }
    }

    boost::asio::awaitable<void> ExecuteCashIn::SendStorageError(XFS4IoTFramework::Storage::FailureEnum failure)
    {
        if (events_)
        {
            co_await events_->StorageErrorEvent(failure, { "1" });
        }
    }

    void ExecuteCashIn::SubscribeForDeviceEvents(std::shared_ptr<StateMachine::BlockedWaitTermination> terminator)
    {
        using enum DorsHW::POLL_RES;

        guard_.add(handler_->m_stateMachine.Subscribe(
            StateMachine::EventTo{ { RejInhibit, EscrowPos, Holding } },
            [this](DorsHW::POLL_RES)
            {
                std::lock_guard lock(stateMutex_);
                processedNoteId_ = handler_->m_bAdditionalRes == 0xFE
                    ? 0
                    : (handler_->m_usCurrentNoteID != 0
                        ? handler_->m_usCurrentNoteID
                        : static_cast<uint16_t>(handler_->m_bAdditionalRes));
            }));

        guard_.add(handler_->m_stateMachine.Subscribe(
            StateMachine::EventTo{ { EscrowPos, Holding } },
            [this, terminator](DorsHW::POLL_RES)
            {
                {
                    std::lock_guard lock(stateMutex_);
                    if (processedNoteId_ == 0 && handler_->m_bAdditionalRes != 0xFE)
                    {
                        processedNoteId_ = handler_->m_usCurrentNoteID != 0
                            ? handler_->m_usCurrentNoteID
                            : static_cast<uint16_t>(handler_->m_bAdditionalRes);
                    }

                    if (!accepted_)
                    {
                        AddAcceptedBanknote(processedNoteId_);
                    }
                }

                terminator->AsyncCancel();
            }));

        guard_.add(handler_->m_stateMachine.Subscribe(
            StateMachine::EventTo{ BillStacked },
            [this, terminator](DorsHW::POLL_RES)
            {
                std::lock_guard lock(stateMutex_);
                if (!accepted_)
                {
                    AddAcceptedBanknote(processedNoteId_);
                }
                terminator->AsyncCancel();
            }));

        guard_.add(handler_->m_stateMachine.Subscribe(
            StateMachine::EventTo{ Accepting },
            [this](DorsHW::POLL_RES)
            {
                std::lock_guard lock(stateMutex_);
                processedNoteId_ = 0;
            }));
    }

    void ExecuteCashIn::AddAcceptedBanknote(uint16_t noteId)
    {
        if (accepted_)
        {
            return;
        }

        auto cashItemId = CashItemIdByNoteId(noteId);
        if (!cashItemId)
        {
            AddUnrecognizedBanknote();
            return;
        }

        const XFS4IoTFramework::Storage::CashItemCountClass itemCount(1, 0, 0, 0, 0);
        handler_->currentCashInItems_[*cashItemId] += itemCount;
        (*handler_->acceptedItems_)[*cashItemId] += itemCount;

        XFS4IoTFramework::Storage::StorageCashCountClass cashCount(0, { { *cashItemId, itemCount } });
        if (handler_->escrowManager_)
        {
            handler_->escrowManager_->AddStorageCashCount(cashCount);
        }

        accepted_ = true;

        if (handler_->logger_)
        {
            handler_->logger_->trace(
                std::format("{}() - принята банкнота noteId={}, cashItemId={}", __FUNCTION__, noteId, *cashItemId),
                LOGLEVEL1);
        }
    }

    void ExecuteCashIn::AddUnrecognizedBanknote()
    {
        ++unrecognized_;

        XFS4IoTFramework::Storage::StorageCashCountClass cashCount(1, {});
        if (handler_->escrowManager_)
        {
            handler_->escrowManager_->AddStorageCashCount(cashCount);
        }

        accepted_ = true;

        if (handler_->logger_)
        {
            handler_->logger_->trace(
                std::format("{}() - принята нераспознанная банкнота", __FUNCTION__),
                LOGLEVEL1);
        }
    }

    std::optional<std::string> ExecuteCashIn::CashItemIdByNoteId(uint16_t noteId) const
    {
        if (noteId == 0)
        {
            return std::nullopt;
        }

        for (const auto& [cashItemId, banknote] : handler_->allBanknoteIDs_)
        {
            if (banknote.GetNoteId() == noteId)
            {
                return cashItemId;
            }
        }

        return std::nullopt;
    }

    std::chrono::milliseconds ExecuteCashIn::CashInTimeout() const
    {
        if (request_.timeout > 0)
        {
            return std::chrono::milliseconds(request_.timeout);
        }

        return std::chrono::minutes(1);
    }

    void ExecuteCashIn::UpdateInputPositionStatus(bool accepted)
    {
        handler_->positionStatus_.SetPositionStatus(
            XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::Empty);
        handler_->positionStatus_.SetTransportStatus(
            XFS4IoTFramework::Common::CashManagementStatusClass::TransportStatusEnum::Empty);

        auto& positions = handler_->cashAcceptorStatus_->GetPositions();
        auto position = positions.find(XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::InCenter);
        if (position != positions.end())
        {
            position->second = handler_->positionStatus_;
        }

        if (accepted)
        {
            handler_->cashAcceptorStatus_->SetIntermediateStacker(
                XFS4IoTFramework::Common::CashAcceptorStatusClass::IntermediateStackerEnum::NotEmpty);
            handler_->cashAcceptorStatus_->SetStackerItems(
                XFS4IoTFramework::Common::CashAcceptorStatusClass::StackerItemsEnum::NoCustomerAccess);
        }
    }
}
