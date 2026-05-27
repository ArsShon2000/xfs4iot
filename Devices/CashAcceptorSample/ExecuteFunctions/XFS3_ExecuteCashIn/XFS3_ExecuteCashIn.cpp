#include "XFS3_ExecuteCashIn.hpp"
#include "CIM.PS/Managers/NotesInhibitManager/NotesInhibitManager.hpp"
#include "CIM.PS/INFO_MODULES/GetinfoBanknoteTypes/GetinfoBanknoteTypes.h"
#include "CIM.PS/Managers/PowerUpManager/PowerUpManager.hpp"
#include <boost/scope_exit.hpp>
#include "XFSCommon/XfsIdentifiersToText.h"

using namespace FS365::Utilities::XfsIdentifiersToText;

const StateMachine::EventTo failureStates{ {
        DorsHW::POLL_RES::StackMotorFail,
        DorsHW::POLL_RES::TransportMotorFail,
        DorsHW::POLL_RES::InitialCassetteStatusFail,
        DorsHW::POLL_RES::OpticCanalFail,
        DorsHW::POLL_RES::MagneticCanalFail,
        DorsHW::POLL_RES::StartTrayFailure,
        DorsHW::POLL_RES::Group47UnknownFailure,
        DorsHW::POLL_RES::PortError
    } };


void XFS3_ExecuteCashIn::fillResult(LPWFSRESULT* lppWfsResult)
{
    HRESULT hResult = ValidateProcessingConditions();
    if (WFS_SUCCESS != hResult) {
        auto& m_psHandler = dynamic_cast<DorsPSHandler&>(this->m_iHandler);
        //m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}", __FUNCTION__, XfsGetErrCodeText(hResult)), LOGLEVEL1);

        (*lppWfsResult)->hResult = hResult;
        return;
    }
}

