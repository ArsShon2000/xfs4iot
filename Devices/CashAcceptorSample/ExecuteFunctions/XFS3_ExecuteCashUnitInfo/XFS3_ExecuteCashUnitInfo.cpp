#include "XFS3_ExecuteCashUnitInfo.hpp"
#include "CIM.PS/Managers/EscrowManager/EscrowManager.h"


using namespace STRUCT;

void XFS3_ExecuteCashUnitInfo::fillResult(LPWFSRESULT* lppWfsResult)
{
    HRESULT hResult = ValidateProcessingConditions();
    if (WFS_SUCCESS != hResult) {

        //m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}",  __FUNCTION__, XfsGetErrCodeText(hResult)), LOGLEVEL1);

        (*lppWfsResult)->hResult = hResult;
        return;
    }
}

void XFS3_ExecuteCashUnitInfo::doBasicLogic(LPWFSRESULT lpWfsResult)
{
    using enum DorsHW::POLL_RES;
    m_psHandler.m_log->trace(std::format("{} ------------------- Запуск обработчика WFS_CMD_CIM_SET_CASH_UNIT_INFO -------------------",  __FUNCTION__), LOGLEVEL1);

    lpWfsResult->hResult = ValidateProcessingConditions();

    if (WFS_SUCCESS == lpWfsResult->hResult)
    {
        // Сбрасываем все внутренние счетчики
        m_psHandler.ResetCassetteCounters();

        m_psHandler.m_ulCassetteCapacity = m_params[0].physicalCUs[0].ulMaximum;
        SettingModule::GetInstance()->setCassetteCapacity(m_params[0].physicalCUs[0].ulMaximum);
        m_psHandler.m_ulThreshold = (m_psHandler.m_ulCassetteCapacity * SettingModule::GetInstance()->getCassetteHighLevelPercent()) / 100;

        // Рассылаем сообщение WFS_SRVE_CIM_CASHUNITINFOCHANGED
        m_psHandler.SendCashUnitInfoChanged();
    }
    else {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}", __FUNCTION__, XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
        return;
    }

    m_psHandler.m_log->trace(std::format("{}() ------------------- Завершение обработчика WFS_CMD_CIM_SET_CASH_UNIT_INFO [hResult = {}] -------------------",  __FUNCTION__
        , XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1
    );
}

HRESULT XFS3_ExecuteCashUnitInfo::ValidateProcessingConditions()
{
    // Ряд проверок на возможность исполнения
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

    if (m_psHandler.m_bExchangeInProgress) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Инкассация в процессе!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_EXCHANGEACTIVE;
    }

    if (1 != m_params.size()) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Некорректный размер входных параметров!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_INVALID_DATA;
    }

    //Обрабатываем неверные входные параметры
    if (1 != m_params[0].usNumber) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Некорректный входной параметр usNumber!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_INVALIDCASHUNIT;
    }

    if (1 != m_params[0].physicalCUs.size()
        || 0 != m_params[0].physicalCUs[0].ulCashInCount
        || 0 != m_params[0].physicalCUs[0].ulCount
        || m_params[0].physicalCUs[0].ulCount != m_params[0].ulCount
        || m_params[0].physicalCUs[0].ulCashInCount != m_params[0].ulCashInCount)
    {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Некорректные входные параметры!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_INVALID_DATA;
    }

    return WFS_SUCCESS;
}
