//#pragma once
//#ifndef __ESCROWMANAGER_H__
//#define __ESCROWMANAGER_H__
//
///**
// * @file EscrowManager.h
// * @brief Менеджер состояния escrow для транзакций CashIn.
// *
// * Файл содержит определение класса `EscrowManager`, который отвечает за хранение
// * и управление данными текущей транзакции CashIn (статус, список номеров банкнот,
// * счётчики отказанных банкнот и дополнительная информация). Класс обеспечивает
// * потокобезопасный доступ к данным транзакции и интеграцию с системой персистентного
// * хранения через `PersistentDatasHandler` (вызовы производятся в реализации).
// */
//
//#include <mutex>
//#include <memory>
//#include "../../../core/Logger/ILogger.hpp"
//
///**
// * @class EscrowManager
// * @brief Потокобезопасный менеджер состояния escrow (CashIn transaction manager).
// *
// * `EscrowManager` инкапсулирует данные текущей транзакции CashIn в поле
// * `m_status` (тип `CimCashInStatus`) и предоставляет интерфейс для работы с
// * транзакцией: открытие/закрытие, добавление номеров банкнот, сохранение и
// * загрузка состояния, подсчёт частичного состояния промежуточного стакера и
// * пометка спорных банкнот. Все публичные методы безопасны для вызова из
// * нескольких потоков, т.к. используют внутренний мьютекс `m_mutex`.
// */
//class EscrowManager
//{
//public:
//    /**
//     * @brief Конструктор менеджера.
//     * @param logger Общий логгер, используемый для записи диагностических сообщений
//     *               (класс сохраняет `shared_ptr`, владелец логгера остаётся за вызывающей стороной).
//     */
//    EscrowManager(std::shared_ptr<ILogger> logger);
//
//    /**
//     * @brief Деструктор.
//     */
//    ~EscrowManager() {};
//
//    /**
//     * @brief Получить текущий статус транзакции (WFS_CIM_CI*).
//     * @return Код статуса транзакции.
//     */
//    uint16_t GetStatus();
//
//    /**
//     * @brief Получить список номеров банкнот, участвующих в транзакции.
//     * @return Копия `CimNoteNumberList` текущей транзакции.
//     */
//    //CimNoteNumberList GetNoteNumberList();
//
//    /**
//     * @brief Получить количество отвергнутых купюр в рамках текущей транзакции.
//     * @return Количество отказанных банкнот.
//     */
//    uint16_t GetNumOfRefused();
//
//    /**
//     * @brief Получить полную структуру данных транзакции `CimCashInStatus`.
//     * @return Копия текущих данных транзакции.
//     */
//    //CimCashInStatus GetCashInStatus();
//
//    /**
//     * @brief Закрыть транзакцию и выставить указанный статус.
//     * @param wStatus Код статуса транзакции (одно из WFS_CIM_CI*).
//     */
//    void CloseTransaction(uint16_t wStatus);
//
//    /**
//     * @brief Проверить, активна ли транзакция CashIn.
//     * @return true, если статус транзакции соответствует ACTIVE.
//     */
//    bool IsCashInActive();
//
//    /**
//     * @brief Добавить список номеров банкнот к текущему списку транзакции.
//     * @param additiveNoteNumberList Список, который необходимо присоединить.
//     *
//     * После объединения списков состояние сохраняется (SaveTransactionStatus).
//     */
//    //void AddNoteNumberList(CimNoteNumberList& additiveNoteNumberList);
//
//    /**
//     * @brief Сбросить (очистить) внутренний список номеров банкнот текущей транзакции.
//     */
//    void ResetNoteNumberList();
//
//    /**
//     * @brief Открыть новую транзакцию CashIn: выставить статус ACTIVE и сбросить счётчики.
//     */
//    void OpenTransaction();
//
//    /**
//     * @brief Увеличить количество отвергнутых банкнот на `usCount`.
//     * @param usCount Количество добавляемых отказанных банкнот.
//     */
//    void AddRefusedBanknotes(uint16_t usCount);
//
//    /**
//     * @brief Сохранить текущее состояние транзакции в персистентное хранилище.
//     *
//     * Реализация использует `PersistentDatasHandler` для записи полей `m_status`.
//     */
//    void SaveTransactionStatus();
//
//    /**
//     * @brief Загрузить состояние транзакции из персистентного хранилища.
//     *
//     * Метод читает значения из `PersistentDatasHandler` и восстанавливает `m_status`.
//     */
//    void LoadTransactionStatus();
//
//    /**
//     * @brief Частичный расчёт состояния промежуточного стакера (escrow).
//     *
//     * Возвращает одно из значений WFS_CIM_IS* (например, ISEMPTY/ISFULL) на основе
//     * текущих данных транзакции. Этот метод выполняет упрощённую оценку и может
//     * использоваться при формировании WFSCIMSTATUS.fwIntermediateStacker — для
//     * окончательного значения следует учитывать общее состояние устройства.
//     *
//     * @return Частичное значение состояния промежуточного стакера.
//     */
//    uint16_t GetIntermediateStackerPartialState();
//
//    /**
//     * @brief Пометить спорную/оспариваемую банкноту.
//     * @param rejNoteNumberList Список спорных номеров (обычно один элемент).
//     */
//    //void SetDisputedNote(const CimNoteNumberList& rejNoteNumberList);
//
//private:
//
//    /// @brief Данные текущей транзакции (публично доступны для чтения/копирования).
//    //CimCashInStatus m_status;
//    /**
//     * @brief Мьютекс для защиты доступа к `m_status` и другим внутренним данным.
//     */
//    std::mutex m_mutex;
//
//    /**
//     * @brief Логгер для записи диагностических сообщений (shared_ptr, владелец — вызывающая сторона).
//     */
//    std::shared_ptr<class ILogger> m_pLog;
//};
//
//#endif // __ESCROWMANAGER_H__
//
//
//
