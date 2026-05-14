// ------------------------------------------------------------------------------------
// Copyright © 2006, SYSTEMA LLC. All rights reserved.
// System: CashCode FrontLoad BillValidator
//
// Description:
// Реализация класса, инкапсулирующего интерфейс управления купюроприемником 
// CashCode Front Load Bill Validator
// ------------------------------------------------------------------------------------
#ifndef __DORS_210BA_H__
#define __DORS_210BA_H__
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/algorithm/string.hpp>
#include "Identification.h"
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <queue>
#include "./DevicePort/DevicePort.hpp"
#include "../../framework/core/Logger/ILogger.hpp"


using namespace std::chrono_literals;

// Время ответа (в миллисекундах)
constexpr auto RESPONSE_TIME = 1000ms;
// Интервал между подтверждением ACK/NAK и посылкой следующей команды
constexpr auto FREE = 20ms;
// Минимальный интервал времени между двумя POLL командами 
constexpr auto T_POLL_MIN = 100ms;
// Максимальный интервал времени между двумя POLL командами
constexpr auto T_POLL_MAX = 2000ms;

/**
 * @file Dors_210BA.h
 * @brief Заголовочный файл драйвера купюроприёмника DORS 210BA.
 *
 * Файл содержит объявление класса `DorsHW`, который реализует низкоуровневое
 * управление устройством через последовательный порт (boost::asio). Класс
 * выполняет формирование команд протокола, чтение ответов, обработку POLL,
 * управление очередью задач и поток-исполнитель для последовательной отправки
 * команд к устройству.
 */

namespace FS365::HW::Dors {

    /**
     * @class DorsHW
     * @brief Низкоуровневый интерфейс управления купюроприёмником DORS.
     *
     * Обеспечивает отправку команд устройства, чтение и парсинг ответов,
     * обработку POLL-событий, управление кассетами, получение таблицы
     * номиналов и идентификационных данных. Для последовательности операций
     * используется очередь задач и поток-исполнитель.
     */
    class DorsHW {

    public:
        /**
         * @enum DEV_CMD
         * @brief Набор кодов команд устройства ( байтовые коды протокола ).
         */
        enum class DEV_CMD : uint8_t {
            RESET = 0x30,                        /**< Сброс устройства */
            GET_STATUS = 0x31,                   /**< Получить статус */
            SET_SECURITY = 0x32,                 /**< Установить параметры безопасности */
            POLL = 0x33,                         /**< Запрос состояния (POLL) */
            ENABLE_BILL_TYPES = 0x34,            /**< Включить набор принимаемых номиналов */
            STACK = 0x35,                        /**< Команда положить в кассету (STACK) */
            RETURN = 0x36,                       /**< Команда вернуть банкноту (RETURN) */
            IDENTIFICATION = 0x37,               /**< Идентификация модуля */
            HOLD = 0x38,                         /**< Удержать банкноту */
            GET_BILL_TABLE = 0x41,               /**< Получить таблицу номиналов */
            GET_CRC32_OF_THE_CODE = 0x51,        /**< Получить CRC32 */
            VALIDATION_MODULE_IDENTIFICATION = 0x54, /**< Идентификация модуля валидации */
            DIAGNOSTIC_SETTINGS = 0xF0,          /**< Диагностические установки */
            CASSETTE_HIGH_LEVEL = 0xD0,          /**< Управление высоким уровнем кассеты */
            SELECT_ENCRYPT_KEY = 0xD1,           /**< Выбор ключа шифрования */
            REBOOT = 0xD2,                       /**< Перезагрузка модуля */
            SET_STATISTIC = 0xD3,                /**< Установить статистику */
            GET_STATISTIC = 0xD4,                /**< Получить статистику */
            CASSETTE_CONTROL = 0xD5,             /**< Управление кассетой */
            STATES_STACK_TRANSFER_ENABLE = 0xD6, /**< Режим передачи полного стека состояний */
        };

