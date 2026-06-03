#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include "../../../../framework/core/common/StateMachine/StateMachine.hpp"
#include "../../../HW/Dors_210BA.h"

/**
 * @file NotesInhibitManager.hpp
 * @brief Менеджер отложенного запрещения приёма банкнот (Notes Inhibit).
 *
 * Класс `NotesInhibitManager` реализует логику отложенного и немедленного
 * запрещения/разрешения приёма банкнот в устройстве. Поддерживает запуск
 * фоновой нити для задержанного запрещения, отмену отложенной операции,
 * а также непосредственное вызов EnableBillTypes/Return на устройстве через
 * `DorsPSHandler`.
 *
 * Взаимодействие со стейт‑машиной реализовано через `StateMachine` и
 * используется механизм BlockedWait/BlockedWaitTermination для ожидания
 * стабилизации состояний после подачи запрета.
 */

//namespace XfsCommon { class BlockedWaitTermination; } // forward

namespace XFS4IoTSP::CashAcceptor::Sample
{
        class CashAcceptorSample;
        //using SM = XfsCommon::StateMachine< FS365::HW::Dors::DorsHW::POLL_RES >;

        /**
         * @class NotesInhibitManager
         * @brief Управляет запретом/разрешением приёма банкнот с возможностью задержки.
         *
         * Экземпляр класса не владеет `DorsPSHandler*` (указывает на внешний объект).
         * При создании можно указать задержку (в миллисекундах) — время, по истечении
         * которого будет выполнено запрещение приёма (если не будет отмены).
         *
         * Особенности:
         * - Потокобезопасность реализована через внутренний мьютекс `m_mtx` и
         *   атомарные флаги для управления фоновым потоком;
         * - При уничтожении объекта поток задержки гарантированно останавливается
         *   и предпринимаются попытки привести устройство в корректное состояние
         *   (вызов `InhibitAccept()` в деструкторе);
         */
        class NotesInhibitManager {
        public:
            /**
             * @brief Конструктор.
             * @param pHandler Указатель на обработчик `DorsPSHandler` (класс не владеет указателем).
             * @param dwDelayTimeAcceptanceStop Время задержки (в миллисекундах) перед запрещением приёма.
             */
            NotesInhibitManager(std::shared_ptr<CashAcceptorSample> device
                , uint16_t dwDelayTimeAcceptanceStop = 0);

            /**
             * @brief Деструктор — останавливает фоновые операции и пытается выставить inhibit.
             */
            ~NotesInhibitManager();

            /**
             * @brief Отложенное запрещение приёма банкнот.
             *
             * Если `m_dwDelayTimeAcceptanceStop.count() == 0`, запрещение выполняется
             * немедленно. В противном случае запускается фоновый поток, который
             * через заданную задержку выполнит запрещение, если не получит сигнал отмены.
             */
            void DelayInhibitAccept();

            /**
             * @brief Немедленное запрещение приёма (останавливает отложенный поток).
             * @return true при успешном выполнении запрета, false при ошибке/отсутствии связи с устройством.
             */
            bool InhibitAccept();

            /**
             * @brief Немедленное разрешение приёма (останавливает отложенный поток).
             * @return true при успешном выполнении операции, false при ошибке/отсутствии связи с устройством.
             */
            bool AllowAccept();

            /**
             * @brief Проверить, разрешён ли приём (есть ли разрешённые номиналы).
             * @return true если приём разрешён (имеются note ids), false если приём запрещён.
             */
            bool IsAcceptAllowed();

            // Нельзя копировать/перемещать
            NotesInhibitManager(const NotesInhibitManager&) = delete;
            NotesInhibitManager& operator=(const NotesInhibitManager&) = delete;

        private:
            /**
             * @brief Запустить фоновый поток ожидания задержки.
             *
             * Создаёт и запускает `m_thread`, который ждёт указанный интервал и затем
             * вызывает `_inhibit()` при отсутствии отмены.
             */
            void _start();

            /**
             * @brief Остановить фоновый поток ожидания (если запущен) и инициировать отмену.
             */
            void _stop();

            /**
             * @brief Функция фонового потока — ожидание и выполнение запрета.
             */
            void _delay_inhibit_thread();

            /**
             * @brief Выполнить непосредственное запрещение приёма (EnableBillTypes(0,0)).
             *
             * Метод самостоятельно защищает своё состояние (захватывает мьютекс).
             * Производит подписки на стейт-машину, ждёт наступления устойчивых
             * состояний и помечает кеш noteIds как запрещённый.
             *
             * @return true при успешном выполнении, false в случае ошибки.
             */
            bool _inhibit();

            /**
             * @brief Выполнить непосредственное разрешение приёма (EnableBillTypes(m_lBillTypes)).
             *
             * Метод самостоятельно защищает своё состояние (захватывает мьютекс).
             * Обновляет кеш текущих noteIds при успешной операции.
             *
             * @return true при успешном выполнении, false в случае ошибки.
             */
            bool _allow();

        private:
            std::shared_ptr<CashAcceptorSample> device_;

            // синхронизация потока ожидания
            std::mutex m_mtx;                      /**< Мьютекс для защиты состояния и condition_variable */
            std::condition_variable m_cvStop;     /**< Сигнал для прерывания ожидания */
            std::thread m_thread;                 /**< Фоновый поток для отложенного inhibit */
            std::atomic<bool> m_stopRequested{ false };   /**< Флаг запроса остановки фонового потока */
            std::atomic<bool> m_threadRunning{ false };   /**< Флаг, показывающий, что поток запущен */

            // delay в миллисекундах
            std::chrono::milliseconds m_dwDelayTimeAcceptanceStop; /**< Задержка перед запрещением (ms) */

            // Асинхронный токен для прерывания BlockedWait
            std::weak_ptr<XfsCommon::StateMachine< FS365::HW::Dors::DorsHW::POLL_RES >::BlockedWaitTermination> m_p_async_terminator;

            // Кеш текущего набора номиналов
            uint32_t m_ulNoteIds{ 0 };              /**< Битовая маска разрешённых note IDs */
            bool m_bNoteIdsCached{ false };      /**< Флаг, указывающий, что кеш инициализирован */
        };

} 
