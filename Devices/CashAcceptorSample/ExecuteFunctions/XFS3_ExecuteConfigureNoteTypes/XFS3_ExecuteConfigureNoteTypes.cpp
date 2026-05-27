#include "XFS3_ExecuteConfigureNoteTypes.h"

void XFS3_ExecuteConfigureNoteTypes::doBasicLogic(LPWFSRESULT lpWfsResult)
{
    m_psHandler.m_log->trace(std::format("{} ------------------- Запуск обработчика WFS_CMD_CIM_CONFIGURE_NOTETYPES -------------------", __FUNCTION__), LOGLEVEL1);

    lpWfsResult->hResult = ValidateProcessingConditions();
    if (WFS_SUCCESS != lpWfsResult->hResult) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}", __FUNCTION__, XfsGetErrCodeText(lpWfsResult->hResult)), LOGLEVEL1);
        return;
    }

    if (!m_psHandler.EnsureBillTableIsAvailable())
    {
        lpWfsResult->hResult = WFS_ERR_DEV_NOT_READY;
        return;
    }

    //Сбрасываем список распознаваемых купюр
    m_psHandler.m_lBillTypes = 0;
    for (const auto noteID : m_noteIDs)
    {
        if (noteID == 0) {
            break;
        }

        auto it_idxs = m_psHandler.m_denominationsLogicalToPhysical.find(noteID);    // Ищем идентификатор номинала в карте соответствия
        if (it_idxs != m_psHandler.m_denominationsLogicalToPhysical.end()) {

            // В цикле, по каждому циклу, взводим бит в маске
            for (auto it_idx = it_idxs->second.begin(); it_idx != it_idxs->second.end(); ++it_idx) {
                SetBit(*it_idx, m_psHandler.m_lBillTypes);
            }
        }
    }

    //Сохраняем сделанные настройки
    m_psHandler.SaveNotesList();

    m_psHandler.m_log->trace(std::format("{}() ------------------- Завершение обработчика WFS_CMD_CIM_CONFIGURE_NOTETYPES [hResult = {}] -------------------", __FUNCTION__
        , XfsGetErrCodeText(lpWfsResult->hResult), LOGLEVEL1)
    );
}

void XFS3_ExecuteConfigureNoteTypes::fillResult(LPWFSRESULT* lpWfsResult)
{
    HRESULT hResult = ValidateProcessingConditions();
    if (WFS_SUCCESS != hResult) {

        //m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён : {}", __FUNCTION__, XfsGetErrCodeText(hResult)), LOGLEVEL1);

        (*lpWfsResult)->hResult = hResult;
        return;
    }
}

// Проверяет, что все идентификаторы номиналов в m_noteIDs поддерживаются устройством
// Валидация входных данных. Если хотя бы один идентификатор не поддерживается - прерываем исполнение
int XFS3_ExecuteConfigureNoteTypes::CheckNoteIDs() const {
    if (!&m_psHandler) {
        return WFS_ERR_INVALID_POINTER; // Предполагаемый код ошибки для nullptr
    }

    bool unsupported = std::ranges::any_of(m_noteIDs,
        [this](uint16_t id) {
            return !m_psHandler.m_denominationsLogicalToPhysical.contains(id);
        });

    if (unsupported)
    {
        m_psHandler.m_log->trace(std::format("{}() -  встречен неподдерживаемый идентификатор банкноты", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_UNSUPP_DATA;
    }
    else return WFS_SUCCESS;
}

HRESULT XFS3_ExecuteConfigureNoteTypes::ValidateProcessingConditions()
{
    if (m_psHandler.m_bSoftwareConfigurationFault) {
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Девайс неправильно сконфигугрирован!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_SOFTWARE_ERROR;          // Некорректная конфигурация ПО
    }

    if (!m_psHandler.m_pDevice->IsDeviceInitialized()) { 
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Девайс не инициализирован!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_DEV_NOT_READY;           // Устройство не инициализировано
    }

    if (m_psHandler.m_bExchangeInProgress) { 
        m_psHandler.m_log->trace(std::format("{}() - Запрос отклонён: Инкассация в процессе!", __FUNCTION__), LOGLEVELMSG);
        return WFS_ERR_CIM_EXCHANGEACTIVE;      // Инкасация в прогрессе
    }

    // Есть примеры, когда управляющее ПО(TellMe) конфигурирует номиналы после подачи WFS_CMD_CIM_CASH_IN_START.
    auto tranStatus = m_psHandler.m_pEscrowManager->GetCashInStatus();
    if ((WFS_CIM_CIACTIVE == tranStatus.wStatus)
        && (!tranStatus.noteNumberList.IsBundleEmpty()      // noteNumberList очищается после WFS_CMD_CIM_CASH_IN_START
            || (tranStatus.usNumOfRefused > 0))
        )
    {
        m_psHandler.m_log->trace(std::format("{}() - АКТИВНА CASHIN-СЕССИЯ, В РАМКАХ КОТОРОЙ УЖЕ БЫЛ ПРИЁМ НАЛИЧНЫХ", __FUNCTION__), LOGLEVELMSG);
        // Однако не позволяем этого делать, если в рамках транзакции был приём
        return WFS_ERR_CIM_CASHINACTIVE;
    }

    return CheckNoteIDs();
}

void XFS3_ExecuteConfigureNoteTypes::SetBit(BYTE n, uint32_t& Val)
{
    unsigned long tmp = 1;
    Val |= (tmp << n);
}
