#include "GetinfoCashinStatus.h"


Xfs::Cim::GetinfoCashinStatus* Xfs::Cim::GetinfoCashinStatus::pinstance_ = nullptr;
std::mutex Xfs::Cim::GetinfoCashinStatus::m_sendMutex;
std::mutex Xfs::Cim::GetinfoCashinStatus::m_instanceMutex;

namespace Xfs::Cim {

    void GetinfoCashinStatus::fillResult()
    {
        std::lock_guard<std::mutex> lock(m_sendMutex);
    }

    void GetinfoCashinStatus::SaveTransactionStatus()
    {
        std::lock_guard<std::mutex> lock(m_sendMutex);
        PersistentDatasHandler::GetInstance()->update("CashInStatus.Status", status);
    }

    void GetinfoCashinStatus::CloseTransaction(uint16_t wStatus)
    {
        std::lock_guard<std::mutex> lock(m_sendMutex);
        // При закрытии транзакции необходимо выставить нужный статус транзакции
        status = wStatus;

        // Сохраняем параметры
        SaveTransactionStatus();
    }

    bool GetinfoCashinStatus::IsCashInActive()
    {
        std::lock_guard<std::mutex> lock(m_sendMutex);
        return (CIM_CIACTIVE == status);
    }

    void GetinfoCashinStatus::AddNoteNumberList(STRUCT::CimNoteNumberList& additiveNoteNumberList)
    {
        std::lock_guard<std::mutex> lock(m_sendMutex);
        // Суммируем
        noteNumberList += additiveNoteNumberList;

        // Сохраняем измененные данные транзакции
        SaveTransactionStatus();

        m_server->broadcast(PersistentDatasHandler::GetInstance()->readByKey("CashInStatus").dump(4), 0x00000002);
    }

    void GetinfoCashinStatus::ResetNoteNumberList()
    {
        std::lock_guard<std::mutex> lock(m_sendMutex);
        if (!noteNumberList.empty()) {
            noteNumberList.clear();
            SaveTransactionStatus();
        }
    }

    void GetinfoCashinStatus::OpenTransaction()
    {
        std::lock_guard<std::mutex> lock(m_sendMutex);

        // При открытии транзакции необходимо выставить статус - ACTIVE и обнулить необходимые параметры
        status = CIM_CIACTIVE;
        noteNumberList.clear();
        numOfRefused = 0;
        extra.clear();

        // Сохраняем параметры
        SaveTransactionStatus();
    }

    void GetinfoCashinStatus::AddRefusedBanknotes(uint16_t usCount)
    {
        std::lock_guard<std::mutex> lock(m_sendMutex);
        numOfRefused += usCount;

        // Сохраняем параметры
        SaveTransactionStatus();
    }

    void GetinfoCashinStatus::LoadTransactionStatus(PersistentDatasHandler& handler)
    {
        std::lock_guard<std::mutex> lock(m_sendMutex);
        // Извлекаем значения по ключам

        getNumberValue("CashInStatus.Status", status);
        getNumberValue("CashInStatus.NumOfRefused", numOfRefused);
        std::string strNoteNumberList;
        getStringValue("CashInStatus.NoteNumberList", strNoteNumberList);
        noteNumberList.Parse(strNoteNumberList, {});
        std::string strunfitNoteNumberList;
        getStringValue("CashInStatus.UnfitNoteNumberList", strunfitNoteNumberList);
        unfitNoteNumberList.Parse(strNoteNumberList, {});
    }

    void GetinfoCashinStatus::getNumberValue(std::string path, uint16_t& var)
    {
        try {
            nlohmann::json value = PersistentDatasHandler::GetInstance()->readByKey(path);
            // Проверяем, что значение не пустое и является числом
            if (!value.is_null() && value.is_number_unsigned()) {
                std::uint16_t count = value.get<std::uint32_t>();
                if (count > 0) { // Проверяем, что значение больше 0 (если нужно)
                    var = value.get<std::uint32_t>();
                }
                else {
                    throw PersistentDatasHandler::PersistentDatasHandlerException("value is zero or negative");
                }
            }
            else {
                throw PersistentDatasHandler::PersistentDatasHandlerException("value is null or not an unsigned number");
            }
        }
        catch (const PersistentDatasHandler::PersistentDatasHandlerException&) {
            // Ключ path не найден или недействителен, создаем новый
            // Сохраняем в dorsConfig.json
            PersistentDatasHandler::GetInstance()->update(path, 0u);
            var = 0u;
        }
    }

    void GetinfoCashinStatus::getStringValue(std::string path, std::string& var)
    {
        try {
            nlohmann::json value = PersistentDatasHandler::GetInstance()->readByKey(path);
            // Проверяем, что значение не пустое и является числом
            if (!value.is_null() && value.is_string()) {
                std::string text = value.get<std::string>();
                if (text.length() > 0) { // Проверяем, что значение больше 0 (если нужно)
                    var = text;
                }
                else {
                    throw PersistentDatasHandler::PersistentDatasHandlerException("value of length is zero or negative");
                }
            }
            else {
                throw PersistentDatasHandler::PersistentDatasHandlerException("value is null or not an string");
            }
        }
        catch (const PersistentDatasHandler::PersistentDatasHandlerException&) {
            // Ключ path не найден или недействителен, создаем новый
            // Сохраняем в dorsConfig.json
            PersistentDatasHandler::GetInstance()->update(path, "0, 0");
            var = "0, 0";
        }
    }


    std::string GetinfoCashinStatus::PrintFromResult() const {

        std::ostringstream oss;

        oss << "Status: " << status << "\n";
        oss << "NumOfRefused: " << numOfRefused << "\n";
        oss << "NoteNumberList" << "\n\t";

        oss << "NumOfNoteNumbers: " << noteNumberList.size() << "\n\t\t";
        for (auto& note : noteNumberList)
        {
            oss << "NoteId: " << note.usNoteID;
            oss << "\tCount: " << note.ulCount << "\n";
        }

        oss << "Extra: " << (extra.empty() ? "EMPTY" : extra) << "\n";

        return oss.str();
    }
}
