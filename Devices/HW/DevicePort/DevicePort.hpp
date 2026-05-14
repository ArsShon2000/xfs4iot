/**
 * @file DevicePort.hpp
 * @brief Класс для работы с последовательным портом устройства через Boost.Asio.
 * @details Требует Boost.Asio 1.88.0, C++23.
 */

#pragma once

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <chrono>
#include <atomic>
#include <thread>
#include <vector>
#include <span>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <functional>
#include <iostream>
#include <algorithm>
#include <ranges>
#include "../../../framework/core/Logger/ILogger.hpp"

/**
 * @class DevicePort
 * @brief Управление последовательным портом устройства, поддержка протокола ACK/NAK, CRC, переподключение.
 */
class DevicePort {
public:
    /**
     * @enum RESULT
     * @brief Результаты выполнения команд и ошибок протокола.
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

    /**
     * @brief Конструктор класса DevicePort.
     * @param port_name Имя последовательного порта (например, "COM1").
     * @param external_io Указатель на внешний io_context (опционально).
     */
    DevicePort(std::string_view port_name,
        std::shared_ptr<ILogger> log = nullptr,
        boost::asio::io_context* external_io = nullptr);

    /**
     * @brief Деструктор класса DevicePort.
     */
    ~DevicePort();

    /**
     * @brief Установить скорость передачи данных (baud rate).
     * @param baud Скорость передачи.
     */
    void setBaudRate(unsigned int baud) { baudRate_ = baud; }

    /**
     * @brief Установить размер символа (бит).
     * @param size Размер символа.
     */
    void setCharacterSize(unsigned int size) { charSize_ = size; }

    /**
     * @brief Установить режим четности.
     * @param p Параметры четности.
     */
    void setParity(boost::asio::serial_port_base::parity p) { parity_ = p; }

    /**
     * @brief Установить количество стоп-бит.
     * @param s Параметры стоп-бит.
     */
    void setStopBits(boost::asio::serial_port_base::stop_bits s) { stopBits_ = s; }

    /**
     * @brief Установить количество попыток отправки команды.
     * @param r Количество попыток.
     */
    void setRetries(int r) { retries_ = r; }

    /**
     * @brief Установить таймаут чтения.
     * @param t Таймаут в миллисекундах.
     */
    void setReadTimeout(std::chrono::milliseconds t) { readTimeout_ = t; }

    /**
     * @brief Установить имя последовательного порта.
     * @param name Имя порта.
     */
    void setPortName(const std::string& name) { m_portName = name; }

   
    /**
     * @brief Установить задержку между попытками.
     * @param d Задержка в миллисекундах.
     */
    void setAttemptDelay(std::chrono::milliseconds d) { attemptDelay_ = d; }

    /**
     * @brief Открыть последовательный порт.
     * @return true если порт успешно открыт, иначе false.
     * @throws boost::system::system_error при ошибке открытия.
     */
    bool open();

    /**
     * @brief Закрыть последовательный порт.
     */
    void close();

    /**
     * @brief Проверить, открыт ли порт.
     * @return true если порт открыт, иначе false.
     */
    bool isOpen() const {
        return portValid_.load();
    }

    /**
     * @brief Запустить фоновый мониторинг переподключения.
     */
    void startReconnectMonitor();

    /**
     * @brief Остановить фоновый мониторинг переподключения.
     */
    void stopReconnectMonitor();

    /**
     * @brief Отправить команду устройству и получить ответ.
     * @param command Буфер команды.
     * @param commandSize Размер команды.
     * @param result Буфер для результата.
     * @param resultSize Размер полученного результата.
     * @param timeout Таймаут выполнения.
     * @param sendAck Отправлять ли ACK после выполнения.
     * @return Результат выполнения команды.
     * @note Блокирует вызывающий поток до завершения.
     */
    RESULT sendCommand(std::span<uint8_t> command, int commandSize,
        std::span<uint8_t> result, size_t& resultSize,
        std::chrono::milliseconds timeout, bool sendAck);

    // --- helpers (protocol specific) ---

    /**
     * @brief Вычислить CRC для данных.
     * @param data Входные данные.
     * @return Значение CRC.
     */
    static uint16_t GetCrc(std::span<const uint8_t> data) {
        uint16_t crc = 0;
        for (auto byte : data) {
            crc ^= byte;
            for (int j = 0; j < 8; ++j) {
                if (crc & 0x0001) {
                    crc >>= 1;
                    crc ^= 0x08408; // оставил вашу константу (как было)
                }
                else {
                    crc >>= 1;
                }
            }
        }
        return crc;
    }

