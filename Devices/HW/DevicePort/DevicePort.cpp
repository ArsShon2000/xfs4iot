#include "DevicePort.hpp"
#include "../../../framework/core/SettingModule/SettingModule.h"
//#include "XFSCommon/Utilities/AsciiHexConversions/AsciiHexConversions.h"


// Конструктор: можно передать внешний io_context или использовать внутренний (по умолчанию)
DevicePort::DevicePort(std::string_view port_name,
    std::shared_ptr<ILogger> logger,
    boost::asio::io_context* external_io)
    : m_portName(port_name),
    externalIo_(external_io),
    ioOwned_(external_io == nullptr),
    ioContextInternal_(),
    workGuard_(boost::asio::make_work_guard(ioOwned_ ? ioContextInternal_ : *external_io)),
    serialPort_(nullptr),
    reconnectEnabled_(true),
    retries_(4),
    readTimeout_(std::chrono::milliseconds(800)),
    attemptDelay_(std::chrono::milliseconds(100)),
    commandCounter_(0),
    m_log(logger)
{
    if (ioOwned_) {
        ioThread_ = std::thread([this]() { ioContextInternal_.run(); });
    }
    if (!ioOwned_) {
        // already have external io_context; workGuard_ used with it in ctor above
    }
}

DevicePort::~DevicePort() {
    stopReconnectMonitor();
    close();
    workGuard_.reset();
    if (ioOwned_) {
        ioContextInternal_.stop();
        if (ioThread_.joinable()) ioThread_.join();
    }
}

// Открыть порт (throws boost::system::system_error при фейле)
bool DevicePort::open() {
    std::lock_guard lock(mu_);
    try {
        if (!serialPort_) {
            boost::asio::io_context& io = ioOwned_ ? ioContextInternal_ : *externalIo_;
            serialPort_ = std::make_unique<boost::asio::serial_port>(io);
        }
        if (serialPort_->is_open()) serialPort_->close();
        serialPort_->open(m_portName);
        serialPort_->set_option(boost::asio::serial_port_base::baud_rate(baudRate_));
        serialPort_->set_option(boost::asio::serial_port_base::character_size(charSize_));
        serialPort_->set_option(parity_);
        serialPort_->set_option(stopBits_);
        serialPort_->set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
        lastError_.reset();
        portValid_.store(true);
        SettingModule::GetInstance()->setPort(m_portName);
        return true;
    }
    catch (const std::exception& e) {
        lastError_ = e.what();
        portValid_.store(false);
        return false;
    }
}

void DevicePort::close() {
    std::lock_guard lock(mu_);
    try {
        if (serialPort_ && serialPort_->is_open()) {
            boost::system::error_code ec;
            serialPort_->cancel(ec);
            serialPort_->close(ec);
        }
    }
    catch (...) {}
    portValid_.store(false);
}

// Запуск фонового мониторинга переподключения (если порт потерян) — опционально
void DevicePort::startReconnectMonitor() {
    if (reconnectEnabled_ && !reconnectThread_.joinable()) {
        stopReconnectFlag_.store(false);
        reconnectThread_ = std::thread([this]() { reconnectLoop(); });
    }
}

void DevicePort::stopReconnectMonitor() {
    stopReconnectFlag_.store(true);
    if (reconnectThread_.joinable()) reconnectThread_.join();
}

