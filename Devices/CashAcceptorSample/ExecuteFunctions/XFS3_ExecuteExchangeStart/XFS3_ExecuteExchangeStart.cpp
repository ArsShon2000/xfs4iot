#include "XFS3_ExecuteExchangeStart.hpp"
#include "MODULE_STRUCTURES/CimCurrencyExp/CimCurrencyExp.h"
#include "CIM.PS/Managers/PowerUpManager/PowerUpManager.hpp"
#include <CIM.PS/INFO_MODULES/GetInfoCashUnit/GetInfoCashUnit.h>

using namespace STRUCT;

void XFS3_ExecuteExchangeStart::fillResult(LPWFSRESULT* lppWfsResult)
{
    HRESULT hResult = ValidateProcessingConditions();
    if (WFS_SUCCESS != hResult) {

        //m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}",  __FUNCTION__, XfsGetErrCodeText(hResult)), LOGLEVEL1);

        (*lppWfsResult)->hResult = hResult;
        return;
    }
}

void XFS3_ExecuteExchangeStart::doBasicLogic(LPWFSRESULT lpWfsResult)
{
    using enum DorsHW::POLL_RES;
    m_psHandler.m_log->trace(std::format("{} ------------------- Запуск обработчика WFS_CMD_CIM_START_EXCHANGE -------------------",  __FUNCTION__), LOGLEVEL1);

    lpWfsResult->hResult = ValidateProcessingConditions();
    if (WFS_SUCCESS == lpWfsResult->hResult)
    {
        // Если на транспорте бакннота, то необходим Reset,
        // который либо вернет банкноту обратно, либо складирует в кассету
        switch (m_psHandler.m_State)
        {
        case PowerUpWithBillInValidator:
        case PowerUpWithBillInStacker:
        {
            // Выполнение Reset без учета стратегии
            // Цель: убрать банкноту с транспорта, либо наружу, либо в кассету
            lpWfsResult->hResult = m_psHandler.m_pPowerUpManager->Reset();
        } break;
        default:
            break;
        }

        if (WFS_SUCCESS == lpWfsResult->hResult)
        {
            m_psHandler.m_bExchangeInProgress = true;
            CimCashUnit cashUnit;
            GetInfoCashUnit::GetInstance()->fill_XFS3_Result(cashUnit);

            CimCashUnitInfo cuInfo;
            cuInfo.push_back(cashUnit);

            lpWfsResult->hResult = cuInfo.AllocateRawXfs(lpWfsResult, &lpWfsResult->lpBuffer);
        }
    }
    else {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}", __FUNCTION__, XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
        return;
    }

    m_psHandler.m_log->trace(std::format("{}() ------------------- Завершение обработчика WFS_CMD_CIM_START_EXCHANGE [hResult = {}] -------------------",  __FUNCTION__
        , XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1
    );
}

HRESULT XFS3_ExecuteExchangeStart::ValidateProcessingConditions()
{
    if (m_psHandler.m_bSoftwareConfigurationFault) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Девайс неправильно сконфигугрирован!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_SOFTWARE_ERROR;
    }

    if (!m_psHandler.m_pDevice->IsDeviceInitialized()) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Девайс не инициализирован!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_DEV_NOT_READY;
    }

    if (m_psHandler.m_pEscrowManager->IsCashInActive()) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: WFS_CMD_CASH_IN_START подан!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_CASHINACTIVE;
    }

    if (m_psHandler.m_bExchangeInProgress) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Инкассация в процессе!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_EXCHANGEACTIVE;
    }

    //// Обрабатываем неверные входные параметры
    //if (WFS_CIM_EXBYHAND != m_params.fwExchangeType || 1 < m_params.cuNumList.size() || 1 != m_params.cuNumList[0]) {
    //    return WFS_ERR_UNSUPP_DATA;
    //}

    // Обрабатываем неверные входные параметры
    if (WFS_CIM_EXBYHAND != m_params.fwExchangeType || 1 != m_params.cuNumList.size() || 1 != m_params.cuNumList[0]) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Некорректные входные параметры!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_INVALID_DATA;
    }

    return WFS_SUCCESS;
}
