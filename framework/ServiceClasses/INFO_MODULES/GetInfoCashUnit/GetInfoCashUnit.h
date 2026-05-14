#ifndef __GETINFOCASHUNIT_H__
#define __GETINFOCASHUNIT_H__
#include "ComTransport/DorsPSHandler.h"
#include "Network/TcpServer.h"
#include <MODULE_STRUCTURES/CimNoteNumber/CimNoteNumber.h>
#include <MODULE_STRUCTURES/CimCashUnit/CimCashUnit.hpp>
#include <MODULE_STRUCTURES/CimPhCashUnit/CimPhCashUnit.h>
#include <MODULE_STRUCTURES/StructFillers/FillByExtra.h>
#include <mutex>

using namespace STRUCT;

/**
 * @file GetInfoCashUnit.h
 * @brief Модуль для получения информации о кассетах (CashUnit) купюроприёмника.
 *
 * Класс `GetInfoCashUnit` реализует хранение и обновление информации о
 * логической кассете, поддерживает потокобезопасность, предоставляет
 * методы для интеграции с обработчиком устройства и сетевым сервером,
 * а также для заполнения структур XFS3/XFS4.
 * Реализован как ленивый синглтон.
 */

namespace Xfs::Cim {

    /**
     * @class GetInfoCashUnit
     * @brief Хранение и обновление информации о логической кассете.
     *
     * Класс инкапсулирует поля, соответствующие структуре кассеты (номер,
     * тип, номинал, статус, список номеров банкнот, физические кассеты и др.),
     * а также методы для обновления, интеграции с обработчиком и сервером,
     * заполнения структур XFS3/XFS4 и потокобезопасного доступа.
     * Используется как синглтон через `GetInstance()`.
     */
    class GetInfoCashUnit
    {
    public:
        /**
         * @brief Конструктор по умолчанию.
         */
        GetInfoCashUnit() = default;
        GetInfoCashUnit(const GetInfoCashUnit&) = delete;            /**< Копирование запрещено */
        GetInfoCashUnit& operator=(const GetInfoCashUnit&) = delete; /**< Присваивание запрещено */

        /**
         * @brief Обновить все поля из структуры `CimCashUnit`.
         * @param cashUnit Структура с актуальными данными.
         */
        void update(const CimCashUnit& cashUnit);

        /** @brief Номер кассеты. */
        uint16_t usNumber;
        /** @brief Тип кассеты (флаги XFS). */
        uint32_t fwType;
        /** @brief Тип предмета (флаги XFS). */
        uint32_t fwItemType;
        /** @brief Идентификатор кассеты (строка). */
        std::string strUnitID;
        /** @brief Валюта кассеты (строка). */
        std::string strCurrencyID;
        /** @brief Номинал кассеты. */
        uint32_t ulValues;
        /** @brief Количество внесённых банкнот. */
        uint32_t ulCashInCount;
        /** @brief Текущее количество банкнот. */
        uint32_t ulCount;
        /** @brief Максимальная ёмкость кассеты. */
        uint32_t ulMaximum;
        /** @brief Статус кассеты (флаги XFS). */
        uint16_t usStatus;
        /** @brief Флаг блокировки приложения. */
        bool bAppLock;
        /** @brief Список номеров банкнот. */
        CimNoteNumberList noteNumberList;
        /** @brief Список физических кассет. */
        CimPhCashUnitList physicalCashUnitList;
        /** @brief Дополнительные параметры (extra). */
        XfsExtra_t extra;
        /** @brief Список идентификаторов банкнот. */
        std::vector< uint16_t > noteIDs;
        /** @brief Тип CDM (для XFS3.10+). */
        uint16_t usCDMType;
        /** @brief Имя кассеты (строка). */
        std::string strCashUnitName;
        /** @brief Начальное количество банкнот. */
        uint32_t ulInitialCount;
        /** @brief Количество выданных банкнот. */
        uint32_t ulDispensedCount;
        /** @brief Количество предъявленных банкнот. */
        uint32_t ulPresentedCount;
        /** @brief Количество ретрактированных банкнот. */
        uint32_t ulRetractedCount;
        /** @brief Количество отклонённых банкнот. */
        uint32_t ulRejectCount;
        /** @brief Минимальная ёмкость кассеты. */
        uint32_t ulMinimum;

        /**
         * @brief Получить или создать глобальный экземпляр (ленивый синглтон).
         * @return Указатель на экземпляр `GetInfoCashUnit`.
         */
        static GetInfoCashUnit* GetInstance() {
            std::lock_guard<std::mutex> lock(m_instanceMutex);
            if (pinstance_ == nullptr)
            {
                pinstance_ = new GetInfoCashUnit;
                pinstance_->initialize();
            }
            return pinstance_;
        }

        /**
         * @brief Установить обработчик устройства (не владеет указателем).
         * @param handler Ссылка на `DorsPSHandler`.
         */
        void setHandler(DorsPSHandler& handler) noexcept;

        /**
         * @brief Установить сетевой сервер (не владеет указателем).
         * @param server Ссылка на `TcpServer`.
         */
        void setServer(TcpServer& server) noexcept;

        /**
         * @brief Заполнить структуру `CimCashUnit` актуальными данными (XFS3).
         * @param cashUnit Структура для заполнения.
         */
        void fill_XFS3_Result(CimCashUnit& cashUnit);

        /**
         * @brief Заполнить структуру `CimCashUnit` актуальными данными (XFS4).
         * @param cashUnit Структура для заполнения.
         */
        void fill_XFS4_Result(CimCashUnit& cashUnit);

    private:
        /**
         * @brief Указатель на синглтон (если создан).
         */
        static GetInfoCashUnit* pinstance_;
        /**
         * @brief Мьютекс для сериализации операций (отправка/обновление).
         */
        static std::mutex m_sendMutex;
        /**
         * @brief Мьютекс для безопасной инициализации синглтона.
         */
        static std::mutex m_instanceMutex;
        /**
         * @brief Мьютекс для защиты операций заполнения.
         */
        std::mutex m_fillMutex;
        /**
         * @brief Указатель на обработчик устройства (не владеет).
         */
        DorsPSHandler* m_psHandler = nullptr;
        /**
         * @brief Указатель на сетевой сервер (не владеет).
         */
        TcpServer* m_server = nullptr;

        /**
         * @brief Вернуть текстовое представление текущего состояния (для логов/отладки).
         * @param nIndent Количество табуляций для форматирования.
         * @return Многострочная строка с данными кассеты.
         */
        std::string PrintFromResult(int nIndent = 0) const;

        /**
         * @brief Инициализировать поля кассеты значениями по умолчанию.
         */
        void initialize() noexcept;
    };
}

#endif // #ifndef __GETINFOCASHUNIT_H__
