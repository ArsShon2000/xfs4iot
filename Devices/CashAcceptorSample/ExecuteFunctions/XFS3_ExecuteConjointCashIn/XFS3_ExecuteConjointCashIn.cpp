#include "XFS3_ExecuteConjointCashIn.hpp"
#include "XFSCommon/XfsIdentifiersToText.h"
#include "CIM.PS/Managers/EscrowManager/EscrowManager.h"
#include "CIM.PS/Managers/NotesInhibitManager/NotesInhibitManager.hpp"
#include "CIM.PS/INFO_MODULES/GetinfoBanknoteTypes/GetinfoBanknoteTypes.h"
#include "CIM.PS/INFO_MODULES/GetInfoCashUnit/GetInfoCashUnit.h"
#include <thread> 

using namespace FS365::Utilities::XfsIdentifiersToText;

static const StateMachine::EventTo hwErrorOccurred{
        {
            DorsHW::POLL_RES::DropCassetteOutOfPosition,
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

XFS3_ExecuteConjointCashIn::XFS3_ExecuteConjointCashIn(IHandler& handler)
    : m_iHandler(handler)
    , m_guard(dynamic_cast<DorsPSHandler&>(this->m_iHandler).m_stateMachine)
    , m_lastProcessedStateUID{ 0 }
    , m_usNoteIDAccepted{ 0 }
    , m_bHaveUnknownBanknoteJammed{ false }
    , m_bLimitReached{ false }
    , m_bCanceledByTimeout{ false }
    , m_bStackInProgress{ false }
    , banknotesTypes{ GetinfoBanknoteTypes::GetInstance()->GetPreparedData() } // Сптсок разрешенных банкнот
{
}

void XFS3_ExecuteConjointCashIn::InterruptRequest()
{
    if (auto p = m_p_async_terminator.lock())
    {
        p->AsyncCancel();
    }
}

void XFS3_ExecuteConjointCashIn::fillResult(LPWFSRESULT* lppWfsResult)
{
    HRESULT hResult = ValidateProcessingConditions();
    if (WFS_SUCCESS != hResult) {
        auto& m_psHandler = dynamic_cast<DorsPSHandler&>(this->m_iHandler);
        //m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}", __FUNCTION__, XfsGetErrCodeText(hResult)), LOGLEVEL1);

        (*lppWfsResult)->hResult = hResult;
        return;
    }
}

HRESULT XFS3_ExecuteConjointCashIn::ValidateProcessingConditions()
{
    auto& m_psHandler = dynamic_cast<DorsPSHandler&>(this->m_iHandler);
    if (m_psHandler.m_bSoftwareConfigurationFault)
    {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Девайс неправильно сконфигугрирован!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_SOFTWARE_ERROR;
    }

    if (!m_psHandler.m_pDevice->IsDeviceInitialized())
    {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Девайс не инициализирован!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_DEV_NOT_READY;
    }

    if (m_psHandler.m_bExchangeInProgress)
    {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Инкассация в процессе!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_EXCHANGEACTIVE;
    }

    if (m_psHandler.IsCassetMissing())
    {
        m_psHandler.m_log->trace(std::format("{}() - Внимание отсутствует кассета", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_CASHUNITERROR;
    }

    if (!m_psHandler.m_pEscrowManager->IsCashInActive())
    {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: WFS_CMD_CASH_IN_START не подан!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_NOCASHINACTIVE;
    }

    if (m_psHandler.m_State != DorsHW::POLL_RES::Idling &&
        m_psHandler.m_State != DorsHW::POLL_RES::UnitDisabled)
    {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: WFS_CMD_CASH_IN_START не подан!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_DEV_NOT_READY;
    }

    // Не запретил ли WFS_CMD_CIM_SET_CASH_IN_LIMIT все банкноты
    if (m_psHandler.m_bIsSetCashInLimit && m_psHandler.m_limits.ulTotalItemsLimit <= 0
        && m_psHandler.m_limits.amountLimit.ulAmount < 5) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Ограничен командой WFS_CMD_CIM_SET_CASH_IN_LIMIT!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_TOOMANYITEMS;
    }

    return WFS_SUCCESS;
}

HRESULT XFS3_ExecuteConjointCashIn::DoCancelation()
{
    auto& m_psHandler = dynamic_cast<DorsPSHandler&>(this->m_iHandler);
    return m_psHandler.m_pNotesInhibitManager->InhibitAccept()
        ? WFS_ERR_CANCELED
        : WFS_ERR_HARDWARE_ERROR;
}

void XFS3_ExecuteConjointCashIn::AddNote(LPWFSRESULT& lpWfsResult)
{
    // Получаем CimNoteTypeList с помощью itNoteId
    CimNoteNumberList noteNumberList{ {CimNoteNumber(m_usNoteIDAccepted, 1)} };
    lpWfsResult->hResult = noteNumberList.AllocateRawXfs(lpWfsResult, &lpWfsResult->lpBuffer);
}

bool XFS3_ExecuteConjointCashIn::CheckLimitAndMaybeSendInsertItems(
    std::weak_ptr<StateMachine::BlockedWaitTermination> asyncTerminator,
    bool isNeedOneBill,
    bool& limitReached)
{
    auto& m_psHandler = dynamic_cast<DorsPSHandler&>(this->m_iHandler);

    bool stop = false;
    
    if (m_psHandler.m_AllNumberList.GetTotalNotesCount() >=
        m_psHandler.m_ulCassetteCapacity)
    {
        if (m_psHandler.m_log) m_psHandler.m_log->trace(std::format("{}() - Кассета переполнена (программный порог)", __FUNCTION__), LOGLEVELMSG);
        limitReached = true;
        stop = true;
    }

    if (stop || isNeedOneBill) {
        if (auto p = asyncTerminator.lock()) {
            // Выполняем отмену асинхронно, чтобы избежать реентрантного разрушения ресурсов
            std::thread([sp = std::move(p)]() {
                try { sp->AsyncCancel(); }
                catch (...) { /* защита от исключений в отдельном потоке */ }
                }).detach();
        }
        return true;
    }
    
    
    if (auto p = asyncTerminator.lock()) {
        if (!p->IsCancelSet()) {
            m_psHandler.SendInsertItemsEvent(m_psHandler.m_curRq->hService, m_psHandler.m_curRq->reqID);
        }
    }

    return false;
}

void XFS3_ExecuteConjointCashIn::SubscribeForEscrowEvents(
    std::mutex& mtx,
    std::weak_ptr<StateMachine::BlockedWaitTermination> asyncTerminator,
    bool& isNeedOneBill,
    LPWFSRESULT& lpWfsResult)
{
    auto& m_psHandler = dynamic_cast<DorsPSHandler&>(this->m_iHandler);
    using enum DorsHW::POLL_RES;
    // Обновление последнего номинала
    m_guard.add(m_psHandler.m_stateMachine.Subscribe(
        StateMachine::EventTo{ {RejInhibit, EscrowPos, Holding} },
        [&](DorsHW::POLL_RES) {
            if (m_psHandler.m_bAdditionalRes != 0xFE) {
                // Получаем id банкноты
                    // Если есть лимиты
                std::unique_lock lk(mtx);
                m_usNoteIDAccepted = m_psHandler.m_usCurrentNoteID;
                if (m_psHandler.m_bIsSetCashInLimit)
                {
                    if (m_psHandler.m_limits.ulTotalItemsLimit > 0)
                    {
                        // Получаем CimNoteTypeList с помощью itNoteId
                        auto noteValue = banknotesTypes.FindByNoteId(m_usNoteIDAccepted);
                        // Если сумма банкноты больше чем указано в лимите
                        if (m_psHandler.m_limits.amountLimit.ulAmount < noteValue->ulValues)
                        {
                            m_psHandler.m_log->trace(std::format("{}() - Номинал вставленной банкнотs превышает лимиты установленные командой WFS_CMD_CIM_SET_CASH_IN_LIMIT", __FUNCTION__), LOGLEVELMSG);
                            m_bLimitReached = true;
                        }
                        else AddNote(lpWfsResult);
                    }
                }
                else AddNote(lpWfsResult);
            }
        }
    )
    );

    // Escrow → Stack
    m_guard.add(m_psHandler.m_stateMachine.Subscribe(
        StateMachine::EventTo{ {EscrowPos, Holding} },
        [&, asyncTerminator](DorsHW::POLL_RES) {
            isNeedOneBill = true;
            if (auto p = asyncTerminator.lock())
            {
                p->AsyncCancel();
            }
            for (int i = 0; i < 3; ++i) {
                if (i) std::this_thread::sleep_for(std::chrono::milliseconds(500));
                if (m_psHandler.m_bIsSetCashInLimit) {
                    auto noteValue = banknotesTypes.FindByNoteId(m_usNoteIDAccepted)->ulValues;
                    if (noteValue <= m_psHandler.m_limits.amountLimit.ulAmount)
                    {
                        DorsHW::RESULT res = m_psHandler.m_pDevice->Stack();
                        if (res == DorsHW::RESULT::Ok) {
                            m_bStackInProgress = true;
                            return;
                        }
                        else {
                            m_psHandler.m_log->trace(std::format("{}() - ошибка при выполнении Stack() с лимитом = {}. Прерываем исполнение. Попытка {}", __FUNCTION__, GetResultText(res), i + 1), LOGLEVELMSG);
                            return;
                        }
                    }
                }
                else
                {
                    DorsHW::RESULT res = m_psHandler.m_pDevice->Stack();
                    if (res == DorsHW::RESULT::Ok) {
                        m_bStackInProgress = true;
                        return;
                    }
                    else {
                        m_psHandler.m_log->trace(std::format("{}() - ошибка при выполнении Stack() = {}. Прерываем исполнение. Попытка {}", __FUNCTION__, GetResultText(res), i + 1), LOGLEVELMSG);
                        return;
                    }
                }
            }
            if (auto p = asyncTerminator.lock()) p->AsyncCancel();
        })
    );
}

void XFS3_ExecuteConjointCashIn::SubscribeForStackedTransition(std::mutex& mtx, LPWFSRESULT& lpWfsResult, bool& bProtectionFromPreviousReturned)
{
    auto& m_psHandler = dynamic_cast<DorsPSHandler&>(this->m_iHandler);
    using enum DorsHW::POLL_RES;
    m_guard.add(m_psHandler.m_stateMachine.Subscribe(
        StateMachine::EventFromTo{
            {BillStacked},
            {Idling, Accepting,
             DropCassetteFull, ValidatorJammed,
             UnitDisabled} },
             [&](DorsHW::POLL_RES state) {
            std::unique_lock lk(mtx);

            m_psHandler.m_log->trace(std::format("{}(): была добавлена банкнота", __FUNCTION__), LOGLEVELMSG);
            CimNoteNumberList noteNumberList{ {m_usNoteIDAccepted, 1 } };
            if (state != ValidatorJammed)
            {
                //m_psHandler.AddBanknote(m_usNoteIDAccepted);
                if (m_psHandler.m_bIsSetCashInLimit) {
                    // Вычитание принятой суммы
                    auto noteValue = banknotesTypes.FindByNoteId(m_usNoteIDAccepted)->ulValues;
                    // Если сумма банкноты больше чем указано в лимите
                    if (noteValue <= m_psHandler.m_limits.amountLimit.ulAmount)
                    {
                        // то вычитаем из лимита
                        m_psHandler.m_limits.amountLimit.ulAmount = m_psHandler.m_limits.amountLimit.ulAmount - noteValue;
                    }
                    else
                    {
                        m_psHandler.m_log->trace(std::format("{}() - Номинал вставленной банкнотs превышает лимиты установленные командой WFS_CMD_CIM_SET_CASH_IN_LIMIT.", __FUNCTION__), LOGLEVELMSG);
                        m_bLimitReached = true;
                    }
                }
                //m_psHandler.AddBanknote(m_usNoteIDAccepted);

                // Запомненные ограничения
                // Вычитание принятого количества банкнот
                m_psHandler.m_limits.ulTotalItemsLimit -= 1;
            }
            //m_psHandler.m_pEscrowManager->AddNoteNumberList(noteNumberList);
            lpWfsResult->hResult = noteNumberList.AllocateRawXfs(lpWfsResult, &lpWfsResult->lpBuffer);

            if (bProtectionFromPreviousReturned)
            {
                m_psHandler.m_log->trace(std::format("{}() - помечаем банкноту как спорную: {}", __FUNCTION__, m_usNoteIDAccepted), LOGLEVELMSG);
            }
            else
            {
                m_psHandler.m_log->trace(std::format("{}() - помечаем банкноту как успешно принятую: {}", __FUNCTION__, m_usNoteIDAccepted), LOGLEVELMSG);
            }

            m_bStackInProgress = false;
            m_usNoteIDAccepted = 0;
            m_bHaveUnknownBanknoteJammed = false;
        })
    );
}

void XFS3_ExecuteConjointCashIn::SubscribeForReturnProtection(std::mutex& mtx, bool& protectionFlag)
{
    auto& m_psHandler = dynamic_cast<DorsPSHandler&>(this->m_iHandler);
    using enum DorsHW::POLL_RES;
    StateMachine::EventsSequence_t seq =
    {
        StateMachine::EventFromTo{ DorsHW::GetRejectStates(), BillStacked },
        StateMachine::EventFromTo{ ValidatorJammed, DropCassetteFull }
    };
    m_guard.add(m_psHandler.m_stateMachine.Subscribe(seq, [&](DorsHW::POLL_RES) {
        std::unique_lock lk(mtx);
        protectionFlag = true;
        m_psHandler.m_log->trace(std::format("{}(): Сработала защита от возврата ранее складированной банкноты. Удерживаем банкноту с идентификатором номинала {}", __FUNCTION__, m_usNoteIDAccepted), LOGLEVELMSG);
        })
    );
}

void XFS3_ExecuteConjointCashIn::SubscribeForInsertItemsEvents(
    std::mutex& mtx,
    std::weak_ptr<StateMachine::BlockedWaitTermination> asyncTerminator,
    bool& isNeedOneBill,
    bool& limitReached)
{
    auto& m_psHandler = dynamic_cast<DorsPSHandler&>(this->m_iHandler);
    using enum DorsHW::POLL_RES;
     // Захватим shared_ptr логгера, чтобы он оставался живым внутри лямбд
    auto log = m_psHandler.m_log;

    auto fn = [this ,log, &m_psHandler, asyncTerminator, &isNeedOneBill, &limitReached](DorsHW::POLL_RES) {
        // m_psHandler.SendInsertItemsEvent(m_psHandler.m_curRq->hService, m_psHandler.m_curRq->reqID);
        try {
            if (!CheckLimitAndMaybeSendInsertItems(asyncTerminator, isNeedOneBill, limitReached)) {
                if (log) log->trace(std::format("{}  Прием не был отменен", __FUNCTION__), LOGLEVELMSG);
            }
        }
        catch (const std::exception& e) {
            if (log) log->trace(std::format("{}() - exception in fn: {}", __FUNCTION__, e.what()), LOGLEVELMSG);
        }
        catch (...) {
            if (log) log->trace(std::format("{}() - unknown exception in fn", __FUNCTION__), LOGLEVELMSG);
        }
    };

    auto fn2 = [&](DorsHW::POLL_RES) {
        std::unique_lock lk(mtx);
        m_usNoteIDAccepted = 0;
        };
    m_guard.add(m_psHandler.m_stateMachine.Subscribe(StateMachine::EventTo{ Idling }, fn));
    m_guard.add(m_psHandler.m_stateMachine.Subscribe(StateMachine::EventTo{ Accepting }, fn2));
}

void XFS3_ExecuteConjointCashIn::SubscribeForFeedModuleError(std::mutex& mtx)
{
    auto& m_psHandler = dynamic_cast<DorsPSHandler&>(this->m_iHandler);
    using enum DorsHW::POLL_RES;
    m_guard.add(m_psHandler.m_stateMachine.Subscribe(
        StateMachine::EventFromTo{ {Stacking}, {BillReturned} },
        [&](DorsHW::POLL_RES) {
            std::unique_lock lk(mtx);
            m_psHandler.m_log->trace(std::format("{}() - Stacking Returning occurred!", __FUNCTION__), LOGLEVELMSG);

            m_psHandler.SendCashUnitError(WFS_CIM_FEEDMODULEPROBLEM);
        })
    );
}

void XFS3_ExecuteConjointCashIn::SetBit(BYTE n, uint32_t& Val)
{
    unsigned long tmp = 1;
    Val |= (tmp << n);
}


void XFS3_ExecuteConjointCashIn::doBasicLogic(LPWFSRESULT lpWfsResult)
{
    auto& m_psHandler = dynamic_cast<DorsPSHandler&>(this->m_iHandler);
    ThreadPriorityGuard priorityGuard; // Для поднятия уровни потока (RAII)
    m_psHandler.m_log->trace(std::format("{} ------------------- Запуск обработчика WFS_CMD_CIM_CASH_IN -------------------", __FUNCTION__), LOGLEVEL1);


    using enum DorsHW::POLL_RES;
    // Локальные mutex'ы, используемые чтобы дождаться завершения обработчиков подписок
    std::mutex mtxEscrow, mtxStacked, mtxProtect, mtxInsert, mtxFeed;

    // Отключаем возможность менять лимиты при выполнении CASH_IN
    m_psHandler.m_bSetCashInLimitAvailable = false;

    // флаг: если true — операция должна принять ровно одну купюру и завершиться
    bool isNeedOneBill = false;

    bool bProtectionFromPreviousReturned = false;

    lpWfsResult->hResult = ValidateProcessingConditions();
    if (lpWfsResult->hResult != WFS_SUCCESS) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}", __FUNCTION__, XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
        return;
    }

    // Проверяем, разрешён ли приём банкнот
    if (!m_psHandler.m_pNotesInhibitManager->AllowAccept()) {
        lpWfsResult->hResult = WFS_ERR_HARDWARE_ERROR;
        return;
    }


    // ВАЖНО: в лимите может быть указан номинал банкнот, который не входит в текущий список распознаваемых купюр.
    // Точнее, меньше, чем разрешаемого номинала. Например , разрешены 5, 10, 50, 100, 200, а в лимите стоит 10.
    if (m_psHandler.m_bIsSetCashInLimit)
    {

        if (m_psHandler.m_limits.ulTotalItemsLimit == 0 || m_psHandler.m_limits.amountLimit.ulAmount < 5)
        {
            // Выработка ограничений
            m_psHandler.m_log->trace(std::format("{}() - Достигнуто ограничение приема поставленным SetCashInLimit.", __FUNCTION__), LOGLEVELMSG);
            lpWfsResult->hResult = WFS_ERR_CIM_TOOMANYITEMS;
            return;
        }
        else if (m_psHandler.m_limits.ulTotalItemsLimit < 0) {
            // Превысить ограничение можно только из-за аппараной неисправности
            m_psHandler.m_log->trace(std::format("{}() - Превышено ограничение приема поставленным SetCashInLimit.", __FUNCTION__), LOGLEVELMSG);
            lpWfsResult->hResult = WFS_ERR_HARDWARE_ERROR;
            return;
        }
    }
    else
    {
        // Лимиты не установлены - принимаем до заполнения кассеты
        // Но если в кассете мало места - ограничиваем прием

        // Получаем текущее количество свободного места в кассете
        uint32_t emptyCount = GetInfoCashUnit::GetInstance()->physicalCashUnitList[0].ulMaximum - GetInfoCashUnit::GetInstance()->physicalCashUnitList[0].ulCount;
        // Устанавливаем лимит по количеству банкнот
        m_psHandler.m_limits.ulTotalItemsLimit = emptyCount;
    }


    auto asyncTerminator = std::make_shared<StateMachine::BlockedWaitTermination>();
    m_p_async_terminator = asyncTerminator;

    BOOST_SCOPE_EXIT_ALL(&lpWfsResult, &mtxEscrow, &mtxStacked, &mtxProtect, &mtxInsert, &mtxFeed, &m_psHandler) {
        std::unique_lock l1(mtxEscrow);
        std::unique_lock l2(mtxStacked);
        std::unique_lock l3(mtxProtect);
        std::unique_lock l4(mtxInsert);
        std::unique_lock l5(mtxFeed);
        m_psHandler.m_log->trace(std::format("{}() ------------------- Завершение обработчика WFS_CMD_CIM_CASH_IN [hResult = {}] -------------------", __FUNCTION__, XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
        m_psHandler.m_pNotesInhibitManager->InhibitAccept();
    };

    // ---------------------------------------------------------------
    // Подписки на события
    SubscribeForEscrowEvents(mtxEscrow, asyncTerminator, isNeedOneBill, lpWfsResult);           // Escrow -> Stack события
    SubscribeForStackedTransition(mtxStacked, lpWfsResult, bProtectionFromPreviousReturned);    // Stacked события
    SubscribeForReturnProtection(mtxProtect, bProtectionFromPreviousReturned);                  // Защита от возврата
    SubscribeForInsertItemsEvents(mtxInsert, asyncTerminator, isNeedOneBill, m_bLimitReached);  // InsertItems события
    SubscribeForFeedModuleError(mtxFeed);                                                       // FeedModuleError события

    // ---------------------------------------------------------------
        // Основной цикл ожидания случаев: переполнение, hw error, jam/jammed
        // ---------------------------------------------------------------

    for (;;) {
        StateMachine::CEventsSequenceList arrOfOperationOutcomes;
        // 0: ANY -> DropCassetteFull
        arrOfOperationOutcomes.push_back({ StateMachine::EventTo{ { DropCassetteFull } } });
        // 1: ANY -> hardware error group
        arrOfOperationOutcomes.push_back({ hwErrorOccurred });
        // 2: ANY -> DropCassetteJammed || ValidatorJammed
        arrOfOperationOutcomes.push_back({ StateMachine::EventTo{ { DropCassetteJammed, ValidatorJammed } } });

        // BlockedWait: ждем любое из перечисленных сценариев. используем m_lastProcessedStateUID чтобы пропускать старую историю
        auto waitResult = m_psHandler.m_stateMachine.BlockedWait(
            arrOfOperationOutcomes,
            std::chrono::milliseconds(INFINITE), // INFINITE
            asyncTerminator,
            m_lastProcessedStateUID,
            &m_lastProcessedStateUID
        );

        if (waitResult == StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_0) {
            // Cassette full
            m_psHandler.m_log->trace(std::format("{}() - ПЕРЕПОЛНЕНИЕ КАССЕТЫ, ЗАВЕРШАЕМ ПРИЁМ", __FUNCTION__), LOGLEVELMSG);

            if (bProtectionFromPreviousReturned) {
                lpWfsResult->hResult = WFS_ERR_CIM_CASHUNITERROR;
                m_psHandler.SendCashUnitError(WFS_CIM_CASHUNITFULL);
            }
            else {
                lpWfsResult->hResult = WFS_SUCCESS;
            }
            break;

        }
        else if (waitResult == StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_1) {
            // Hardware error
            m_psHandler.m_log->trace(std::format("{}() - АППАРАТНАЯ ОШИБКА. ПРЕРЫВАЕМ CASH-IN ОПЕРАЦИЮ", __FUNCTION__), LOGLEVELMSG);

            lpWfsResult->hResult = WFS_ERR_HARDWARE_ERROR;
            if (m_psHandler.m_State == DropCassetteOutOfPosition || m_psHandler.m_State == StackMotorFail) {
                lpWfsResult->hResult = WFS_ERR_CIM_CASHUNITERROR;
                m_psHandler.SendCashUnitError(WFS_CIM_CASHUNITERROR);
            }
            break;

        }
        else if (waitResult == StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_2) {
            // Jam or validator jammed — на данный момент считаем как hardware error
            if (m_usNoteIDAccepted == 0) {
                m_bHaveUnknownBanknoteJammed = true;
                m_psHandler.m_log->trace(std::format("{}() - ЗАМЯТИЕ БАНКНОТЫ. ПРЕРЫВАЕМ CASH-IN ОПЕРАЦИЮ", __FUNCTION__), LOGLEVELMSG);
            }
            else {
                m_psHandler.m_log->trace(std::format("{}() - ЗАМЯТИЕ РАСПОЗНАННОЙ БАНКНОТЫ: xfsId = {}. ПРЕРЫВАЕМ CASH-IN ОПЕРАЦИЮ", __FUNCTION__, m_usNoteIDAccepted), LOGLEVEL1);
            }
            lpWfsResult->hResult = WFS_ERR_HARDWARE_ERROR;
            if (m_psHandler.m_State == DropCassetteJammed || m_psHandler.m_State == DropCassetteOutOfPosition) {
                lpWfsResult->hResult = WFS_ERR_CIM_CASHUNITERROR;
                m_psHandler.SendCashUnitError(WFS_CIM_CASHUNITERROR);
            }
            break;

        }
        else if (waitResult == StateMachine::BlockedWaitResult::BWR_CANCELLED) {

            // отмена: может быть по лимиту/timeout/вызову WFPCancelAsyncRequest
            if (m_bLimitReached)
            {
                lpWfsResult->hResult = WFS_ERR_CIM_TOOMANYITEMS;
                break;
            }
            else if (isNeedOneBill) {
                // если ConjointCashIn и нужно всего 1 банкнота — считаем успешным завершением, если удалось InhibitAccept
                if (m_psHandler.m_pNotesInhibitManager->InhibitAccept()) {
                    lpWfsResult->hResult = WFS_SUCCESS;
                }
                else {
                    lpWfsResult->hResult = WFS_ERR_HARDWARE_ERROR;
                }
                break;
            }
            else {
                if (m_bCanceledByTimeout) {
                    lpWfsResult->hResult = WFS_ERR_TIMEOUT;
                }
                // осуществляем cancel handling (InhibitAccept etc.)
                lpWfsResult->hResult = DoCancelation();
                break;
            }
        }
        else {
            // невозможно — но логируем
            m_psHandler.m_log->trace(std::format("{}() - unreachable code area", __FUNCTION__), LOGLEVELMSG);

            lpWfsResult->hResult = WFS_ERR_HARDWARE_ERROR;
            break;
        }
    } // end for loop
    //CimNoteNumberList noteNumberList = m_psHandler.m_pEscrowManager->GetNoteNumberList();
}
