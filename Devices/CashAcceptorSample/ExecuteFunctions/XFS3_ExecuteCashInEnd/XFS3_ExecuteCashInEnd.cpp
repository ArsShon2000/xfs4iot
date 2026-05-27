#include "XFS3_ExecuteCashInEnd.hpp"
#include "XFSCommon/XfsIdentifiersToText.h"
#include "CIM.PS/Managers/PowerUpManager/PowerUpManager.hpp"
#include "CIM.PS/Managers/NotesInhibitManager/NotesInhibitManager.hpp"
#include <CIM.PS/INFO_MODULES/GetInfoCashUnit/GetInfoCashUnit.h>

using namespace FS365::Utilities::XfsIdentifiersToText;
using namespace Xfs::Cim;

void XFS3_ExecuteCashInEnd::fillResult(LPWFSRESULT* lppWfsResult)
{
    HRESULT hResult = ValidateProcessingConditions();
    if (WFS_SUCCESS != hResult)
    {
        auto& m_psHandler = dynamic_cast<DorsPSHandler&>(this->m_iHandler);
        m_psHandler.m_pEscrowManager->CloseTransaction(WFS_CIM_CIOK);

        //m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}", __FUNCTION__, XfsGetErrCodeText(hResult)), LOGLEVEL1);

        (*lppWfsResult)->hResult = hResult;
    }
}