        inline static std::string_view DevCmdToString(DEV_CMD cmd) {
            switch (cmd) {
            case DEV_CMD::RESET: return "RESET";
            case DEV_CMD::GET_STATUS: return "GET_STATUS";
            case DEV_CMD::SET_SECURITY: return "SET_SECURITY";
            case DEV_CMD::POLL: return "POLL";
            case DEV_CMD::ENABLE_BILL_TYPES: return "ENABLE_BILL_TYPES";
            case DEV_CMD::STACK: return "STACK";
            case DEV_CMD::RETURN: return "RETURN";
            case DEV_CMD::IDENTIFICATION: return "IDENTIFICATION";
            case DEV_CMD::HOLD: return "HOLD";
            case DEV_CMD::GET_BILL_TABLE: return "GET_BILL_TABLE";
            case DEV_CMD::GET_CRC32_OF_THE_CODE: return "GET_CRC32_OF_THE_CODE";
            case DEV_CMD::VALIDATION_MODULE_IDENTIFICATION: return "VALIDATION_MODULE_IDENTIFICATION";
            case DEV_CMD::DIAGNOSTIC_SETTINGS: return "DIAGNOSTIC_SETTINGS";
            case DEV_CMD::CASSETTE_HIGH_LEVEL: return "CASSETTE_HIGH_LEVEL";
            case DEV_CMD::SELECT_ENCRYPT_KEY: return "SELECT_ENCRYPT_KEY";
            case DEV_CMD::REBOOT: return "REBOOT";
            case DEV_CMD::SET_STATISTIC: return "SET_STATISTIC";
            case DEV_CMD::GET_STATISTIC: return "GET_STATISTIC";
            case DEV_CMD::CASSETTE_CONTROL: return "CASSETTE_CONTROL";
            case DEV_CMD::STATES_STACK_TRANSFER_ENABLE: return "STATES_STACK_TRANSFER_ENABLE";
            default: return "UNKNOWN_COMMAND";
            }
		}

        /**
         * @enum POLL_RES
         * @brief Коды состояний, возвращаемых командой POLL (включая ошибки и детальные причины).
         *
         * Некоторые значения кодируют детальные причины отказа (группы 0x47, 0x1C и т.п.).
         */
        enum class POLL_RES : uint32_t {
            PowerUp = 0x10,                       /**< Подача питания */
            PowerUpWithBillInValidator = 0x11,    /**< Подача питания — банкнота в валидаторе */
            PowerUpWithBillInStacker = 0x12,      /**< Подача питания — банкнота в стакере */
            Initialize = 0x13,                    /**< Инициализация */
            Idling = 0x14,                        /**< Ожидание */
            Accepting = 0x15,                     /**< Приём банкноты */
            Stacking = 0x17,                      /**< Закладка в кассету */
            Returning = 0x18,                     /**< Возврат банкноты */
            UnitDisabled = 0x19,                  /**< Унита отключена */
            Holding = 0x1A,                       /**< Удержание банкноты */
            Rejecting = 0x1C,                     /**< Отклонение банкноты (группа) */
            RejInsertion = 0x601C,                /**< Отклонение: вставка */
            RejMagnetic = 0x611C,                 /**< Отклонение: магнитный признак */
            RejBillInHead = 0x621C,               /**< Отклонение: застряла в голове */
            RejMultiplying = 0x631C,              /**< Отклонение: умножение */
            RejConveying = 0x641C,                /**< Отклонение: транспорт */
            RejIdentification = 0x651C,           /**< Отклонение: ошибка идентификации */
            RejNotebase = 0x661C,                 /**< Отклонение: проблема с нотой-базой */
            RejIAS = 0x671C,                      /**< Отклонение: IAS */
            RejInhibit = 0x681C,                  /**< Отклонение: запрет приёма (inhibit) */
            RejDensity = 0x691C,                  /**< Отклонение: плотность */
            RejOperation = 0x6A1C,                /**< Отклонение: операция */
            RejLength = 0x6C1C,                   /**< Отклонение: длина */
            RejUV = 0x6D1C,                       /**< Отклонение: UV */
            RejUnknown = 0xFF1C,                  /**< Неизвестная причина отказа */
            DropCassetteFull = 0x41,              /**< Кассета заполнена */
            DropCassetteOutOfPosition = 0x42,     /**< Кассета смещена */
            ValidatorJammed = 0x43,               /**< Валидатор застрял */
            DropCassetteJammed = 0x44,            /**< Кассета заклинила */
            Cheated = 0x45,                       /**< Подозрение на мошенничество */
            Pause = 0x46,                         /**< Пауза */
            StackMotorFail = 0x5047,              /**< Отказ мотора стакера */
            TransportMotorFail = 0x5247,          /**< Отказ транспортного мотора */
            InitialCassetteStatusFail = 0x5447,   /**< Ошибка начального состояния кассеты */
            OpticCanalFail = 0x5547,              /**< Отказ оптического канала */
            MagneticCanalFail = 0x5647,           /**< Отказ магнитного канала */
            Group47UnknownFailure = 0xFF47,       /**< Неизвестная ошибка группы 0x47 */
            EscrowPos = 0x80,                     /**< Позиция эскроу */
            BillStacked = 0x81,                   /**< Банкнота положена в кассету */
            BillReturned = 0x82,                  /**< Банкнота возвращена */
            RejTape = 0xD01C,                     /**< Отклонение: обнаружена лента/скотч */
            RejEntryCassetteSns = 0xD11C,         /**< Отклонение: сенсор кассеты на входе */
            RejFastConveying = 0xD21C,            /**< Отклонение: быстрая подача */
            RejTrayClosed = 0xD31C,               /**< Отклонение: лоток закрыт */
            RejTimeout = 0xD41C,                  /**< Отклонение: таймаут */
            FishingDetected = 0xD0,               /**< Обнаружено мошенничество (фишинг) */
            CassetteBracketOpen = 0xD1,            /**< Скоба кассеты открыта */
            StartTrayFailure = 0xD047,            /**< Отказ старта лотка */
            RejFastFeed = 0xD51C,                 /**< Отклонение: быстрая подача */
            Send_States_Stack = 0xDE,             /**< Передача набора состояний (stack) */
            Undefined = 0xDF,                     /**< Неопределённое состояние */
            PortError = 0x1,                      /**< Ошибка порта */
            CrcError = 0x2,                       /**< Ошибка CRC */
            Unknown = 0x3,                        /**< Неизвестный ответ */
            IllegalCommand = 0x4,                 /**< Нелегальная команда */
            Nak = 0x5,                            /**< Получен NAK */
            RejTrayLengthShort = 0xD61C,          /**< Отклонение: лоток слишком короткий */
            PowerNoiseFailure = 0xD147,           /**< Отказ по шуму питания */
        };

