//#include "EscrowManager.h"
////#include "PersistentDatas/PersistentDatasHandler.h"
//
//
//
//EscrowManager::EscrowManager(std::shared_ptr<ILogger> logger)
//    : m_pLog(std::move(logger))
//    //, m_status(CimCashInStatus())
//{
//    LoadTransactionStatus();
//}
//
//uint16_t EscrowManager::GetStatus()
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//    return m_status.wStatus;
//}
//
//CimNoteNumberList EscrowManager::GetNoteNumberList()
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//    return m_status.noteNumberList;
//}
//
//uint16_t EscrowManager::GetNumOfRefused()
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//    return m_status.usNumOfRefused;
//}
//
//CimCashInStatus EscrowManager::GetCashInStatus()
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//    return m_status;
//}
//
//void EscrowManager::CloseTransaction(uint16_t wStatus)
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//    // При закрытии транзакции необходимо выставить нужный статус транзакции
//    m_status.wStatus = wStatus;
//
//    // Сохраняем параметры
//    SaveTransactionStatus();
//}
//
//bool EscrowManager::IsCashInActive()
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//    return (WFS_CIM_CIACTIVE == m_status.wStatus);
//}
//
//void EscrowManager::AddNoteNumberList(STRUCT::CimNoteNumberList& additiveNoteNumberList)
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//    // Суммируем
//    m_status.noteNumberList += additiveNoteNumberList;
//
//    // Сохраняем измененные данные транзакции
//    SaveTransactionStatus();
//
//    //m_server->broadcast(PersistentDatasHandler::GetInstance()->readByKey("CashInStatus").dump(4), 0x00000002);
//}
//
//void EscrowManager::ResetNoteNumberList()
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//    if (!m_status.noteNumberList.empty()) {
//        m_status.noteNumberList.clear();
//        SaveTransactionStatus();
//    }
//}
//
//void EscrowManager::OpenTransaction()
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//
//    // При открытии транзакции необходимо выставить статус - ACTIVE и обнулить необходимые параметры
//    m_status.wStatus = WFS_CIM_CIACTIVE;
//    m_status.noteNumberList.clear();
//    m_status.usNumOfRefused = 0;
//    m_status.extra.clear();
//
//    // Сохраняем параметры
//    SaveTransactionStatus();
//}
//
//void EscrowManager::AddRefusedBanknotes(uint16_t usCount)
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//    m_status.usNumOfRefused += usCount;
//
//    // Сохраняем параметры
//    SaveTransactionStatus();
//}
//
//void EscrowManager::SaveTransactionStatus()
//{
//    //std::lock_guard<std::mutex> lock(m_mutex);
//    PersistentDatasHandler::GetInstance()->update("CashInStatus.Status", m_status.wStatus);
//    PersistentDatasHandler::GetInstance()->update("CashInStatus.NumOfRefused", m_status.usNumOfRefused);
//    PersistentDatasHandler::GetInstance()->update("CashInStatus.NoteNumberList", m_status.noteNumberList.Serialize());
//    for (const auto& _str : m_status.extra)
//    {
//        auto eqPos = _str.find('=');
//        if (eqPos != std::string::npos) {
//            std::string key{ _str.cbegin(), _str.cbegin() + eqPos };
//            std::string value{ _str.cbegin() + eqPos + 1, _str.cend() };
//            PersistentDatasHandler::GetInstance()->update("CashInStatus.extra." + key, value.c_str());
//        }
//    }
//}
//
//void EscrowManager::LoadTransactionStatus()
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//    // Извлекаем значения по ключам
//    PersistentDatasHandler& handler = *PersistentDatasHandler::GetInstance();
//
//    handler.getNumberValue("CashInStatus.Status", m_status.wStatus);
//    if (m_status.wStatus == WFS_CIM_CIACTIVE) {
//        m_status.wStatus = WFS_CIM_CIUNKNOWN;
//    }
//
//    handler.getNumberValue("CashInStatus.NumOfRefused", m_status.usNumOfRefused);
//    std::string strNoteNumberList;
//    handler.getStringValue("CashInStatus.NoteNumberList", strNoteNumberList);
//    m_status.noteNumberList.Parse(strNoteNumberList, {});
//    std::string strunfitNoteNumberList;
//    handler.getStringValue("CashInStatus.UnfitNoteNumberList", strunfitNoteNumberList);
//    m_status.unfitNoteNumberList.Parse(strNoteNumberList, {});
//}
//
//uint16_t EscrowManager::GetIntermediateStackerPartialState()
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//
//    // Если транзакция не активна, то в escrow не может быть банкнот
//    if (WFS_CIM_CIACTIVE != m_status.wStatus) {
//        return WFS_CIM_ISEMPTY;
//    }
//
//    if (m_status.noteNumberList.empty()) {
//        return WFS_CIM_ISEMPTY;
//    }
//
//    return WFS_CIM_ISFULL;
//}
//
//void EscrowManager::SetDisputedNote(const CimNoteNumberList& rejNoteNumberList)
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//
//    for (auto it = m_status.extra.begin(); it != m_status.extra.end(); ) {
//        if (boost::algorithm::icontains(*it, "DISPUTED=")) {
//            it = m_status.extra.erase(it);
//        }
//        else {
//            ++it;
//        }
//    }
//    m_status.extra.push_back(std::string{ "DISPUTED=" } + rejNoteNumberList.Serialize());
//}
