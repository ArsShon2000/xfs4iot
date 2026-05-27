#include "XFS3_ExecuteCashInRollback.hpp"
#include "XFSCommon/XfsIdentifiersToText.h"
#include "CIM.PS/Managers/EscrowManager/EscrowManager.h"
#include "CIM.PS/Managers/NotesInhibitManager/NotesInhibitManager.hpp"
#include "CIM.PS/INFO_MODULES/GetinfoBanknoteTypes/GetinfoBanknoteTypes.h"
#include <CIM.PS/INFO_MODULES/GetInfoCashUnit/GetInfoCashUnit.h>


// Некий длительный период ожидания изъятия банкнот в ходе проведения ROLLBACK'а
// По истечении, дабы не блокировать ПО, завершаем операцию
#define WAIT_FOR_ITEMSTAKEN_PERIOD		120

using namespace FS365::Utilities::XfsIdentifiersToText;

void XFS3_ExecuteCashInRollback::fillResult(LPWFSRESULT* lppWfsResult)
{
    HRESULT hResult = ValidateProcessingConditions();
    if (WFS_SUCCESS != hResult)
    {
        m_psHandler.m_pEscrowManager->CloseTransaction(WFS_CIM_CIROLLBACK);

        //m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}", __FUNCTION__, XfsGetErrCodeText(hResult)), LOGLEVEL1);

        (*lppWfsResult)->hResult = hResult;
    }
}

void XFS3_ExecuteCashInRollback::doBasicLogic(LPWFSRESULT lpWfsResult)
{
    m_psHandler.m_log->trace(std::format("{} ------------------- Запуск обработчика WFS_CMD_CIM_CASH_IN_ROLLBACK -------------------", __FUNCTION__), LOGLEVEL1);
    using enum POLL_RES;
    
    // Отключаем возможность менять лимиты 
    m_psHandler.m_bSetCashInLimitAvailable = false;

    if (m_psHandler.m_bIsSetCashInLimit)
    {
        m_psHandler.m_bIsSetCashInLimit = false;
        // Сбрасываем ранее установленные лимиты
        m_psHandler.m_limits = {};
    }

    lpWfsResult->hResult = ValidateProcessingConditions();

    if (WFS_SUCCESS == lpWfsResult->hResult)
    {
        // Возвращаем банкноту из escrow
        DorsHW::RESULT rResult = DorsHW::RESULT::Err_Illegal;
        switch (m_psHandler.m_State)
        {
        case EscrowPos:
        case Holding:
            rResult = m_psHandler.m_pDevice->Return(); break;
        }

        // Завершение транзакции (ничего не в Escrow)
        m_psHandler.m_pEscrowManager->CloseTransaction(WFS_CIM_CIROLLBACK);

        if (DorsHW::RESULT::Err_Illegal == rResult) {

            // Ок. Нет наличных к возврату
            lpWfsResult->hResult = WFS_ERR_CIM_NOITEMS;

        }
        else if (DorsHW::RESULT::Ok != rResult) {

            // Ошибка при исполнении операции
            m_psHandler.m_log->trace(std::format("{}() - ошибка при выполнении Return() = %s", __FUNCTION__, GetResultText(rResult)), LOGLEVEL1);
            lpWfsResult->hResult = WFS_ERR_HARDWARE_ERROR;

        }
        else
        {
            // Подписываемся на события машины состояний
            std::vector<XfsCommon::StateMachine<POLL_RES>::SubscriptionId_t> subs;
            std::promise<int> promiseEvent; // 1 - возвращена, 2 - ошибка
            auto futureEvent = promiseEvent.get_future();   // Получаем future
            std::once_flag flag;            // Флаг для однократного срабатывания



            auto safe_set_value = [&](int val) {
                std::call_once(flag, [&]() { promiseEvent.set_value(val); });
                };

            // Возвращена
            subs.push_back(m_psHandler.m_stateMachine.Subscribe(XfsCommon::StateMachine<POLL_RES>::EventTo{ Returning },
                [&](POLL_RES)
                {
                    safe_set_value(RESULT::RETURNING);
                }
            ));

            // Ошибка
            subs.push_back(m_psHandler.m_stateMachine.Subscribe(XfsCommon::StateMachine<POLL_RES>::EventTo{ DorsHW::GetErrorStates() },
                 [&](POLL_RES)
                {
                    safe_set_value(RESULT::HW_ERROR);
                }
            ));

            // Ожидаем завершение (состояние Stack)
            constexpr auto TIMEOUT = std::chrono::seconds(WAIT_FOR_ITEMSTAKEN_PERIOD);
            if (futureEvent.wait_for(TIMEOUT) == std::future_status::timeout)
            {
                m_psHandler.m_log->trace(std::format("{}() - таймаут ожидания завершения складирования", __FUNCTION__), LOGLEVELMSG);
                lpWfsResult->hResult = WFS_ERR_TIMEOUT;
            }
            else
            {
                switch (futureEvent.get())
                {
                case RESULT::RETURNING: // Returning
                    m_psHandler.m_log->trace(std::format("{}() - Банкнота возвращается", __FUNCTION__), LOGLEVELMSG);
                    lpWfsResult->hResult = WFS_SUCCESS;
                    break;
                case RESULT::HW_ERROR: // Hardware error
                    m_psHandler.m_log->trace(std::format("{}() - АППАРАТНАЯ ОШИБКА ПРИ ИСПОЛНЕНИИ ОПЕРАЦИИ", __FUNCTION__), LOGLEVELMSG);
                    lpWfsResult->hResult = WFS_ERR_HARDWARE_ERROR;
                    break;
                }
            }

            if (WFS_SUCCESS == lpWfsResult->hResult) {

                // Проверяем, были ли банкноты
                CimNoteNumberList noteNumberList = m_psHandler.m_pEscrowManager->GetNoteNumberList();
                if (noteNumberList.GetTotalNotesCount() > 0)
                {
                    // Заполняем CASHINFO
                    CimCashUnit cashUnit;
                    m_psHandler.FillCashInfoOnTransaction(cashUnit);// Заполняем в рамках одной транзакции
                    cashUnit.ClearBanknotes();                      // Очищаем банкноты, так как банкнота была отвергнута
                    CimCashUnitInfo cuInfo;
                    cuInfo.push_back(cashUnit);

                    lpWfsResult->hResult = cuInfo.AllocateRawXfs(lpWfsResult, &lpWfsResult->lpBuffer);
                }
            }
        }
    }
    else {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}", __FUNCTION__, XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
        return;
    }

    // Запрещаем приём банкнот
    m_psHandler.m_pNotesInhibitManager->InhibitAccept();

    m_psHandler.m_log->trace(std::format("{}() ------------------- Завершение обработчика WFS_CMD_CIM_CASH_IN_ROLLBACK [hResult = {}] -------------------", __FUNCTION__,
        XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
}


HRESULT XFS3_ExecuteCashInRollback::ValidateProcessingConditions()
{
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
        m_psHandler.m_log->error(std::format("{}() - Внимание отсутствует кассета", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_CASHUNITERROR;
    }

    // проверка была ли выполнена CASH_IN_START
    if (!m_psHandler.m_pEscrowManager->IsCashInActive()) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: WFS_CMD_CASH_IN_START не подан!", __FUNCTION__), LOGLEVEL1);
        return WFS_ERR_CIM_NOCASHINACTIVE;
    }

    return WFS_SUCCESS;
}