        inline static std::string_view PollResToString(POLL_RES res) {
            switch (res) {
            case POLL_RES::PowerUp: return "PowerUp";
            case POLL_RES::PowerUpWithBillInValidator: return "PowerUpWithBillInValidator";
            case POLL_RES::PowerUpWithBillInStacker: return "PowerUpWithBillInStacker";
            case POLL_RES::Initialize: return "Initialize";
            case POLL_RES::Idling: return "Idling";
            case POLL_RES::Accepting: return "Accepting";
            case POLL_RES::Stacking: return "Stacking";
            case POLL_RES::Returning: return "Returning";
            case POLL_RES::UnitDisabled: return "UnitDisabled";
            case POLL_RES::Holding: return "Holding";
            case POLL_RES::Rejecting: return "Rejecting";
            case POLL_RES::RejInsertion: return "RejInsertion";
            case POLL_RES::RejMagnetic: return "RejMagnetic";
            case POLL_RES::RejBillInHead: return "RejBillInHead";
            case POLL_RES::RejMultiplying: return "RejMultiplying";
            case POLL_RES::RejConveying: return "RejConveying";
            case POLL_RES::RejIdentification: return "RejIdentification";
            case POLL_RES::RejNotebase: return "RejNotebase";
            case POLL_RES::RejIAS: return "RejIAS";
            case POLL_RES::RejInhibit: return "RejInhibit";
            case POLL_RES::RejDensity: return "RejDensity";
            case POLL_RES::RejOperation: return "RejOperation";
            case POLL_RES::RejLength: return "RejLength";
            case POLL_RES::RejUV: return "RejUV";
            case POLL_RES::RejUnknown: return "RejUnknown";
            case POLL_RES::DropCassetteFull: return "DropCassetteFull";
            case POLL_RES::DropCassetteOutOfPosition: return "DropCassetteOut";
            case POLL_RES::ValidatorJammed: return "ValidatorJammed";
            case POLL_RES::DropCassetteJammed: return "DropCassetteJammed";
            case POLL_RES::Cheated: return "Cheated";
            case POLL_RES::Pause: return "Pause";
            case POLL_RES::StackMotorFail: return "StackMotorFail";
            case POLL_RES::TransportMotorFail: return "TransportMotorFail";
            case POLL_RES::InitialCassetteStatusFail: return "InitialCassetteStatusFail";
            case POLL_RES::OpticCanalFail: return "OpticCanalFail";
            case POLL_RES::MagneticCanalFail: return "MagneticCanalFail";
            case POLL_RES::Group47UnknownFailure: return "Group47UnknownFailure";
            case POLL_RES::EscrowPos: return "EscrowPos";
            case POLL_RES::BillStacked: return "BillStacked";
            case POLL_RES::BillReturned: return "BillReturned";
            case POLL_RES::RejTape: return "RejTape";
            case POLL_RES::RejEntryCassetteSns: return "RejEntryCassetteSns";
            case POLL_RES::RejFastConveying: return "RejFastConveying";
            case POLL_RES::RejTrayClosed: return "RejTrayClosed";
            case POLL_RES::RejTimeout: return "RejTimeout";
            case POLL_RES::FishingDetected: return "FishingDetected";
            case POLL_RES::CassetteBracketOpen: return "CassetteBracketOpen";
            case POLL_RES::StartTrayFailure: return "StartTrayFailure";
            case POLL_RES::RejFastFeed: return "RejFastFeed";
            case POLL_RES::Send_States_Stack: return "Send_States_Stack";
            case POLL_RES::Undefined: return "Undefined";
            case POLL_RES::PortError: return "PortError";
            case POLL_RES::CrcError: return "CrcError";
            case POLL_RES::Unknown: return "Unknown";
            case POLL_RES::IllegalCommand: return "IllegalCommand";
            case POLL_RES::Nak: return "Nak";
            case POLL_RES::RejTrayLengthShort: return "RejTrayLengthShort";
            case POLL_RES::PowerNoiseFailure: return "PowerNoiseFailure";
            default: return "UnknownPollRes";
            }
        }