void XFS3_ExecuteCashIn::doBasicLogic(LPWFSRESULT lpWfsResult)
{
    auto& m_psHandler = dynamic_cast<DorsPSHandler&>(this->m_iHandler);
    ThreadPriorityGuard priorityGuard; // Для поднятия уровни потока (RAII)
    m_psHandler.m_log->trace(std::format("{} ------------------- Запуск обработчика WFS_CMD_CIM_CASH_IN -------------------", __FUNCTION__), LOGLEVEL1);

    BOOST_SCOPE_EXIT(&lpWfsResult, &m_psHandler) {
        m_psHandler.m_log->trace(std::format("{}() ------------------- Завершение обработчика WFS_CMD_CIM_CASH_IN [hResult = {}] -------------------", __FUNCTION__, XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
    } BOOST_SCOPE_EXIT_END;

    lpWfsResult->hResult = ValidateProcessingConditions();
    if (WFS_SUCCESS != lpWfsResult->hResult) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}", __FUNCTION__, XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
        return;
    }


    // Механизм асинхронной отмены ожидания
    auto pTerminator = std::make_shared< StateMachine::BlockedWaitTermination >();
    m_p_async_terminator = pTerminator;

    // Индекс текущей обрабатываемой банкноты
    USHORT usProcessedNoteId = 0;


    // ANY -> Idling
    m_guard.add(m_psHandler.m_stateMachine.Subscribe(StateMachine::EventTo{ DorsHW::POLL_RES::Idling },
        [&](DorsHW::POLL_RES) {
            // Если перед Idling банкнота была взята событие SendInsertItemsEvent происходил первее, так как код в
            // DorsPSHandler.cpp выполнялся позднее. Поэтому SendItemsTaken перенес сюда.
            // Данная проблема была в старой версии драйвера
            if (m_psHandler.m_bNotesArePresented)
            {
                m_psHandler.SendItemsTaken();
            }
            // Устройство готово к приему банкнот, отправляем соответствующее событие
            m_psHandler.SendInsertItemsEvent(m_psHandler.m_curRq->hService, m_psHandler.m_curRq->reqID);
        })
    );

    // ANY -> Accepting
    m_guard.add(m_psHandler.m_stateMachine.Subscribe(StateMachine::EventTo{ DorsHW::POLL_RES::Accepting },
        [&usProcessedNoteId](DorsHW::POLL_RES) {
            // Идет вставка новой банкноты, сбрасываем распознанную ранее (если таковая была)
            usProcessedNoteId = 0;
        })
    );

    // ANY -> Rejecting (Inhibit)
    m_guard.add(m_psHandler.m_stateMachine.Subscribe(StateMachine::EventTo{ DorsHW::POLL_RES::RejInhibit },
        [&](DorsHW::POLL_RES) {
            // Банкнота отбракована
            // DBA: номинал известен
            if (m_psHandler.m_bAdditionalRes != 0xFE) {
                // Получаем id банкноты
                auto itNoteId = m_psHandler.m_denominationsPhysicalToLogical.find(m_psHandler.m_bAdditionalRes);
                if (itNoteId != m_psHandler.m_denominationsPhysicalToLogical.end()) {
                    usProcessedNoteId = itNoteId->second;
                }
            }
        })
    );

    // Rejecting -> BillStacked
    bool bProtectionFromPreviousReturned{ false };
    m_guard.add(m_psHandler.m_stateMachine.Subscribe(
        {
            StateMachine::EventFromTo{ DorsHW::GetRejectStates(), DorsHW::POLL_RES::BillStacked },
            StateMachine::EventFromTo{ DorsHW::POLL_RES::BillStacked, { { DorsHW::POLL_RES::ValidatorJammed, DorsHW::POLL_RES::DropCassetteFull } } }
        },
        [&](DorsHW::POLL_RES) {
            m_psHandler.m_log->trace(std::format("{}() - Сработала защита от возврата ранее складированной банкноты. Удерживаем банкноту с идентификатором номинала {}", __FUNCTION__, usProcessedNoteId), LOGLEVEL1);
            bProtectionFromPreviousReturned = true;
        })
    );

    //ANY -> BillStacked
    m_guard.add(m_psHandler.m_stateMachine.Subscribe(StateMachine::EventTo{ DorsHW::POLL_RES::BillStacked },
        [](DorsHW::POLL_RES) {
        })
    );
    //

         // Initialize -> Returning
    bool bReturningAfterReset = false;
    m_guard.add(m_psHandler.m_stateMachine.Subscribe(StateMachine::EventFromTo{ DorsHW::POLL_RES::Initialize, DorsHW::POLL_RES::Returning },
        [&](DorsHW::POLL_RES) {
            // Идет возврат банкноты после выполнения сброса
            bReturningAfterReset = true;
        })
    );



       // Включаем приём купюр всех валют и номиналов
    if (!m_psHandler.m_pNotesInhibitManager->AllowAccept()) {
        lpWfsResult->hResult = WFS_ERR_HARDWARE_ERROR;
        return;
    }

    StateMachine::CEventsSequenceList expectedEvents;
    {
        // ANY -> ESCROW
        expectedEvents.push_back({ StateMachine::EventTo{ { DorsHW::POLL_RES::EscrowPos, DorsHW::POLL_RES::Holding } } });

        // ANY -> FAILURE
        expectedEvents.push_back({ StateMachine::EventTo{ failureStates } });

        // ANY -> CASSETTE MISSING
        expectedEvents.push_back({ StateMachine::EventTo{ DorsHW::POLL_RES::DropCassetteOutOfPosition } });

        // ANY -> JAMMED
        expectedEvents.push_back({ StateMachine::EventTo{ { DorsHW::POLL_RES::ValidatorJammed, DorsHW::POLL_RES::DropCassetteJammed } } });

        // ANY -> CASSETTE_FULL
        expectedEvents.push_back({ StateMachine::EventTo{ DorsHW::POLL_RES::DropCassetteFull } });

        // ANY -> BillStacked
        expectedEvents.push_back({ StateMachine::EventTo{ DorsHW::POLL_RES::BillStacked } });

    }

    auto waitResult = m_psHandler.m_stateMachine.BlockedWait(expectedEvents, std::chrono::milliseconds(INFINITE), pTerminator);

    switch (waitResult)
    {
    case StateMachine::BlockedWaitResult::BWR_CANCELLED:
    {
        // Запросили отмену
        if (!m_psHandler.m_curRq->bCanceledProcessionByTimeout)
            lpWfsResult->hResult = WFS_ERR_CANCELED;
        else
            lpWfsResult->hResult = WFS_ERR_TIMEOUT;

        // Отключаем прием купюр
        m_psHandler.m_pNotesInhibitManager->InhibitAccept();
    } break;
    case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_0:
    {
        // ESCROW / HOLDING
        CimNoteNumberList noteNumberList{ {m_psHandler.m_usCurrentNoteID, 1 } };
        m_psHandler.m_pEscrowManager->AddNoteNumberList(noteNumberList);

        lpWfsResult->hResult = noteNumberList.AllocateRawXfs(lpWfsResult, &lpWfsResult->lpBuffer);
    } break;
    case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_1:
    {
        // FAILURE
        m_psHandler.m_log->trace(std::format("{}() - АППАРАТНАЯ ОШИБКА. ПРЕРЫВАЕМ CASH-IN ОПЕРАЦИЮ", __FUNCTION__), LOGLEVEL1);

        lpWfsResult->hResult = WFS_ERR_HARDWARE_ERROR;

        // Отключаем прием купюр
        m_psHandler.m_pNotesInhibitManager->InhibitAccept();

    } break;
    case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_2:
    {
        // CASSETTE MISSING
        m_psHandler.m_log->trace(std::format("{}() - КАССЕТА ИЗВЛЕЧЕНА. ПРЕРЫВАЕМ CASH-IN ОПЕРАЦИЮ", __FUNCTION__), LOGLEVEL1);
        lpWfsResult->hResult = WFS_ERR_CIM_CASHUNITERROR;

        // + Генерируем событие 
        m_psHandler.SendCashUnitError(WFS_CIM_CASHUNITERROR);

        m_psHandler.m_pNotesInhibitManager->InhibitAccept();
    } break;
    case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_3:
    {
        // JAMMED
        if (usProcessedNoteId == 0) {
            m_psHandler.m_log->trace(std::format("{}() - ЗАМЯТИЕ БАНКНОТЫ. ПРЕРЫВАЕМ CASH-IN ОПЕРАЦИЮ", __FUNCTION__), LOGLEVEL1);
        }
        else {
            m_psHandler.m_log->trace(std::format("{}() - ЗАМЯТИЕ РАСПОЗНАННОЙ БАНКНОТЫ: xfsId = {}. ПРЕРЫВАЕМ CASH-IN ОПЕРАЦИЮ", __FUNCTION__, usProcessedNoteId), LOGLEVEL1);
        }
        lpWfsResult->hResult = WFS_ERR_HARDWARE_ERROR;

        if (!bReturningAfterReset) {
            // Добавляем в список банкнот текущей транзакции.
            // С целью учета замятой банкноты как спорной на уровне прикладного ПО.
            CimNoteNumberList noteNumberList{ CimNoteNumber(usProcessedNoteId, 1) };
            m_psHandler.m_pEscrowManager->AddNoteNumberList(noteNumberList);

            if (bProtectionFromPreviousReturned) {
                // Кроме прочего, мы знаем, что банкнота попала в кассету - добавляем в статистику принятых
                m_psHandler.AddBanknote(usProcessedNoteId);
            }
        }

        m_psHandler.m_pNotesInhibitManager->InhibitAccept();
        //m_psHandler.m_pLedCtrl->LedOff();
    } break;
    case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_4:
    {
        // CASSETTE_FULL
        m_psHandler.m_log->trace(std::format("{}() - Переполнение кассеты. Операция прервана", __FUNCTION__), LOGLEVEL1);
        lpWfsResult->hResult = WFS_ERR_HARDWARE_ERROR;

        // + Генерируем событие 
        m_psHandler.SendCashUnitError(WFS_CIM_CASHUNITFULL);

        m_psHandler.m_pNotesInhibitManager->InhibitAccept();

        if (bProtectionFromPreviousReturned) {

            // Добавляем в список банкнот текущей транзакции.
            // С целью учета банкноты как спорной на уровне прикладного ПО.
            CimNoteNumberList noteNumberList{ CimNoteNumber(usProcessedNoteId, 1) };
            m_psHandler.m_pEscrowManager->AddNoteNumberList(noteNumberList);

            // Кроме прочего, мы знаем, что банкнота попала в кассету - добавляем в статистику принятых
            m_psHandler.AddBanknote(usProcessedNoteId);
        }
    } break;
    //case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_5:
    //{
    //    // ANY -> BillStacked
    //    CimNoteNumberList noteNumberList{ {m_psHandler.m_usCurrentNoteID, 1 } };
    //    m_psHandler.m_pEscrowManager->AddNoteNumberList(noteNumberList);

    //    m_psHandler.AddBanknote(m_psHandler.m_usCurrentNoteID);

    //    lpWfsResult->hResult = noteNumberList.AllocateRawXfs(lpWfsResult, &lpWfsResult->lpBuffer);
    //    m_psHandler.m_pNotesInhibitManager->InhibitAccept();
    //    //m_psHandler.m_pLedCtrl->LedOff();
    //} break;
    default:
        break;
    }
}