    /**
     * @brief Добавить CRC в конец команды.
     * @param data Буфер команды.
     * @param commandSize Размер команды.
     */
    static void AddCrc(std::span<uint8_t> data, int commandSize) {
        if (commandSize < 2) return;
        auto crc = GetCrc(data.subspan(0, commandSize - 2));
        data[commandSize - 2] = static_cast<uint8_t>(crc & 0xFF);
        data[commandSize - 1] = static_cast<uint8_t>(crc >> 8);
    }

    /**
     * @brief Отправить ACK устройству.
     */
    void SendAck() {
        uint8_t command[] = { 0x02, static_cast<uint8_t>(commandCounter_), 0x06, 0x00, 0xC2, 0x82 };
        AddCrc(std::span(command, 6), 6);
        writeRaw(command, sizeof(command));
    }

    /**
     * @brief Отправить NAK устройству.
     */
    void SendNak() {
        uint8_t command[] = { 0x02, static_cast<uint8_t>(commandCounter_), 0x06, 0xFF, 0xBA, 0x8D };
        AddCrc(std::span(command, 6), 6);
        writeRaw(command, sizeof(command));
    }

    /**
     * @brief Проверить, является ли ответ ACK.
     * @param data Буфер ответа.
     * @param resultSize Размер ответа.
     * @param counter Счетчик команд.
     * @return true если это ACK, иначе false.
     */
    static bool IsAck(std::span<const uint8_t> data, int resultSize, uint8_t counter) {
        if (resultSize != 6) return false;
        uint8_t command[] = { 0x02, counter, 0x06, 0x00, 0xC2, 0x82 };
        uint8_t tmp[6]; std::copy(std::begin(command), std::end(command), tmp);
        AddCrc(std::span(tmp, 6), 6);
        return std::equal(data.begin(), data.begin() + resultSize, tmp, tmp + 6);
    }

    /**
     * @brief Проверить, является ли ответ ACK (использует текущий счетчик).
     * @param data Буфер ответа.
     * @param resultSize Размер ответа.
     * @return true если это ACK, иначе false.
     */
    bool IsAck(std::span<const uint8_t> data, int resultSize) const {
        return IsAck(data, resultSize, static_cast<uint8_t>(commandCounter_));
    }

    /**
     * @brief Проверить, является ли ответ NAK.
     * @param data Буфер ответа.
     * @param resultSize Размер ответа.
     * @param counter Счетчик команд.
     * @return true если это NAK, иначе false.
     */
    static bool IsNak(std::span<const uint8_t> data, int resultSize, uint8_t counter) {
        if (resultSize != 6) return false;
        uint8_t command[] = { 0x02, counter, 0x06, 0xFF, 0xBA, 0x8D };
        uint8_t tmp[6]; std::copy(std::begin(command), std::end(command), tmp);
        AddCrc(std::span(tmp, 6), 6);
        return std::equal(data.begin(), data.begin() + resultSize, tmp, tmp + 6);
    }

    /**
     * @brief Проверить, является ли ответ NAK (использует текущий счетчик).
     * @param data Буфер ответа.
     * @param resultSize Размер ответа.
     * @return true если это NAK, иначе false.
     */
    bool IsNak(std::span<const uint8_t> data, int resultSize) const {
        return IsNak(data, resultSize, static_cast<uint8_t>(commandCounter_));
    }

    /**
     * @brief Проверить, является ли ответ Illegal Command.
     * @param data Буфер ответа.
     * @param resultSize Размер ответа.
     * @param counter Счетчик команд.
     * @return true если это Illegal, иначе false.
     */
    static bool IsIllegal(std::span<const uint8_t> data, int resultSize, uint8_t counter) {
        // Подстройте под правильный код Illegal, у вас был RESET в примере
        if (resultSize != 6) return false;
        uint8_t command[] = { 0x02, counter, 0x06, 0x30, 0x41, 0xB3 }; // 
        uint8_t tmp[6]; std::copy(std::begin(command), std::end(command), tmp);
        AddCrc(std::span(tmp, 6), 6);
        return std::equal(data.begin(), data.begin() + resultSize, tmp, tmp + 6);
    }

