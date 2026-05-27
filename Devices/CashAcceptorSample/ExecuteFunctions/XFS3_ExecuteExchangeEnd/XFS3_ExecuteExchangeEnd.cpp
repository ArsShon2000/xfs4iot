#include "XFS3_ExecuteExchangeEnd.hpp"
#include <CIM.PS/INFO_MODULES/GetInfoCashUnit/GetInfoCashUnit.h>

void XFS3_ExecuteExchangeEnd::fillResult(LPWFSRESULT* lppWfsResult)
{
    HRESULT hResult = ValidateProcessingConditions();
    if (WFS_SUCCESS != hResult) {

        //m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}",  __FUNCTION__, XfsGetErrCodeText(hResult)), LOGLEVEL1);

        (*lppWfsResult)->hResult = hResult;
        return;
    }
}

void XFS3_ExecuteExchangeEnd::doBasicLogic(LPWFSRESULT lpWfsResult)
{
    using enum DorsHW::POLL_RES;
    m_psHandler.m_log->trace(std::format("{} ------------------- Запуск обработчика WFS_CMD_CIM_END_EXCHANGE -------------------",  __FUNCTION__), LOGLEVEL1);

    lpWfsResult->hResult = ValidateProcessingConditions();
    if (WFS_SUCCESS == lpWfsResult->hResult)
    {
        m_psHandler.ResetManipulated();
        m_psHandler.m_bExchangeInProgress = false;
        m_psHandler.ResetFraudAttempts();

        if (!m_params.empty() || m_psHandler.m_bResetCassetteCounters)// разрешаем сбрасывать счетчики при открытии оперцикла, даже если команда пришла без параметров
        {
            GetInfoCashUnit::GetInstance()->update(m_params[0]);

            // Сбрасываем все внутренние счетчики
            m_psHandler.ResetCassetteCounters();

            // Рассылаем сообщение WFS_SRVE_CIM_CASHUNITINFOCHANGED
            m_psHandler.SendCashUnitInfoChanged();
        }
    }
    else {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}", __FUNCTION__, XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
        return;
    }

    if (m_psHandler.IsCassetMissing())
    {
        m_psHandler.m_log->trace(std::format("{}() - Внимание отсутствует кассета",  __FUNCTION__), LOGLEVELMSG);
        if (!m_psHandler.m_bDisableCuManipulated) lpWfsResult->hResult = WFS_ERR_CIM_CASHUNITERROR;
    }

    m_psHandler.m_log->trace(std::format("{}() ------------------- Завершение обработчика WFS_CMD_CIM_END_EXCHANGE [hResult = {}] -------------------",  __FUNCTION__
        , XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1
    );
}

HRESULT XFS3_ExecuteExchangeEnd::ValidateProcessingConditions()
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
        return WFS_ERR_DEV_NOT_READY;
    }

    if (!m_psHandler.m_bExchangeInProgress) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Инкассация в процессе!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_NOEXCHANGEACTIVE;
    }

    if (!m_params.empty())
    {
        if (1 != m_params[0].usNumber
            || 1 != m_params[0].physicalCUs.size()
            || 0 != m_params[0].physicalCUs[0].ulCashInCount
            || 0 != m_params[0].physicalCUs[0].ulCount
            || m_params[0].physicalCUs[0].ulCount != m_params[0].ulCount
            || m_params[0].physicalCUs[0].ulCashInCount != m_params[0].ulCashInCount
            || m_params[0].ulValues != 0
            || m_params[0].ulDispensedCount != 0
            || m_params[0].ulInitialCount != 0
            || m_params[0].ulPresentedCount != 0
            || m_params[0].ulRejectCount != 0
            || m_params[0].ulRetractedCount != 0
            || m_params[0].ulMinimum != 0

            || m_params[0].physicalCUs[0].ulDispensedCount != 0
            || m_params[0].physicalCUs[0].ulInitialCount != 0
            || m_params[0].physicalCUs[0].ulPresentedCount != 0
            || m_params[0].physicalCUs[0].ulRejectCount != 0
            || m_params[0].physicalCUs[0].ulRetractedCount != 0

            || m_params[0].noteNumberList.size() != 0

            || m_params[0].noteIDs.size() != 0
            )
        {
            m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Некорректные входные параметры!", __FUNCTION__), LOGLEVELMSG);
            return WFS_ERR_INVALID_DATA;
        }
    }
    return WFS_SUCCESS;
}