        /**
         * @brief Утилиты для проверки типа состояния POLL_RES.
         * @param state Код состояния.
         * @return true/false в зависимости от типа состояния.
         */
        static bool _isRejCode(POLL_RES state);
        static bool _isInoperative(POLL_RES state);
        static bool _isHWError(POLL_RES state);
        static bool _isOnline(POLL_RES state);
        static bool _isPowerUp(POLL_RES state);
        static bool _isNoteDetected(POLL_RES state);
        static const std::set<POLL_RES>& GetRejectStates();
        static const std::set<POLL_RES>& GetErrorStates();

        /**
         * @enum RESULT
         * @brief Результат выполнения операций отправки/приёма команд.
         */
        enum class RESULT {
            Err_Ack,     /**< Ожидается ACK, но не получен */
            Err_Nak,     /**< Получен NAK */
            Err_Crc,     /**< Ошибка CRC */
            Err_Unknown, /**< Неизвестная ошибка */
            Err_Port,    /**< Ошибка порта/коммуникации */
            Err_Illegal, /**< Нелегальная команда */
            Ok           /**< Успешно */
        };

        inline static std::string_view ResultToString(RESULT res) {
            switch (res) {
            case RESULT::Err_Ack: return "Err_Ack";
            case RESULT::Err_Nak: return "Err_Nak";
            case RESULT::Err_Crc: return "Err_Crc";
            case RESULT::Err_Unknown: return "Err_Unknown";
            case RESULT::Err_Port: return "Err_Port";
            case RESULT::Err_Illegal: return "Err_Illegal";
            case RESULT::Ok: return "Ok";
            default: return "UnknownResult";
            }
		}

        /// Коды возврата операции считывания
        enum class DEVICE_READ_RESULT {
            ERR_OK = 0,        /**< Успешное завершение */
            ERR_FAILED = 1,    /**< Провал операции */
            ERR_CANCELED = 2,  /**< Возврат по отмене операции */
            ERR_TIMEOUT = 3,   /**< Истёк таймаут на считывание данных */
            ERR_DSR_OFF = 4,   /**< Обнаружен обрыв DSR */
            ERR_COMM_DISCONNECTED = 5 /**< COM-порт отключён во время операции */
        };

