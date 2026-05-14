#ifndef __GETINFOCURRENCYEXP_H__
#define __GETINFOCURRENCYEXP_H__

#include <mutex>
#include "ComTransport/DorsPSHandler.h"
#include "Network/TcpServer.h"
#include "MODULE_STRUCTURES/CimCurrencyExp/CimCurrencyExp.h"

/**
 * @file GetinfoCurrencyExp.h
 * @brief Получение списка валют и их экспонент (Currency Exponent).
 *
 * Заголовочный файл содержит объявление класса `GetinfoCurrencyExp`, который
 * отвечает за получение списка поддерживаемых валют и соответствующих им
 * экспонент (число знаков после запятой) из слоя обработки устройства
 * `DorsPSHandler` и предоставляет готовые данные в виде `STRUCT::CimCurrencyExpList`.
 *
 * Класс реализован как ленивый синглтон и предназначен для многопоточного
 * использования: операции, изменяющие внутренний кэш, защищены мьютексом.
 */

namespace Xfs::Cim {

    /**
     * @class GetinfoCurrencyExp
     * @brief Сервис формирования и кэширования списка валют и экспонент.
     *
     * Класс хранит внутренний кэш `expList` типа `STRUCT::CimCurrencyExpList` и
     * предоставляет методы для его обновления (`fillResult`) и получения
     * подготовленных данных (`GetPreparedData`). Для интеграции с системой
     * предусмотрены методы `setHandler` и `setServer`, которые устанавливают
     * ссылки на `DorsPSHandler` и `TcpServer` (класс не владеет этими указателями).
     *
     * Потокобезопасность:
     * - `m_instanceMutex` защищает инициализацию синглтона;
     * - `m_fillMutex` защищает доступ к `expList` при заполнении/чтении.
     */
    class GetinfoCurrencyExp
    {
    public:
        GetinfoCurrencyExp() = default;
        GetinfoCurrencyExp(const GetinfoCurrencyExp&) = delete;            /**< Копирование запрещено */
        GetinfoCurrencyExp& operator=(const GetinfoCurrencyExp&) = delete; /**< Присваивание запрещено */

        /**
         * @brief Получить глобальный экземпляр (ленивый синглтон).
         *
         * При первом вызове создаётся экземпляр класса. Инициализация
         * защищена `m_instanceMutex`.
         *
         * @return Указатель на единственный экземпляр `GetinfoCurrencyExp`.
         */
        static GetinfoCurrencyExp* GetInstance() {
            std::lock_guard<std::mutex> lock(m_instanceMutex);
            if (pinstance_ == nullptr)
            {
                pinstance_ = new GetinfoCurrencyExp;
            }
            return pinstance_;
        }

        /**
         * @brief Установить обработчик устройства, используемый для получения данных.
         * @param handler Ссылка на существующий `DorsPSHandler` (класс не владеет указателем).
         */
        void setHandler(DorsPSHandler& handler) noexcept {
            m_psHandler = &handler;
        }

        /**
         * @brief Установить сетевой сервер (опционально) для отправки уведомлений.
         * @param server Ссылка на `TcpServer` (класс не владеет указателем).
         */
        void setServer(TcpServer& server) noexcept {
            m_server = &server;
        }

        /**
         * @brief Заполнить внутренний кэш `expList` актуальными данными.
         *
         * Метод вызывает соответствующие методы `DorsPSHandler` для получения
         * списка валют и их экспонент и сохраняет результат в `expList`.
         * Операция защищена мьютексом `m_fillMutex`.
         */
        void fillResult();

        /**
         * @brief Получить подготовленный список валют и экспонент.
         *
         * Возвращает копию внутреннего кэша `expList`. Если необходимо —
         * сначала вызывается `fillResult()` извне. Чтение защищено
         * `m_fillMutex`.
         *
         * @return Копия `STRUCT::CimCurrencyExpList`.
         */
        STRUCT::CimCurrencyExpList GetPreparedData() const
        {
            std::lock_guard<std::mutex> lock(m_fillMutex);
            return expList;
        }

    private:
        /**
         * @brief Указатель на единственный экземпляр (синглтон).
         * @note Освобождение памяти не выполняется автоматически.
         */
        static GetinfoCurrencyExp* pinstance_;

        /**
         * @brief Мьютекс для синхронизации операций отправки/логики (не всегда используется прямо в этом классе).
         */
        static std::mutex m_sendMutex;

        /**
         * @brief Мьютекс для инициализации синглтона.
         */
        static std::mutex m_instanceMutex;

        /**
         * @brief Мьютекс для защиты операции заполнения и чтения `expList`.
         */
        static std::mutex m_fillMutex;

        /**
         * @brief Указатель на обработчик PS, используемый для получения данных (не владеет).
         */
        DorsPSHandler* m_psHandler = nullptr;

        /**
         * @brief Указатель на сетевой сервер (не владеет).
         */
        TcpServer* m_server = nullptr;

        /**
         * @brief Внутренний кэш списка валют и экспонент.
         */
        STRUCT::CimCurrencyExpList expList;

        /**
         * @brief Вернуть текстовое представление текущего содержимого `expList`.
         *
         * Используется для логирования и отладки.
         *
         * @return Многострочная строка с перечислением пар CurrencyID/Exponent.
         */
        std::string PrintFromResult() const;
    };

}

#endif // #ifndef __GETINFOCURRENCYEXP_H__