HRESULT XFS3_ExecuteCashIn::ValidateProcessingConditions()
{
    auto& m_psHandler = dynamic_cast<DorsPSHandler&>(this->m_iHandler);
    if (m_psHandler.m_bSoftwareConfigurationFault) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Девайс неправильно сконфигугрирован!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_SOFTWARE_ERROR;
    }

    if (!m_psHandler.m_pDevice->IsDeviceInitialized()) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Девайс не инициализирован!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_DEV_NOT_READY;
    }

    if (m_psHandler.m_bExchangeInProgress) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Инкассация в процессе!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_EXCHANGEACTIVE;
    }

    if (m_psHandler.IsCassetMissing())
    {
        m_psHandler.m_log->trace(std::format("{}() - Внимание отсутствует кассета", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_CASHUNITERROR;
    }

    // проверка была ли выполнена CASH_IN_START
    if (!m_psHandler.m_pEscrowManager->IsCashInActive()) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: WFS_CMD_CASH_IN_START не подан!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_NOCASHINACTIVE;
    }

    // операция CASH_IN уже могла быть выполнена
    if (WFS_CIM_ISFULL == m_psHandler.m_pEscrowManager->GetIntermediateStackerPartialState()) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Промежуточный не пустой!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_TOOMANYITEMS;
    }

    //// Не запретил ли WFS_CMD_CIM_SET_CASH_IN_LIMIT все банкноты
    //if (m_psHandler.m_limits.ulTotalItemsLimit > 0
    //    && m_psHandler.m_limits.amountLimit.ulAmount < 5) {
    //    return WFS_ERR_CIM_TOOMANYITEMS;
    //}

    return WFS_SUCCESS;
}

void XFS3_ExecuteCashIn::SetBit(BYTE n, uint32_t& Val)
{
    unsigned long tmp = 1;
    Val |= (tmp << n);
}