        inline static std::string_view DeviceReadResultToString(DEVICE_READ_RESULT res) {
            switch (res) {
            case DEVICE_READ_RESULT::ERR_OK: return "ERR_OK";
            case DEVICE_READ_RESULT::ERR_FAILED: return "ERR_FAILED";
            case DEVICE_READ_RESULT::ERR_CANCELED: return "ERR_CANCELED";
            case DEVICE_READ_RESULT::ERR_TIMEOUT: return "ERR_TIMEOUT";
            case DEVICE_READ_RESULT::ERR_DSR_OFF: return "ERR_DSR_OFF";
            case DEVICE_READ_RESULT::ERR_COMM_DISCONNECTED: return "ERR_COMM_DISCONNECTED";
            default: return "UnknownDeviceReadResult";
            }
        }


        /**
         * @struct Task
         * @brief Описание задачи, помещаемой в очередь исполнителя.
         *
         * Хранит команду, буфер для результата, таймаут и callback для обработки результата.
         */
        struct Task {
            std::span<uint8_t> command;                /**< Буфер команды */
            int commandSize;                           /**< Фактический размер команды */
            std::span<uint8_t> result;                 /**< Буфер для приёма ответа */
            size_t* resultSize;                        /**< Указатель на переменную для записи размера ответа */
            std::chrono::milliseconds timeout;         /**< Таймаут ожидания ответа */
            bool sendAck;                              /**< Нужно ли отправлять ACK при успешном ответе */
            std::function<void(DorsHW::RESULT)> callback; /**< Callback вызывается после завершения задачи */
            uint8_t taskId;                            /**< Идентификатор задачи (назначается при постановке в очередь) */
        };

        /**
         * @brief Конструктор DorsHW.
         * @param portName Имя последовательного порта (например, "COM1" или "/dev/ttyUSB0").
         * @param baudRate Скорость порта в бодах.
         * @param byteSize Количество бит данных (обычно 8).
         * @param parity Параметр чётности (boost::asio::serial_port_base::parity::type).
         * @param stopBits Количество стоп-битов (boost::asio::serial_port_base::stop_bits::type).
         * @param skipDeviceReset Если true — пропустить автоматический сброс устройства при инициализации.
         */
        DorsHW(
            const std::string& portName,
            uint32_t baudRate,
            uint8_t byteSize,
            boost::asio::serial_port_base::parity::type parity,
            boost::asio::serial_port_base::stop_bits::type stopBits,
            std::shared_ptr<ILogger> log
        );

        /**
         * @brief Деструктор. Останавливает исполнитель и закрывает порт.
         */
        virtual ~DorsHW();

        /**
         * @brief Отправить команду сброса на устройство.
         * @return Результат выполнения.
         */
        RESULT Reset();

        /**
         * @brief Выполнить POLL и получить состояние устройства.
         * @param additionalRes [out] Дополнительный параметр состояния (при наличии).
         * @return Код состояния POLL_RES.
         */
        POLL_RES Poll(uint8_t& additionalRes);

        /**
         * @brief Выполнить POLL и получить состояние и дополнительные сервисные байты.
         * @param additionalRes [out] Дополнительный параметр состояния (при наличии).
         * @param hwServiceBytes [out] Вектор сервисных байтов, полученных от устройства.
         * @return Код состояния POLL_RES.
         */
        POLL_RES Poll(uint8_t& additionalRes, std::vector<uint8_t>& hwServiceBytes);

    protected:
        /**
         * @brief Разобрать три байта ответа POLL и вернуть соответствующее состояние.
         * @param firstByte Первый байт ответа.
         * @param secondByte Второй байт ответа.
         * @param thirdByte Третий байт ответа.
         * @param responseLen [out] Длина значимой части ответа (в байтах).
         * @param additionalRes [out] Дополнительный код состояния (если применимо).
         * @return Распознанное состояние POLL_RES.
         */
        POLL_RES Poll_State(uint8_t firstByte, uint8_t secondByte, uint8_t thirdByte, uint8_t& responseLen, uint8_t& additionalRes);

    public:
        /**
         * @brief Получить текущую конфигурацию billEnabled и security из ответа устройства.
         * @param billEnabled [out] Битовая маска включённых номиналов.
         * @param security [out] Параметры безопасности.
         * @return Результат операции.
         */
        RESULT GetStatus(uint32_t& billEnabled, uint32_t& security);

        /**
         * @brief Включить набор типов купюр для приёма.
         * @param billTypes Битовая маска типов купюр.
         * @param billsWithEscrow Маска купюр, которые должны оставаться в эскроу до подтверждения.
         * @return Результат операции.
         */
        RESULT EnableBillTypes(uint32_t billTypes, uint32_t billsWithEscrow);