    /**
     * @brief Проверить, является ли ответ Illegal Command (использует текущий счетчик).
     * @param data Буфер ответа.
     * @param resultSize Размер ответа.
     * @return true если это Illegal, иначе false.
     */
    bool IsIllegal(std::span<const uint8_t> data, int resultSize) const {
        return IsIllegal(data, resultSize, static_cast<uint8_t>(commandCounter_));
    }

private:
    template< class _InArrIter >
    std::string BinToAsciiString(const _InArrIter& _from, const _InArrIter& _to, bool bInsertSpaces = true, const std::string prefixString = std::string())
    {
        std::string strResult;
        strResult.reserve(std::distance(_from, _to) * (3 + prefixString.length()));

        _InArrIter it = _from;

        std::string strFmt = { bInsertSpaces ? prefixString + "%02X " : prefixString + "%02X" };

        std::string _psz;
        _psz.resize(strFmt.length() + strFmt.length() + 1);

        for (; it != _to; ++it) {
            std::fill(_psz.begin(), _psz.end(), 0);
            sprintf_s((char*)_psz.data(), _psz.length(), strFmt.c_str(), (unsigned char)*it);
            strResult += _psz.c_str();
        }

        // Удаляем лишний пробел
        if ((!strResult.empty()) && bInsertSpaces) {
            strResult.erase(strResult.size() - 1, 1);
        }

        return strResult;
    }

    std::string BinToAsciiString(const std::span< uint8_t >& arrHex, bool bInsertSpaces)
    {
        return BinToAsciiString(arrHex.cbegin(), arrHex.cend(), bInsertSpaces);
    }
    /**
     * @brief Низкоуровневая запись данных в порт.
     * @param data Указатель на данные.
     * @param n Количество байт.
     */
    void writeRaw(const uint8_t* data, size_t n);

    /**
     * @brief Цикл переподключения (фоновый поток).
     */
    void reconnectLoop();

    /**
     * @brief Получить активный io_context.
     * @return Ссылка на io_context.
     */
    boost::asio::io_context& active_io() {
        return ioOwned_ ? ioContextInternal_ : *externalIo_;
    }

    /**
     * @brief Вывести строку в консоль (утилита).
     * @tparam Args Аргументы для вывода.
     * @param args Аргументы.
     */
    template<typename... Args>
    static void println(Args&&... args) {
        ((std::cout << std::forward<Args>(args)), ...);
        std::cout << '\n';
    }

private:
    std::string m_portName; ///< Имя порта
    boost::asio::io_context* externalIo_ = nullptr; ///< Внешний io_context
    bool ioOwned_ = true; ///< Владеет ли объект своим io_context
    boost::asio::io_context ioContextInternal_; ///< Внутренний io_context
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workGuard_; ///< Guard для io_context
    std::unique_ptr<boost::asio::serial_port> serialPort_; ///< Указатель на serial_port
    std::thread ioThread_; ///< Поток для io_context

    std::mutex mu_; ///< Мьютекс для serialPort_ и состояния порта
    std::mutex sendMu_; ///< Мьютекс для сериализации sendCommand

    std::atomic<bool> portValid_{ false }; ///< Флаг валидности порта
    std::optional<std::string> lastError_; ///< Последняя ошибка

    std::thread reconnectThread_; ///< Поток переподключения
    std::atomic<bool> stopReconnectFlag_{ true }; ///< Флаг остановки переподключения
    std::atomic<bool> reconnectEnabled_; ///< Флаг разрешения переподключения
    std::atomic<bool> stopFlag_{ false }; ///< Флаг остановки

    int retries_; ///< Количество попыток
    std::chrono::milliseconds readTimeout_; ///< Таймаут чтения
    std::chrono::milliseconds attemptDelay_; ///< Задержка между попытками

    unsigned int baudRate_ = 115200; ///< Скорость передачи
    unsigned int charSize_ = 8; ///< Размер символа
    boost::asio::serial_port_base::parity parity_{ boost::asio::serial_port_base::parity::none }; ///< Четность
    boost::asio::serial_port_base::stop_bits stopBits_{ boost::asio::serial_port_base::stop_bits::one }; ///< Стоп-биты

    std::atomic<uint8_t> commandCounter_; ///< Счетчик команд
    std::optional<std::string> lastErrorString_; ///< Последняя строка ошибки

    std::shared_ptr<ILogger> m_log;             /**< @brief Логгер (spdlog-обёртка). */

};