void XFS3_ExecuteCashInEnd::doBasicLogic(LPWFSRESULT lpWfsResult)
{
    auto& m_psHandler = dynamic_cast<DorsPSHandler&>(this->m_iHandler);
    using enum POLL_RES;
    m_psHandler.m_log->trace(std::format("{} ------------------- Запуск обработчика WFS_CMD_CIM_CASH_IN_END -------------------", __FUNCTION__), LOGLEVEL1);

    lpWfsResult->hResult = ValidateProcessingConditions();
    if (lpWfsResult->hResult != WFS_SUCCESS)
    {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}", __FUNCTION__, XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
        return;
    }

    auto& sm = m_psHandler.m_stateMachine; // короткий доступ к машине состояний

    switch (m_psHandler.m_State)
    {
    case EscrowPos:
    case Holding:
        // Банкнота в Escrow, можно переходить к складированию
        break;

    default:
    {

        // Проверяем, были ли банкноты
        CimNoteNumberList noteNumberList = m_psHandler.m_pEscrowManager->GetNoteNumberList();
        if (noteNumberList.GetTotalNotesCount() > 0)
        {
            lpWfsResult->hResult = WFS_SUCCESS;

            CimCashUnit cashUnit;
            m_psHandler.FillCashInfoOnTransaction(cashUnit);

            CimCashUnitInfo cuInfo;
            cuInfo.push_back(cashUnit);

            lpWfsResult->hResult = cuInfo.AllocateRawXfs(lpWfsResult, &lpWfsResult->lpBuffer);
        }
        else
        {
            lpWfsResult->hResult = WFS_ERR_CIM_NOITEMS;
        }
        // Завершение транзакции (ничего не в Escrow)
        m_psHandler.m_pEscrowManager->CloseTransaction(WFS_CIM_CIOK);

        m_psHandler.m_log->trace(std::format("{}() ------------------- Завершение обработчика WFS_CMD_CIM_CASH_IN_END [hResult = {}] -------------------", __FUNCTION__,
            XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
        return;
    }
    }

    // --- Настройка ожидания событий ---
    m_psHandler.m_bHasBeenCheated = false;

    // Сбрасываем флаги
    bool bNoteStacked = false;
    bool bNoteReturned = false;
    bool bHardwareError = false;

    // Подписываемся на события машины состояний
    std::vector<XfsCommon::StateMachine<POLL_RES>::SubscriptionId_t> subs;
    std::promise<int> promiseEvent; // 1 - складирована, 2 - возвращена, 3 - ошибка
    auto futureEvent = promiseEvent.get_future();   // Получаем future
    std::once_flag flag;            // Флаг для однократного срабатывания

    auto safe_set_value = [&](int val) {
        std::call_once(flag, [&]() { promiseEvent.set_value(val); });
        };

    // Складирована
    subs.push_back(sm.Subscribe(XfsCommon::StateMachine<POLL_RES>::EventTo{ BillStacked },
        [&](POLL_RES)
        {
            bNoteStacked = true;
            safe_set_value(1);
        }
    ));

    // Возвращена
    subs.push_back(sm.Subscribe(XfsCommon::StateMachine<POLL_RES>::EventTo{ Returning },
        [&](POLL_RES)
        {
            bNoteReturned = true;
            safe_set_value(2);
        }
    ));

    // Ошибка
    subs.push_back(sm.Subscribe(XfsCommon::StateMachine<POLL_RES>::EventTo{ DorsHW::GetErrorStates() },
        [&](POLL_RES)
        {
            bHardwareError = true;
            safe_set_value(3);
        }
    ));

    // --- Начинаем складирование ---
    m_psHandler.m_pDevice->Stack();

    // Ожидаем завершение (состояние Stack)
    constexpr auto TIMEOUT = std::chrono::seconds(30);
    if (futureEvent.wait_for(TIMEOUT) == std::future_status::timeout)
    {
        m_psHandler.m_log->trace(std::format("{}() - таймаут ожидания завершения складирования", __FUNCTION__), LOGLEVELMSG);
        lpWfsResult->hResult = WFS_ERR_TIMEOUT;
    }
    else
    {
        switch (futureEvent.get())
        {
        case 1: // BillStacked
            lpWfsResult->hResult = WFS_SUCCESS;
            break;
        case 2: // Returning
            m_psHandler.m_log->trace(std::format("{}() - StackingReturning occurred!", __FUNCTION__), LOGLEVELMSG);
            lpWfsResult->hResult = WFS_ERR_CIM_NOITEMS;
            m_psHandler.SendCashUnitError(WFS_CIM_FEEDMODULEPROBLEM);
            break;
        case 3: // Hardware error
            m_psHandler.m_log->trace(std::format("{}() - HardwareError Occurred!", __FUNCTION__), LOGLEVELMSG);
            lpWfsResult->hResult = WFS_ERR_HARDWARE_ERROR;
            if (m_psHandler.m_State == DropCassetteJammed ||
                m_psHandler.m_State == ValidatorJammed)
            {
                // Есть вероятность, что Reset может исправить замятие банкноты
                if (WFS_SUCCESS == m_psHandler.m_pPowerUpManager->Reset())
                {
                    // Reset помог.
                                    // а) переход куюроприемника в состояние 13H (Initialize), затем переход в состояние 18Н (Returning), а затем переход в состояние 19Н (Disabled), 
                                    //    означает освобождение купюропремника от замятия и успешный возврат банкноты клиенту. Зачисление банкноты на счет клиента не производится.
                                    // б) переход купюроприемника в состояние 13Н (Initialize), а затем переход в состояние 19Н (Disabled), означает успешную укладку банкноты в кэшбокс 
                                    //    купюроприемника без зачисления номинала банкноты. Зачисление банкноты на счет клиента не производится.
                                    // 
                                    // Завершаем операцию с кодом NOITEMS, даже если банкнота была складирована, т.к. производитель не рекомендует учитывать такую банкноту в статистике
                    lpWfsResult->hResult = WFS_ERR_CIM_NOITEMS;
                }
                else
                {
                    lpWfsResult->hResult = WFS_ERR_CIM_CASHUNITERROR;
                    m_psHandler.SendCashUnitError(WFS_CIM_CASHUNITERROR);
                }
            }
            else if (m_psHandler.m_State == DropCassetteOutOfPosition) {

                lpWfsResult->hResult = WFS_ERR_CIM_CASHUNITERROR;
                m_psHandler.SendCashUnitError(WFS_CIM_CASHUNITERROR);
            }

            break;
        default:
            lpWfsResult->hResult = WFS_ERR_HARDWARE_ERROR;
            break;
        }
    }

    // Отписываемся
    for (auto id : subs)
        sm.Unsubscribe(id);

    // Таймер задержки приёма
    m_psHandler.m_pNotesInhibitManager->DelayInhibitAccept();

    if (WFS_ERR_CIM_NOITEMS == lpWfsResult->hResult)
    {
        m_psHandler.m_log->critical(std::format("{}() - Нужно сделать логику ожидания забора банкот", __FUNCTION__), LOGLEVEL1);
    }

    // --- Анализ финального состояния ---
    if (lpWfsResult->hResult == WFS_SUCCESS)
    {
        switch (m_psHandler.m_State)
        {
        case Idling:
        case Accepting:
        case UnitDisabled:
        case Holding:
        case DropCassetteFull:
        case EscrowPos:
        case BillStacked:
            lpWfsResult->hResult = WFS_SUCCESS;
            break;

        case StackMotorFail:
        case DropCassetteJammed:
        case DropCassetteOutOfPosition:
            // Проблема с кассетой
            lpWfsResult->hResult = WFS_ERR_CIM_CASHUNITERROR;
            m_psHandler.SendCashUnitError(WFS_CIM_CASHUNITERROR);
            break;
        default:

            // В остальных случаях считаем операцию проваленной и выдаём
            // аппаратную ошибку
            lpWfsResult->hResult = WFS_ERR_HARDWARE_ERROR;
            break;
        }

        CimCashUnit cashUnit;
        m_psHandler.FillCashInfoOnTransaction(cashUnit);

        CimCashUnitInfo cuInfo;
        cuInfo.push_back(cashUnit);

        lpWfsResult->hResult = cuInfo.AllocateRawXfs(lpWfsResult, &lpWfsResult->lpBuffer);
    }

    // Завершение транзакции
    m_psHandler.m_pEscrowManager->CloseTransaction(WFS_CIM_CIOK);

    m_psHandler.m_log->trace(std::format("{}() ------------------- Завершение обработчика WFS_CMD_CIM_CASH_IN_END [hResult = {}] -------------------", __FUNCTION__,
        XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
}


HRESULT XFS3_ExecuteCashInEnd::ValidateProcessingConditions()
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

    return WFS_SUCCESS;
}
