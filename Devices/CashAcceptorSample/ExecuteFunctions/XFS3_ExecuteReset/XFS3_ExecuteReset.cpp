#include "XFS3_ExecuteReset.hpp"
#include "CIM.PS/Managers/NotesInhibitManager/NotesInhibitManager.hpp"
#include "CIM.PS/Managers/PowerUpManager/PowerUpManager.hpp"

void XFS3_ExecuteReset::fillResult(LPWFSRESULT* lppWfsResult)
{
    // Если активна сессия приема наличных - завершаем в любом случае
    if (m_psHandler.m_pEscrowManager->IsCashInActive()) {
        m_psHandler.m_pEscrowManager->CloseTransaction(WFS_CIM_CIRESET);
    }

    // Если активна exchange-сессия кассет - завершаем в любом случае
    if (m_psHandler.m_bExchangeInProgress) {
        m_psHandler.m_bExchangeInProgress = false;
    }

    HRESULT hResult = ValidateProcessingConditions();
    if (WFS_SUCCESS != hResult) {

        //m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}",  __FUNCTION__, XfsGetErrCodeText(hResult)), LOGLEVEL1);

        (*lppWfsResult)->hResult = hResult;
    }
}

void XFS3_ExecuteReset::doBasicLogic(LPWFSRESULT lpWfsResult)
{
    using enum DorsHW::POLL_RES;
    m_psHandler.m_log->trace(std::format("{} ------------------- Запуск обработчика WFS_CMD_CIM_RESET -------------------",  __FUNCTION__), LOGLEVEL1);

    lpWfsResult->hResult = ValidateProcessingConditions();
    if (WFS_SUCCESS == lpWfsResult->hResult)
    {
        lpWfsResult->hResult = WFS_ERR_HARDWARE_ERROR;

        // Выполняем Reset и проверяем наличие банкноты внутри устройства
        // Если банкнота обнаружена - выдаем ее обратно

        if (m_psHandler.m_State == Holding) {
            CimItemPosition itemPosition;
            itemPosition.fwOutputPosition = WFS_CIM_POSOUTCENTER;
            m_psHandler.SendMediaDetected(itemPosition);
        }
        m_psHandler.m_pNotesInhibitManager->InhibitAccept();
        lpWfsResult->hResult = m_psHandler.m_pPowerUpManager->Reset();
    }
    else {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}", __FUNCTION__, XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
        return;
    }

    m_psHandler.m_log->trace(std::format("{}() ------------------- Завершение обработчика WFS_CMD_CIM_RESET [hResult = {}] -------------------",  __FUNCTION__
        , XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1
    );
}


HRESULT XFS3_ExecuteReset::ValidateProcessingConditions()
{
    using enum DorsHW::POLL_RES;

    if (m_psHandler.m_bSoftwareConfigurationFault) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Девайс неправильно сконфигугрирован!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_SOFTWARE_ERROR;
    }

    if (!m_psHandler.m_pDevice->IsDeviceInitialized()) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Девайс не инициализирован!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_DEV_NOT_READY;
    }

    // проверка корректности ввода
    if (m_ptrParams.get() != NULL)
    {
        if (m_ptrParams->fwOutputPosition != WFS_CIM_POSNULL && m_ptrParams->fwOutputPosition != WFS_CIM_POSOUTCENTER) {
            m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Некорректные позиции!", __FUNCTION__), LOGLEVELMSG);
            return WFS_ERR_CIM_UNSUPPOSITION;
        }

        if (m_ptrParams->usNumber > 1) {
            m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Некорректные позиции!", __FUNCTION__), LOGLEVELMSG);
            return WFS_ERR_CIM_INVALIDCASHUNIT;
        }
    }

    // В случае отсутствующей кассеты, открытых крышках распознавателей и т.п. (одним словом,
    // в случае любых ошибочных состояний), превентивно сообщаем о
    // невозможности выполнить операцию
    switch (m_psHandler.m_State) {
    case OpticCanalFail:
        m_psHandler.m_log->trace(std::format("{}() - текущее состояние = OpticCanalFail.", __FUNCTION__), LOGLEVELMSG);

        return WFS_ERR_HARDWARE_ERROR;
    case DropCassetteOutOfPosition:
        m_psHandler.m_log->trace(std::format("{}() - текущее состояние = DropCassetteOutOfPosition. Не является ошибочной ситуацией, механический сброс не имеет смысла и выполнен не будет", __FUNCTION__), LOGLEVELMSG);
        // Не является ошибочной ситуацией, механический сброс не имеет смысла и выполнен не будет
        return WFS_ERR_DEV_NOT_READY;

    case DropCassetteFull:
        // По обращению #0010164: если аппарат в состоянии DropCassetteFull - не имеет смысла пытаться выполнить Reset
        m_psHandler.m_log->trace(std::format("{}() - текущее состояние = DropCassetteFull. Действия не нужны",  __FUNCTION__), LOGLEVELMSG);
        m_psHandler.SendCashUnitError(WFS_CIM_CASHUNITFULL);
        return WFS_ERR_CIM_CASHUNITERROR;
    }

    return WFS_SUCCESS;
}
