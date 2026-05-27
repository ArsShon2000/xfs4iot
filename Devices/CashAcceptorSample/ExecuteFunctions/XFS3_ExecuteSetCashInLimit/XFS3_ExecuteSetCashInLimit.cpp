#include "XFS3_ExecuteSetCashInLimit.hpp"
#include "MODULE_STRUCTURES/CimCurrencyExp/CimCurrencyExp.h"
#include <CIM.PS/INFO_MODULES/GetinfoBanknoteTypes/GetinfoBanknoteTypes.h>
#include <CIM.PS/INFO_MODULES/GetInfoCashUnit/GetInfoCashUnit.h>
#include <boost/scope_exit.hpp>

using namespace STRUCT;

void XFS3_ExecuteSetCashInLimit::fillResult(LPWFSRESULT* lppWfsResult)
{
    HRESULT hResult = ValidateProcessingConditions();
    if (WFS_SUCCESS != hResult) {

        //m_psHandler.m_log->warn(std::format("{}() - Запрос отклонён : {}",  __FUNCTION__, XfsGetErrCodeText(hResult)), LOGLEVEL1);

        (*lppWfsResult)->hResult = hResult;
        return;
    }
}

void XFS3_ExecuteSetCashInLimit::doBasicLogic(LPWFSRESULT lpWfsResult)
{
    m_psHandler.m_log->trace(std::format("{} ------------------- Запуск обработчика WFS_CMD_CIM_SET_CASH_IN_LIMIT -------------------",  __FUNCTION__), LOGLEVEL1);

    BOOST_SCOPE_EXIT(&lpWfsResult, &m_psHandler) {
        m_psHandler.m_log->trace(std::format("{}() ------------------- Завершение обработчика WFS_CMD_CIM_SET_CASH_IN_LIMIT [hResult = {}] -------------------", __FUNCTION__, XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
    } BOOST_SCOPE_EXIT_END;

    lpWfsResult->hResult = ValidateProcessingConditions();
    if (WFS_SUCCESS != lpWfsResult->hResult) {
        m_psHandler.m_log->warn(std::format("{}() - Запрос отклонён : {}", __FUNCTION__, XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
        return;
    }


    if (m_limit.ulTotalItemsLimit == 0) {
        // Нулевые значения не ограничивают транзакцию приема
        m_psHandler.m_limits = {};
    }
    else {
        m_psHandler.m_bIsSetCashInLimit = true;						// Признак применения ограничений приема
        // Вычисление свободного места в кассете
        ULONG count = GetInfoCashUnit::GetInstance()->physicalCashUnitList[0].ulMaximum - GetInfoCashUnit::GetInstance()->physicalCashUnitList[0].ulCount;
        m_limit.ulTotalItemsLimit = (count > m_limit.ulTotalItemsLimit) ? m_limit.ulTotalItemsLimit : count;	// Ограничение количества банкнот
        m_psHandler.m_limits = m_limit;
    }

    m_psHandler.m_log->trace(std::format("{}() ------------------- Завершение обработчика WFS_CMD_CIM_SET_CASH_IN_LIMIT [hResult = {}] -------------------",  __FUNCTION__
        , XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1
    );
}

HRESULT XFS3_ExecuteSetCashInLimit::ValidateProcessingConditions()
{
    // Проверка входной структуры
    CimNoteTypeList ntList = GetinfoBanknoteTypes::GetInstance()->GetPreparedData();						// Получение списка валют. Указанная валюта должна поддерживаться
    std::set< std::string > setCurrencies = ntList.GetCurrencies();	// Получение списка валют
    setCurrencies.insert("   ");									// Дополнение пустой валютой (три пробела)
    if (setCurrencies.find(m_limit.amountLimit.strCurrencyID) == setCurrencies.end()) {
        m_psHandler.m_log->warn(std::format("{}() - Запрос отклонён: Некорректные входные параметры!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_UNSUPP_DATA;
    }

    if (m_psHandler.m_bSoftwareConfigurationFault) {
        m_psHandler.m_log->warn(std::format("{}() - Запрос отклонён: Девайс неправильно сконфигугрирован!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_SOFTWARE_ERROR;
    }

    if (!m_psHandler.m_pDevice->IsDeviceInitialized()) {
        m_psHandler.m_log->warn(std::format("{}() - Запрос отклонён: Девайс не инициализирован!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_DEV_NOT_READY;
    }

    if (m_psHandler.m_bExchangeInProgress) {
        m_psHandler.m_log->warn(std::format("{}() - Запрос отклонён: Инкассация в процессе!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_EXCHANGEACTIVE;
    }

    // Проверка была ли выполнена CASH_IN_START
    if (!m_psHandler.m_pEscrowManager->IsCashInActive()) {
        m_psHandler.m_log->warn(std::format("{}() - Запрос отклонён: WFS_CMD_CASH_IN_START не подан!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_NOCASHINACTIVE;
    }

    if (!m_psHandler.m_bSetCashInLimitAvailable) {
        m_psHandler.m_log->warn(std::format("{}() - Запрос отклонён: Устройство не готов принять команду WFS_CMD_CIM_SET_CASH_IN_LIMIT!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_SEQUENCE_ERROR;
    }

    // Был выполнен пересчет хотя бы одной банкноты или муляжа
    CimNoteNumberList nnList = m_psHandler.m_pEscrowManager->GetNoteNumberList();
    if ((nnList.GetTotalNotesCount() != 0) || (m_psHandler.m_pEscrowManager->GetNumOfRefused() != 0)) {
        m_psHandler.m_log->warn(std::format(
            "{}() - Нарушена последовательность запросов.",
            __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_SEQUENCE_ERROR;
    }

    return WFS_SUCCESS;
}
