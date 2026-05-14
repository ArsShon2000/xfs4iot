#ifndef __GETINFOBANKNOTETYPEES_H__
#define __GETINFOBANKNOTETYPEES_H__

#include <mutex>
#include "../../../../Devices/CashAcceptorSample/CashAcceptorSample.hpp"

/**
 * @file GetinfoBanknoteTypes.h
 * @brief Модуль подготовки/предоставления информации о типах банкнот (GetinfoBanknoteTypes).
 *
 * Файл содержит класс `GetinfoBanknoteTypes` — лёгкий фасад/сервис для получения
 * подготовленного списка типов банкнот из слоя обработчика `DorsPSHandler` и
 * представления результата в виде строки или структуры `STRUCT::CimNoteTypeList`.
 * Класс реализован как ленивый синглтон и предназначен для использования из
 * различных потоков, поэтому части его поведения защищены мьютексами.
 */

    /**
     * @class GetinfoBanknoteTypes
     * @brief Сервис получения информации о типах банкнот.
     *
     * Класс отвечает за запрос таблицы номиналов у `DorsPSHandler` и хранит
     * подготовленный список `STRUCT::CimNoteTypeList` в поле `notesList`.
     * Предоставляет методы для заполнения кэша (`fillResult`), получения
     * отформатированной строки (`getResult`) и получения готовой структуры
     * (`GetPreparedData`).
     *
     * Реализован как синглтон через `GetInstance()` — потокобезопасная ленивый
     * инициализация при помощи `m_instanceMutex`.
     */
    class GetinfoBanknoteTypes
    {
        /**
         * @enum ErrCodes
         * @brief Локальные коды ошибок/состояний, возвращаемые внутренними методами.
         */
        enum ErrCodes : uint8_t {
            SUCCESS             = 0,    /**< Успешное выполнение */
            ERR_DEV_NOT_READY   = 1     /**< Устройство не готово/недоступно */
        };

    public:
        /**
         * @brief Конструктор по умолчанию.
         *
         * Закрыт (default) — создание экземпляров извне не рекомендуется,
         * используйте `GetInstance()`.

         Вызывается автоматически при создании экземпляра синглтона.
         */
        GetinfoBanknoteTypes() = default;
        GetinfoBanknoteTypes(const GetinfoBanknoteTypes&) = delete;            /**< Копирование запрещено */
        GetinfoBanknoteTypes& operator=(const GetinfoBanknoteTypes&) = delete; /**< Присваивание запрещено */

        /**
         * @brief Получить глобальный экземпляр (ленивый синглтон).
         *
         * Потокобезопасно создаёт единственный экземпляр класса при первом
         * вызове. Возвращает указатель на статический объект (владелец не
         * освобождает объект автоматически).
         *
         * @return Указатель на экземпляр `GetinfoBanknoteTypes`.
         */
        static GetinfoBanknoteTypes* GetInstance() {
            std::lock_guard<std::mutex> lock(m_instanceMutex);
            if (pinstance_ == nullptr)
            {
                pinstance_ = new GetinfoBanknoteTypes;
            }
            return pinstance_;
        }

        /**
         * @brief Заполнить внутренний кэш `notesList` актуальными типами банкнот.
         *
         * Метод обращается к `m_psHandler` и вызывает `EnsureBillTableIsAvailable()` и
         * затем `GetBanknoteTypes()` для получения актуального списка. Операция
         * защищена мьютексом `m_fillMutex`.
         */
        void fillResult();

        /**
         * @brief Получить отформатированное текстовое представление текущего содержимого `notesList`.
         *
         * @return Строка с результатом; если список пуст, возвращается "EMPTY\n".
         */
        std::string getResult() const;


    private:
        /**
         * @brief Единственный экземпляр синглтона (если создан).
         * @note Освобождение памяти не выполняется автоматически.
         */
        static GetinfoBanknoteTypes* pinstance_;
        static std::mutex m_sendMutex;    /**< Мьютекс для сериализации отправки/получения данных (используется в других местах). */
        static std::mutex m_instanceMutex;/**< Мьютекс для безопасной инициализации синглтона. */
        static std::mutex m_fillMutex;    /**< Мьютекс для защиты `fillResult` и поля `notesList`. */

        std::shared_ptr<XFS4IoTSP::CashAcceptor::Sample::CashAcceptorSample> m_psHandler = nullptr; /**< Набор данных извлекается через этот обработчик (не владеет). */
        
        /**
         * @brief Вспомогательный метод форматирования `notesList` в многострочную строку.
         * @return Отформатированная строка с деталями полей (Tabular view).
         */
        std::string PrintFromResult() const;

        /**
         * @brief Проверить, можно ли выполнять обработку (готовность устройства и корректное состояние).
         *
         * Метод проверяет флаги и текущее состояние устройства через `m_psHandler`.
         * Возвращает код из `ErrCodes`.
         *
         * @return `SUCCESS` (0) при готовности, `ERR_DEV_NOT_READY` при отсутствии готовности.
         */
        uint32_t validateProcessingConditions();
    };

#endif // #ifndef __GETINFOBANKNOTETYPEES_H__
