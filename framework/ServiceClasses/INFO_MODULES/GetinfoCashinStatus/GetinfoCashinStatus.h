#pragma once
#ifndef __GETINFOCASHINSTATUS_H__
#define __GETINFOCASHINSTATUS_H__

#include <mutex>
#include "ComTransport/DorsPSHandler.h"
#include "Network/TcpServer.h"
#include <string>
#include "PersistentDatas/PersistentDatasHandler.h"

/**
 * @file GetinfoCashinStatus.h
 * @brief Менеджер статуса транзакции CashIn.
 *
 * Файл содержит класс `GetinfoCashinStatus`, предназначенный для хранения и
 * управления состоянием транзакции CashIn: открытия/закрытия транзакции,
 * отслеживания списка номеров банкнот, счётчика отвергнутых банкнот и
 * сохранения/загрузки состояния в/из `PersistentDatasHandler`.
 * Класс реализован как ленивый синглтон и предназначен для многопоточного
 * использования (защита критических участков посредством мьютексов).
 */

namespace Xfs::Cim {

    /**
     * @enum Status
     * @brief Возможные состояния транзакции CashIn.
     *
     * Описывает стандартные состояния транзакции (ACTIVE, ROLLBACK, RETRACT и т.д.).
     */
    enum Status : uint16_t {
        CIM_CIOK = 0,       /**< Транзакция завершена корректно */
        CIM_CIROLLBACK = 1, /**< Транзакция откатывается */
        CIM_CIACTIVE = 2,   /**< Транзакция активна */
        CIM_CIRETRACT = 3,  /**< Транзакция в состоянии retract */
        CIM_CIUNKNOWN = 4,  /**< Неизвестное состояние */
        CIM_CIRESET = 5     /**< Статус сброшен */
    };

    /**
     * @class GetinfoCashinStatus
     * @brief Управление статусом CashIn и связанными сессиями/данными.
     *
     * Класс хранит текущее состояние транзакции CashIn (`status`), список
     * номеров банкнот (`noteNumberList`), список непригодных банкнот
     * (`unfitNoteNumberList`), счётчик отказанных банкнот (`numOfRefused`) и
     * дополнительные данные (`extra`). Предоставляет методы для открытия/закрытия
     * транзакции, добавления номеров банкнот, сохранения статуса в
     * `PersistentDatasHandler` и загрузки его при инициализации синглтона.
     *
     * Потокобезопасность: операции записи/чтения защищены статическим мьютексом
     * `m_sendMutex` либо другими мьютексами, где это необходимо.
     */
    class GetinfoCashinStatus {
    public:
        GetinfoCashinStatus() = default;
        GetinfoCashinStatus(const GetinfoCashinStatus&) = delete;            /**< Копирование запрещено */
        GetinfoCashinStatus& operator=(const GetinfoCashinStatus&) = delete; /**< Присваивание запрещено */

        /**
         * @brief Заполнить/обновить внутреннее представление результата (кэш) при необходимости.
         *
         * Метод может использоваться для формирования отладочной информации
         * или подготовки данных для отдачи клиенту. Реализация защищена мьютексом.
         */
        void fillResult();

        /**
         * @brief Получить глобальный экземпляр (ленивый синглтон).
         *
         * При первом вызове создаёт экземпляр и загружает состояние транзакции
         * из `PersistentDatasHandler` (вызывает `LoadTransactionStatus`).
         * Потокобезопасно благодаря `m_instanceMutex`.
         *
         * @return Указатель на единственный экземпляр `GetinfoCashinStatus`.
         */
        static GetinfoCashinStatus* GetInstance();

        /**
         * @brief Установить указатель на обработчик устройства (не владеет указателем).
         * @param handler Ссылка на `DorsPSHandler`.
         */
        void setHandler(DorsPSHandler& handler) noexcept {
            m_psHandler = &handler;
        }

        /**
         * @brief Установить сетевой сервер для broadcast (не владеет указателем).
         * @param server Ссылка на `TcpServer`.
         */
        void setServer(TcpServer& server) noexcept {
            m_server = &server;
        }

        /**
         * @brief Сохранить текущее состояние транзакции в `PersistentDatasHandler`.
         *
         * Вызывается для постоянного хранения параметров транзакции (например,
         * после изменения списка номеров или статуса).
         */
        void SaveTransactionStatus();