// Основной публичный метод. Блокирует вызывающий поток до завершения
DevicePort::RESULT DevicePort::sendCommand(std::span<uint8_t> command,
    int commandSize,
    std::span<uint8_t> result,
    size_t& resultSize,
    std::chrono::milliseconds timeout,
    bool sendAck)
{
    std::unique_lock lock(sendMu_);

    if (!portValid_.load()) {
        if (!open()) return RESULT::Err_Port;
    }

    RESULT finalRes = RESULT::Err_Crc;

    for (int attempt = 0; attempt < retries_; ++attempt) {
        // update counter in your command format (same as original)
        if (commandSize >= 2) {
            command[1] = static_cast<uint8_t>((commandCounter_ < 0x0F) ? ++commandCounter_ : (commandCounter_ = 0));
        }

        if (command[3] != 0x33)
        {
            m_log->trace(std::format("{}() Counter: {}", __FUNCTION__, std::to_string(commandCounter_)), LOGLEVEL_TRANSMIT);
        }

        AddCrc(command, commandSize);

        // write synchronously
        boost::system::error_code ec;
        size_t bytesWritten = 0;
        {
            std::lock_guard lockPort(mu_);
            if (!serialPort_ || !serialPort_->is_open()) {
                portValid_.store(false);
                finalRes = RESULT::Err_Port;
                break;
            }
            bytesWritten = boost::asio::write(*serialPort_, boost::asio::buffer(command.data(), static_cast<size_t>(commandSize)), ec);
        }
        if (ec || bytesWritten != static_cast<size_t>(commandSize)) {
            // write error
            if (command[3] != 0x33)
            {
                m_log->warn(std::format("{}() Не удалось отправить команду в устройcтво. Ошибка - {}", __FUNCTION__, ec.message()), WARNLEVEL_TRANSMIT);
            }
            portValid_.store(false);
            finalRes = RESULT::Err_Port;
            // small delay before retry
            std::this_thread::sleep_for(attemptDelay_);
            continue;
        }
        if (command[3] != 0x33)
        {
            m_log->trace(std::format("{}() Байты успешно были отправлены в устройтсво: длина = {}; data = [ {} ]", __FUNCTION__, bytesWritten, BinToAsciiString(command.subspan(0, bytesWritten), true).c_str()), LOGLEVEL_TRANSMIT);
        }


        // === read with timeout ===
        std::vector<uint8_t> readBuf(1024);
        resultSize = 0;
        bool gotData = false;
        boost::asio::steady_timer timer(active_io(), timeout);
        boost::system::error_code read_ec;
        std::mutex readMutex;
        std::condition_variable readCv;

        // start async read_some
        {
            std::lock_guard lockPort(mu_);
            if (!serialPort_ || !serialPort_->is_open()) {
                portValid_.store(false);
                finalRes = RESULT::Err_Port;
                break;
            }
            serialPort_->async_read_some(boost::asio::buffer(readBuf),
                [&](const boost::system::error_code& ec_read, std::size_t bytes) {
                    std::unique_lock lk(readMutex);
                    read_ec = ec_read;
                    if (!ec_read && bytes > 0) {
                        resultSize = bytes;
                        gotData = true;
                    }
                    lk.unlock();
                    readCv.notify_one();
                }
            );
        }

        // timer handler to cancel read
        bool timerFired = false;
        timer.async_wait([&](const boost::system::error_code& tec) {
            std::unique_lock lk(readMutex);
            if (!gotData) {
                timerFired = true;
                // cancel pending operations on serial
                std::lock_guard lockPort(mu_);
                if (serialPort_) {
                    boost::system::error_code ec2;
                    serialPort_->cancel(ec2);
                }
            }
            lk.unlock();
            readCv.notify_one();
            });

        // run until read or timeout
        // If using external io_context, we cannot run it here. Instead, rely on io_context running elsewhere.
        // For safety, if internal io owned, we are already running it. We'll wait on condition variable.
        {
            std::unique_lock lk(readMutex);
            if (!readCv.wait_for(lk, timeout + std::chrono::milliseconds(50), [&] { return gotData || timerFired || read_ec; })) {
                // timeout waiting
            }
        }

        // cancel timer
        boost::system::error_code ec_cancel;
        auto timerCancelRes = timer.cancel();


        if (!gotData || resultSize == 0) {
            m_log->warn(std::format("{}() Данные не получены в течение истечения времени ожидания.", __FUNCTION__), WARNLEVEL_TRANSMIT);
            finalRes = RESULT::Err_Port;
            // try again (or reconnect)
            std::this_thread::sleep_for(attemptDelay_);
            continue;
        }


        // copy to result buffer
        if (resultSize > result.size()) {
            if (command[3] != 0x33)
            {
                m_log->warn(std::format("{}() Буфер результатов слишком мал: {} > {}", __FUNCTION__, resultSize, result.size()), WARNLEVEL_TRANSMIT);
            }
            finalRes = RESULT::Err_Port;
            continue;
        }
        std::ranges::copy(readBuf.begin(), readBuf.begin() + static_cast<std::ptrdiff_t>(resultSize), result.begin());
        if (command[3] != 0x33)
        {
            m_log->trace(std::format("{}() Байты успешно были получены из устройтсва: длина = {}; data = [ {} ]", __FUNCTION__, resultSize, BinToAsciiString(result.subspan(0, resultSize), true).c_str()), LOGLEVEL_TRANSMIT);
        }

        // check CRC and protocol
        if (resultSize >= 6) {
            auto crc = GetCrc(result.subspan(0, static_cast<int>(resultSize) - 2));
            uint16_t recCrc = static_cast<uint16_t>(result[resultSize - 2] | (result[resultSize - 1] << 8));
            if (crc != recCrc) {
                if (command[3] != 0x33)
                {
                    m_log->warn(std::format("{}() CRC error.", __FUNCTION__), WARNLEVEL_TRANSMIT);
                }
                SendNak();
                finalRes = RESULT::Err_Crc;
                std::this_thread::sleep_for(attemptDelay_);
                continue;
            }
            if (IsNak(result, static_cast<int>(resultSize))) {
                if (command[3] != 0x33)
                {
                    m_log->warn(std::format("{}() Received NAK.", __FUNCTION__), WARNLEVEL_TRANSMIT);
                }
                finalRes = RESULT::Err_Nak;
                std::this_thread::sleep_for(attemptDelay_);
                continue;
            }
            if (IsIllegal(result, static_cast<int>(resultSize))) {
                if (command[3] != 0x33)
                {
                    m_log->warn(std::format("{}() Illegal command.", __FUNCTION__), WARNLEVEL_TRANSMIT);
                }
                finalRes = RESULT::Err_Illegal;
                break;
            }
            if (sendAck) SendAck();
            finalRes = RESULT::Ok;
            std::this_thread::sleep_for(attemptDelay_);
            break;
        }
        else {
            if (command[3] != 0x33)
            {
                m_log->warn(std::format("{}() Invalid response length.", __FUNCTION__), WARNLEVEL_TRANSMIT);
            }
            finalRes = RESULT::Err_Port;
            continue;
        }
    } // attempts loop

    if (finalRes != RESULT::Ok && !portValid_.load()) {
        // try to trigger reconnect monitor
        if (reconnectEnabled_) startReconnectMonitor();
    }
    return finalRes;
}

// Нижний уровень записи
void DevicePort::writeRaw(const uint8_t* data, size_t n) {
    std::lock_guard lock(mu_);
    if (!serialPort_ || !serialPort_->is_open()) return;
    boost::system::error_code ec;
    boost::asio::write(*serialPort_, boost::asio::buffer(data, n), ec);
    if (ec) {
        m_log->warn(std::format("{}() WriteRaw error: {}", __FUNCTION__, ec.message()), WARNLEVEL_TRANSMIT);
        portValid_.store(false);
    }
}

// reconnect loop (в фоновом потоке)
void DevicePort::reconnectLoop() {
    while (!stopReconnectFlag_.load()) {
        if (!portValid_.load()) {
            try {
                close();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                if (open()) {
                    m_log->warn(std::format("{}() Reconnected to port  {}", __FUNCTION__, m_portName), WARNLEVEL_TRANSMIT);
                }
                else {
                    // failed, wait a bit
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
            catch (...) {}
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
