#include "XFS3_ExecuteConjointCashInEnd.hpp"
#include "XFSCommon/XfsIdentifiersToText.h"
#include "CIM.PS/Managers/PowerUpManager/PowerUpManager.hpp"
#include "CIM.PS/Managers/NotesInhibitManager/NotesInhibitManager.hpp"
#include <CIM.PS/INFO_MODULES/GetInfoCashUnit/GetInfoCashUnit.h>

using namespace FS365::Utilities::XfsIdentifiersToText;
using namespace Xfs::Cim;

void XFS3_ExecuteConjointCashInEnd::fillResult(LPWFSRESULT* lppWfsResult)
{
    HRESULT hResult = ValidateProcessingConditions();
    if (WFS_SUCCESS != hResult)
    {
        m_psHandler.m_pEscrowManager->CloseTransaction(WFS_CIM_CIOK);

        //m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}",  __FUNCTION__, XfsGetErrCodeText(hResult)), LOGLEVEL1);

        (*lppWfsResult)->hResult = hResult;
    }
}

void XFS3_ExecuteConjointCashInEnd::doBasicLogic(LPWFSRESULT lpWfsResult)
{
    using enum POLL_RES;
    m_psHandler.m_log->trace(std::format("{} ------------------- Запуск обработчика WFS_CMD_CIM_CASH_IN_END -------------------",  __FUNCTION__), LOGLEVEL1);

    // Отключаем возможность менять лимиты 
    m_psHandler.m_bSetCashInLimitAvailable = false;

    lpWfsResult->hResult = ValidateProcessingConditions();
    if (lpWfsResult->hResult != WFS_SUCCESS)
    {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}", __FUNCTION__, XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
        return;
    }

    if (WFS_SUCCESS == lpWfsResult->hResult) {

        // Проверяем, были ли банкноты
        CimNoteNumberList noteNumberList = m_psHandler.m_pEscrowManager->GetNoteNumberList();
        if (noteNumberList.GetTotalNotesCount() > 0)
        {
            // Заполняем CASHINFO
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

    }

    if (m_psHandler.m_bIsSetCashInLimit)
    {
        m_psHandler.m_bIsSetCashInLimit = false;
        // Сбрасываем ранее установленные лимиты
        m_psHandler.m_limits = {};
    }

    m_psHandler.m_log->trace(std::format("{}() ------------------- Завершение обработчика WFS_CMD_CIM_CASH_IN_END [hResult = {}] -------------------",  __FUNCTION__,
        XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
}


HRESULT XFS3_ExecuteConjointCashInEnd::ValidateProcessingConditions()
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

    // проверка была ли выполнена CASH_IN_START
    if (!m_psHandler.m_pEscrowManager->IsCashInActive()) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: WFS_CMD_CASH_IN_START не подан!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_NOCASHINACTIVE;
    }

    return WFS_SUCCESS;
}