        /**
         * @brief Увеличить счётчик отвергнутых банкнот на `usCount` и сохранить изменения.
         * @param usCount Количество отказанных банкнот для добавления.
         */
        void AddRefusedBanknotes(uint16_t usCount);

        /**
         * @brief Открыть транзакцию CashIn: выставить статус ACTIVE и обнулить счётчики.
         */
        void OpenTransaction();

        /**
         * @brief Закрыть транзакцию с указанным статусом.
         * @param wStatus Код статуса (см. `Status`).
         */
        void CloseTransaction(uint16_t wStatus);

        /**
         * @brief Проверить, активна ли текущая транзакция CashIn.
         * @return true если `status == CIM_CIACTIVE`.
         */
        bool IsCashInActive();

        /**
         * @brief Добавить элементы в список номеров банкнот (складывание списков).
         * @param additiveNoteNumberList Список для добавления.
         *
         * Метод складывает переданный список с внутренним `noteNumberList`,
         * сохраняет изменения в `PersistentDatasHandler` и выполняет broadcast
         * через `m_server` (если доступен).
         */
        void AddNoteNumberList(STRUCT::CimNoteNumberList& additiveNoteNumberList);

        /**
         * @brief Сбросить внутренний список номеров банкнот (и сохранить изменения).
         */
        void ResetNoteNumberList();

        /* --- Публичные поля, представляющие текущее состояние транзакции --- */
        uint16_t status;                          /**< Текущий статус транзакции (см. `Status`) */
        uint16_t numOfRefused;                    /**< Количество отказанных банкнот */
        STRUCT::CimNoteNumberList noteNumberList; /**< Список номеров банкнот в текущей транзакции */
        std::string extra;                        /**< Дополнительная информация (строка) */
        STRUCT::CimNoteNumberList unfitNoteNumberList; /**< Список непригодных номеров банкнот */

    private:
        static GetinfoCashinStatus* pinstance_;   /**< Указатель на синглтон (если создан) */
        static std::mutex m_sendMutex;            /**< Мьютекс для защиты критичных операций */
        static std::mutex m_instanceMutex;        /**< Мьютекс для безопасной инициализации синглтона */
        DorsPSHandler* m_psHandler = nullptr;    /**< Указатель на обработчик устройства (не владеет) */
        TcpServer* m_server = nullptr;           /**< Указатель на сетевой сервер (не владеет) */

        /**
         * @brief Вспомогательный метод: вернуть текстовое представление внутреннего состояния.
         * @return Отформатированная многострочная строка.
         */
        std::string PrintFromResult() const;

        /**
         * @brief Загрузить состояние транзакции из `PersistentDatasHandler`.
         * @param handler Ссылка на обработчик персистентных данных.
         *
         * Метод читает сохранённые ключи (статус, список номеров и т.д.) и
         * инициализирует соответствующие поля объекта.
         */
        void LoadTransactionStatus(PersistentDatasHandler& handler);

        /**
         * @brief Вспомогательная функция чтения числового значения из `PersistentDatasHandler`.
         * @param path Точечный путь ключа в JSON (например, "CashInStatus.Status").
         * @param var [out] Переменная для записи значения (uint16_t).
         *
         * При отсутствии или некорректном значении создаёт запись со значением 0.
         */
        void getNumberValue(std::string path, uint16_t& var);

        /**
         * @brief Вспомогательная функция чтения строкового значения из `PersistentDatasHandler`.
         * @param path Точечный путь ключа в JSON.
         * @param var [out] Переменная для записи строки.
         *
         * При отсутствии или некорректном значении создаёт запись со значением "0, 0".
         */
        void getStringValue(std::string path, std::string& var);
    };

    /*
     * Реализация GetInstance вынесена из тела класса в отдельное inline-определение,
     * чтобы избежать возможных проблем статического анализа при использовании
     * статических мьютексов/переменных, объявленных в private секции класса.
     */
    inline GetinfoCashinStatus* GetinfoCashinStatus::GetInstance() {
        std::lock_guard<std::mutex> lock(m_instanceMutex);
        if (pinstance_ == nullptr)
        {
            pinstance_ = new GetinfoCashinStatus;
            pinstance_->LoadTransactionStatus(*PersistentDatasHandler::GetInstance());
        }
        return pinstance_;
    }

}

#endif // #ifndef __GETINFOCASHINSTATUS_H__