        /**
         * @brief Получить таблицу номиналов устройства (GetBillTable).
         * @param bills Буфер размером 120 байт для записи таблицы.
         * @return Результат операции.
         */
        RESULT GetBillTable(std::span<uint8_t, 120> bills);

        RESULT Stack();   /**< Положить банкноту в кассету */
        RESULT Return();  /**< Вернуть банкноту клиенту */
        RESULT Hold();    /**< Удержать банкноту */

        // Идентификационные данные устройства
        RESULT Identification(CDbaIdentification& dbaIdn);
        RESULT Identification(std::string& partNumber, std::string& serialNumber);
        virtual RESULT Identification(CIdentification& idn);

        /**
         * @brief Утилита преобразования различных типов в строковое представление для логов.
         * @tparam T Тип значения.
         * @param value Входное значение.
         * @return std::string Представление значения.
         *
         * Поддерживаются: std::string, const char*, bool, арифметические типы; для прочих типов используется std::format.
         */
        template<typename T>
        auto to_string(const T& value) {
            if constexpr (std::is_same_v<T, std::string>) {
                return value; // std::string возвращается как есть
            }
            else if constexpr (std::is_same_v<T, const char*>) {
                return std::string(value); // const char* преобразуется в std::string
            }
            else if constexpr (std::is_same_v<T, bool>) {
                return value ? "true" : "false"; // Булевые значения преобразуются в "true" или "false"
            }
            else if constexpr (std::is_arithmetic_v<T>) {
                return std::to_string(value); // Числовые типы преобразуются в строку
            }
            else {
                return std::format("{}", value); // Используем std::format для остальных типов
            }
        }

        /**
         * @brief Вывести в stdout конкатенацию переданных аргументов (используется для отладки).
         * @tparam Args Список типов аргументов.
         * @param args Аргументы для вывода.
         */
        template<typename... Args>
        void println(const Args&... args) {
            std::string result = (std::string(" ") + ... + to_string(args)); // Собираем все аргументы в строку

            if (!result.empty()) { // Удаляем начальный пробел, если строка не пустая
                result.erase(0, 1);
            }
            std::cout << result << std::endl;
        }

        [[nodiscard]] bool IsDeviceInitialized(); /**< true если порт и устройство инициализированы */
        bool m_bResetOperationInProgress = false; /**< Флаг, когда операция reset в процессе */

        /// Перезагрузка ПО устройства
        virtual RESULT Reboot();

        /// Режим работы датчика наличия кассеты
        virtual RESULT cassette_control(bool enable);

        DorsHW::RESULT CassetteHighLevel(bool bHigh);

        /// Режим передачи полного стека состояний купюроприёмника
        virtual RESULT states_stack_transfer_enable(bool enable);

        /**
         * @brief Установить флаг инициализации порта (внешнее управление).
         * @param initialized true если порт инициализирован.
         */
        void SetPortInitialized(bool initialized) {
            m_bPortIsInitialized = initialized;
        }

        /**
         * @brief Постановка задачи в очередь исполнителя.
         * @param task Задача для выполнения.
         */
        void EnqueueCommand(Task& task);

        /**
         * @brief Запустить поток-исполнитель задач.
         */
        void StartExecutor();

        /**
         * @brief Остановить поток-исполнитель.
         */
        void StopExecutor();

        bool tempVarIsCurReq = false;       /**< Временный флаг: есть текущий запрос */

    protected:

        /**
         * @brief Получить длину полезной части ответа POLL для конкретного состояния.
         * @param state Код состояния.
         * @return Длина полезной части в байтах.
         */
        virtual int _getPollDataLength(POLL_RES state);

        /**
         * @brief Инициализировать последовательный порт с заданными параметрами.
         * @return true при успешной инициализации.
         */
        bool _initialize_port(void);

        /**
         * @brief Проверить наличие устройства на указанном порту.
         * @param port Имя порта для проверки.
         * @return true если устройство обнаружено.
         */
        bool CheckingTheDevice(const std::string& port);

        // Контекст и объект порта (boost::asio)
        std::unique_ptr<boost::asio::io_context> m_ioContext; /**< Контекст asio */
        std::unique_ptr<DevicePort> m_port;                   /**< Объект порта */
        uint32_t m_baudRate;                                  /**< Скорость порта */
        uint8_t m_byteSize;                                   /**< Размер байта данных */
        boost::asio::serial_port_base::parity m_parity;       /**< Параметр чётности */
        boost::asio::serial_port_base::stop_bits m_stopBits;  /**< Стоп-биты */
        bool m_bPortIsInitialized = false;                    /**< Флаг инициализации порта */
        std::string m_strFirmware;                            /**< Номер/версия прошивки устройства */
        std::mutex m_portMutex;                              /**< Мьютекс для последовательного доступа к порту */
        POLL_RES m_lastState = POLL_RES::Unknown;            /**< Последнее прочитанное состояние устройства */
        uint8_t m_ucCommandCounter = 0x03;                    /**< Счётчик команд протокола (инкрементируется) */

        static std::mutex m_SendCommand_mutex;               /**< Статический мьютекс для сериализации отправки команд */

        /**
         * @brief Вычислить CRC для блока данных.
         * @param data Входной диапазон байтов.
         * @return 16-битный CRC.
         */
        [[nodiscard]] uint16_t GetCrc(std::span<const uint8_t> data);

        /**
         * @brief Добавить CRC в конец команды (последние два байта).
         * @param data Буфер команды (должен иметь место для CRC).
         * @param commandSize Размер команды (включая CRC).
         */
        void AddCrc(std::span<uint8_t> data, int commandSize);

        void SendAck(); /**< Отправить ACK устройству */
        void SendNak(); /**< Отправить NAK устройству */
        void ComPortReadToNull(std::chrono::milliseconds timeout); /**< Прочитать и проигнорировать входящие байты в течение timeout */

        [[nodiscard]] bool IsAck(std::span<const uint8_t> data, int resultSize);     /**< Проверить, соответствует ли ответ ACK */
        [[nodiscard]] bool IsNak(std::span<const uint8_t> data, int resultSize);     /**< Проверить, соответствует ли ответ NAK */
        [[nodiscard]] bool IsIllegal(std::span<const uint8_t> data, int resultSize); /**< Проверить, является ли ответ «illegal command» */

        /**
         * @brief Отправить команду и дождаться ответа (с повторными попытками при ошибках).
         * @param command Буфер команды (включая служебную часть и CRC место).
         * @param commandSize Длина команды в байтах.
         * @param result Буфер для приёма ответа.
         * @param resultSize [out] Результирующий размер принятого ответа.
         * @param timeout Таймаут на приём ответа.
         * @param sendAck Отправлять ли ACK при успешном приёме.
         * @return RESULT Код результата отправки/приёма.
         */
        RESULT SendCommand(
            std::span<uint8_t> command,
            int commandSize,
            std::span<uint8_t> result,
            size_t& resultSize,
            std::chrono::milliseconds timeout = RESPONSE_TIME,
            bool sendAck = true
        );

        /**
         * @brief Получить текущее время в строковом формате для логов (HH:MM:SS:ms).
         * @return Отформатированная временная строка.
         */
        std::string getCurrentTimeFormatted() {
            // Получаем текущее системное время
            auto now = std::chrono::system_clock::now();

            // Конвертируем в time_t для получения компонентов времени
            std::time_t current_time = std::chrono::system_clock::to_time_t(now);

            // Получаем миллисекунды
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

            // Форматируем время в строку
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&current_time), "%H:%M:%S:")
                << std::setw(4) << std::setfill('0') << ms.count();

            return oss.str();
        }

    private:
        mutable std::mutex m_coutMutex;             /**< Мьютекс для защиты вывода */
        std::queue<Task> m_taskQueue;               /**< Очередь задач */
        mutable std::mutex m_queueMutex;            /**< Мьютекс для защиты очереди */
        std::condition_variable m_queueCv;          /**< Условная переменная для ожидания задач */
        std::unique_ptr<std::jthread> m_executorThread; /**< Поток-исполнитель задач */
        bool m_running = false;                     /**< Флаг работы исполнителя */
        uint8_t m_nextTaskId = 0;                   /**< Счётчик для идентификаторов задач */

        std::shared_ptr<ILogger> m_log;             /**< @brief Логгер (spdlog-обёртка). */

        /**
         * @brief Основной цикл потока-исполнителя: извлекает задачи из очереди и выполняет SendCommand,
         * затем вызывает callback задачи.
         */
        void ExecutorThread();


    };

} // namespace FS365::HW::Dors

#endif // #ifndef __DORS_210BA_H__
