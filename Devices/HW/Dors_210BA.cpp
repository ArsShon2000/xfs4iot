#include "Dors_210BA.h"
#include <format> 
#include "./SearchDevices/SearchDevices.h"
#include "../../framework/core/SettingModule/SettingModule.h"

// #pragma warning(disable : 4482)

namespace FS365::HW::Dors {

    std::mutex DorsHW::m_SendCommand_mutex;

    DorsHW::DorsHW(
        const std::string& portName,
        uint32_t baudRate,
        uint8_t byteSize,
        boost::asio::serial_port_base::parity::type parity,
        boost::asio::serial_port_base::stop_bits::type stopBits,
        std::shared_ptr<ILogger> logger
    ) : m_port(std::make_unique<DevicePort>(portName, logger))
        , m_baudRate(baudRate)
        , m_byteSize(byteSize)
        , m_parity(parity)
        , m_stopBits(stopBits)
        , m_lastState(POLL_RES::Unknown)
        , m_strFirmware("DEFAULT")
        , m_log(logger)
        , m_bSoftwareConfigurationFault(false)
    {

        
        std::vector<std::string>  ports;
        bool isMultiDeviceMode = SettingModule::GetInstance()->getMultiDeviceMode();
        if (portName.empty())
        {
            if (isMultiDeviceMode)
            {
                m_bPortIsInitialized = false;
                m_log->warn(std::format("{}() Ошибка инициализации устройства. Режим MultiDeviceMode включён, имя порта не указано. Поиск устройств не будет выполняться", __FUNCTION__));
                return;
            }
            ports = SearchDevices::findDevices();
        }
        else
        {
            if (CheckingTheDevice(portName))
            {
                return; // Устройство успешно инициализировано, выходим из конструктора
            }
            else
            {
                if (isMultiDeviceMode)
                {
                    m_bPortIsInitialized = false;
                    m_log->warn(std::format("{}() Ошибка инициализации устройства. Режим MultiDeviceMode включён, имя порта не указано. Поиск устройств не будет выполняться", __FUNCTION__));
                    return;
                }
                ports = SearchDevices::findDevices();
            }
        }

        if (ports.empty()) {
            m_bPortIsInitialized = false;
            m_log->warn(std::format("{}() Ошибка инициализации устройства", __FUNCTION__));
            return;
        }

        for (auto port : ports)
        {
            CheckingTheDevice(port);
        }
    }

    DorsHW::~DorsHW() {
        StopExecutor();
        if (m_port && m_port->isOpen()) {
            m_port->stopReconnectMonitor();
            m_port->close();
        }
    }


    bool DorsHW::_initialize_port() {
        try {
            m_port->setBaudRate(m_baudRate);
            m_port->setCharacterSize(m_byteSize);
            m_port->setParity(m_parity);
            m_port->setStopBits(m_stopBits);
            if (!m_port->open()) {
                m_bPortIsInitialized = false;
                return m_bPortIsInitialized;
            }
            m_port->startReconnectMonitor();
            m_bPortIsInitialized = true;
        }
        catch (const boost::system::system_error& e) {
            m_log->warn(std::format("{}() Ошибка инициализации порта {}", __FUNCTION__, e.what()));
            m_bPortIsInitialized = false;
        }
        return m_bPortIsInitialized;
    }

    bool DorsHW::CheckingTheDevice(const std::string& port)
    {
        m_log->trace(std::format("{}() - Инициализация порта {}", __FUNCTION__, port));
        m_port->setPortName(port);
        if (_initialize_port()) {
            m_log->trace(std::format("{}() - Открытие порта успешно", __FUNCTION__));
            StartExecutor();

            m_log->trace(std::format("{}() - Порт открыт, выполняем сброс устройства", __FUNCTION__));

            if (RESULT::Ok == Reset()) {
                CIdentification idn;
                if (RESULT::Ok == Identification(idn)) {
                    m_strFirmware = idn.strPartNumber; // Получаем номер прошивки из идентификационных данных
                    m_log->trace(std::format("{}() - Номер прошивки: {}", __FUNCTION__, m_strFirmware));
                    m_bSoftwareConfigurationFault = true;
                    return true; // Успешно инициализировано, выходим из цикла
                }
            }
        }
        else {
            m_log->warn(std::format("{}() - Не удалось открыть порт {}", __FUNCTION__, port));
            return false;
        }
    }

    uint16_t DorsHW::GetCrc(std::span<const uint8_t> data) {
        uint16_t crc = 0;
        for (auto byte : data) {
            crc ^= byte;
            for (int j = 0; j < 8; ++j) {
                if (crc & 0x0001) {
                    crc >>= 1;
                    crc ^= 0x08408;
                }
                else {
                    crc >>= 1;
                }
            }
        }
        return crc;
    }

    void DorsHW::AddCrc(std::span<uint8_t> data, int commandSize) {
        if (commandSize < 2) return;
        auto crc = GetCrc(data.subspan(0, commandSize - 2));
        data[commandSize - 2] = static_cast<uint8_t>(crc & 0xFF);
        data[commandSize - 1] = static_cast<uint8_t>(crc >> 8);
    }

    void DorsHW::SendAck() {
        //uint8_t command[] = { 0x02, 0x03, 0x06, 0x00, 0xC2, 0x82 };
        //if (m_port->write_some(boost::asio::buffer(command, sizeof(command)))) {
        //    //println("SendAck: ACK отправлен");
        //}
        //else {
        //    println("Ошибка отправки ACK");
        //}
    }

    void DorsHW::SendNak() {
        //uint8_t command[] = { 0x02, 0x03, 0x06, 0xFF, 0xBA, 0x8D };
        //if (m_port->write_some(boost::asio::buffer(command, sizeof(command)))) {
        //    //println("SendAck: NAK отправлен");
        //}
        //else {
        //    println("Ошибка отправки NAK");
        //}
    }

    void DorsHW::ComPortReadToNull(std::chrono::milliseconds timeout) {  // Чтение данных из порта до истечения таймаута (можно удалять)
       /* auto startTime = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - startTime < timeout) {
            std::vector<uint8_t> buffer(256);
            boost::system::error_code ec;
            auto bytesRead = m_port->read_some(boost::asio::buffer(buffer), ec);
            if (ec) {
                println("Read error: ", ec.message());
                break;
            }
            if (bytesRead > 0) {
                println("Read ", bytesRead, " bytes in garbage");
            }
        }*/
    }

    bool DorsHW::IsDeviceInitialized() {
        return m_bPortIsInitialized;
    }

    bool DorsHW::IsAck(std::span<const uint8_t> data, int resultSize) {
        if (resultSize != 6) return false;
        uint8_t command[] = { 0x02, m_ucCommandCounter, 0x06, 0x00, 0xC2, 0x82 };
        AddCrc(std::span(command, 6), 6);
        return std::equal(data.begin(), data.begin() + resultSize, std::begin(command), std::begin(command) + 6);
    }

    bool DorsHW::IsNak(std::span<const uint8_t> data, int resultSize) {
        if (resultSize != 6) return false;
        uint8_t command[] = { 0x02, m_ucCommandCounter, 0x06, 0xFF, 0xBA, 0x8D };
        AddCrc(std::span(command, 6), 6);
        return std::equal(data.begin(), data.begin() + resultSize, std::begin(command), std::begin(command) + 6);
    }

    bool DorsHW::IsIllegal(std::span<const uint8_t> data, int resultSize) { // Проверка на Illegal Command нуждается в доработке
        if (resultSize != 6) return false;
        uint8_t command[] = { 0x02, m_ucCommandCounter, 0x06, static_cast<uint8_t>(DEV_CMD::RESET), 0x41, 0xB3 };
        AddCrc(std::span(command, 6), 6);
        return std::equal(data.begin(), data.begin() + resultSize, std::begin(command), std::begin(command) + 6);
    }

    DorsHW::RESULT DorsHW::SendCommand(
        std::span<uint8_t> command,
        int commandSize,
        std::span<uint8_t> result,
        size_t& resultSize,
        std::chrono::milliseconds timeout,
        bool sendAck) {
        std::lock_guard<std::mutex> lock(m_SendCommand_mutex);

        if (!m_bPortIsInitialized) {
            if (!_initialize_port()) {
                return RESULT::Err_Port;
            }
        }

        RESULT res = RESULT::Err_Crc;
        for (int retries = 4; retries > 0 && (res == RESULT::Err_Crc || res == RESULT::Err_Nak || res == RESULT::Err_Port); --retries) {
            // Увеличиваем счетчик команд
            if (m_ucCommandCounter < 0x0F) {
                ++m_ucCommandCounter;
            }
            else {
                m_ucCommandCounter = 0;
            }
            command[1] = m_ucCommandCounter;
            AddCrc(command, commandSize);

            // Отправка команды
            boost::system::error_code ec;
       /*     auto bytesWritten = m_port->write_some(boost::asio::buffer(command, commandSize), ec);
            if (ec || bytesWritten != static_cast<size_t>(commandSize)) {
                println("Write error: ", ec.message());
                res = RESULT::Err_Port;
                continue;
            }*/

            // Подготовка к чтению ответа
            resultSize = 0;
            std::vector<uint8_t> response(256);
            auto startTime = std::chrono::steady_clock::now();
            boost::asio::steady_timer timer(*m_ioContext, timeout);
            bool dataReceived = false;

            // Лямбда для асинхронного чтения
            auto readHandler = [&](const boost::system::error_code& read_ec, size_t bytes) {
                if (!read_ec && bytes > 0) {
                    resultSize = bytes;
                    dataReceived = true;
                    timer.cancel(); // Прерываем таймер, так как данные получены
                }
                else if (read_ec && read_ec != boost::asio::error::operation_aborted) {
                    m_log->warn(std::format("{}() Read error: {}", __FUNCTION__, read_ec.message()));
                }
                };

            // Лямбда для обработки таймаута
            //auto timeoutHandler = [&](const boost::system::error_code& timer_ec) {
            //    if (!timer_ec && !dataReceived) {
            //        println("Read timeout");
            //        m_port->cancel(); // Отменяем операцию чтения
            //    }
            //    };

            // Запускаем асинхронное чтение в цикле
            /*while (!dataReceived && std::chrono::steady_clock::now() - startTime < timeout) {
                m_port->async_read_some(
                    boost::asio::buffer(response),
                    readHandler
                );
                timer.async_wait(timeoutHandler);
                m_ioContext->run();
                m_ioContext->restart();
            }*/

            // Проверка результата чтения
            if (!dataReceived || resultSize == 0) {
                m_log->trace(std::format("{}() No data received within timeout", __FUNCTION__));
                res = RESULT::Err_Port;
                continue;
            }

            // Копируем данные в выходной буфер
            if (resultSize > result.size()) {
                m_log->trace(std::format("{}() Result buffer too small: {} > {}", __FUNCTION__, resultSize,  result.size()));
                res = RESULT::Err_Port;
                continue;
            }
            std::ranges::copy(response.begin(), response.begin() + resultSize, result.begin());

            // Проверка ответа
            if (resultSize >= 6) {
                auto crc = GetCrc(result.subspan(0, resultSize - 2));
                if (crc != (result[resultSize - 2] | (result[resultSize - 1] << 8))) {
                    m_log->warn(std::format("{}() CRC error", __FUNCTION__));
                    SendNak();
                    res = RESULT::Err_Crc;
                    std::this_thread::sleep_for(FREE);
                    continue;
                }
                if (IsNak(result, resultSize)) {
                    m_log->warn(std::format("{}() Received NAK: Err_Nak", __FUNCTION__));
                    res = RESULT::Err_Nak;
                    std::this_thread::sleep_for(FREE);
                    continue;
                }
                if (IsIllegal(result, resultSize)) {
                    m_log->warn(std::format("{}() Illegal command: Err_Illegal", __FUNCTION__));
                    res = RESULT::Err_Illegal;
                    break;
                }
                if (sendAck) {
                    SendAck();
                }
                res = RESULT::Ok;
                std::this_thread::sleep_for(FREE);
            }
            else {
                m_log->warn(std::format("{}() Invalid response length: {}", __FUNCTION__, resultSize));
                res = RESULT::Err_Port;
            }
        }

        return res;
    }

    DorsHW::POLL_RES DorsHW::Poll(uint8_t& additionalRes) {
        std::vector<uint8_t> hwServiceBytes; // Создаём локальный вектор
        return Poll(additionalRes, hwServiceBytes);
    }

    DorsHW::POLL_RES DorsHW::Poll(uint8_t& additionalRes, std::vector<uint8_t>& hwServiceBytes) {
        std::lock_guard<std::mutex> lock(m_portMutex);
        hwServiceBytes.clear();
        std::promise<RESULT> resultPromise;
        auto resultFuture = resultPromise.get_future();

        uint8_t command[] = { 0x02, m_ucCommandCounter, 0x06, static_cast<uint8_t>(DEV_CMD::POLL), 0x00, 0x00 };
        uint8_t result_array[256];
        std::span<uint8_t> resultSpan(result_array);
        POLL_RES pollRes = POLL_RES::Unknown;
        size_t resultSize = 0;

        auto task = Task{
        std::span(command),
        static_cast<int>(sizeof(command)),
        resultSpan,
        &resultSize,
        RESPONSE_TIME,
        true,
        [&resultPromise](RESULT res) {
            resultPromise.set_value(res);
        },
        m_nextTaskId++ // taskId будет перезаписан
        };
        EnqueueCommand(task);

        auto res = resultFuture.get();

        if (res == RESULT::Ok) {
            if (resultSize < 6) {
                return POLL_RES::Unknown;
            }

            if (resultSpan[3] == static_cast<uint8_t>(POLL_RES::Send_States_Stack)) {
                int byteCount = resultSpan[2] - 4 - 1 - 2;
                int startByte = 0;
                int stateCount = resultSpan[4];
                int stateCurrent = 0;

                while (byteCount > startByte) {
                    if ((byteCount - startByte) < 5) {
                        m_log->warn(std::format("{}() Poll: Parse error in extended state", __FUNCTION__));
                        startByte = byteCount;
                    }
                    else if ((resultSpan[4 + startByte + 1] >= 0xD0 && resultSpan[4 + startByte + 1] <= 0xDD) ||
                        (stateCount == stateCurrent)) {
                        hwServiceBytes.assign(resultSpan.begin() + 4 + startByte + 1, resultSpan.end() - 3);
                        startByte = byteCount;
                    }
                    else if (resultSpan[4 + startByte + 1] >= 0xA0 && resultSpan[4 + startByte + 1] <= 0xCF) {
                        uint8_t respLen = 1;
                        /*uint32_t time = (resultSpan[4 + startByte + respLen + 1] << 24) |
                            (resultSpan[4 + startByte + respLen + 2] << 16) |
                            (resultSpan[4 + startByte + respLen + 3] << 8) |
                            resultSpan[4 + startByte + respLen + 4];*/
                        startByte += respLen + 4;
                    }
                    else {
                        uint8_t respLen;
                        pollRes = Poll_State(
                            resultSpan[4 + startByte + 1],
                            resultSpan[4 + startByte + 2],
                            resultSpan[4 + startByte + 3],
                            respLen,
                            additionalRes
                        );
                        if (pollRes == POLL_RES::Unknown) {
                            m_log->warn(std::format("{}() Poll: Parse error - garbage data", __FUNCTION__));
                            startByte = byteCount;
                        }
                        /*uint32_t time = (resultSpan[4 + startByte + respLen + 1] << 24) |
                            (resultSpan[4 + startByte + respLen + 2] << 16) |
                            (resultSpan[4 + startByte + respLen + 3] << 8) |
                            resultSpan[4 + startByte + respLen + 4];*/
                        stateCurrent++;
                        startByte += respLen + 4;
                    }
                }

            }
            else {
                uint8_t respLen;
                pollRes = Poll_State(resultSpan[3], resultSpan[4], resultSpan[5], respLen, additionalRes);
                if (resultSpan.size() > static_cast<size_t> (5 + _getPollDataLength(pollRes))) {
                    hwServiceBytes.assign(resultSpan.begin() + 3 + _getPollDataLength(pollRes), resultSpan.end() - 2);
                }
            }
            return pollRes;
        }

        switch (res) {
        case RESULT::Err_Port: return POLL_RES::PortError;
        case RESULT::Err_Crc: return POLL_RES::CrcError;
        case RESULT::Err_Illegal: return POLL_RES::IllegalCommand;
        case RESULT::Err_Nak: return POLL_RES::Nak;
        default: return POLL_RES::Unknown;
        }

    }

    DorsHW::POLL_RES DorsHW::Poll_State(uint8_t firstByte, uint8_t secondByte, uint8_t thirdByte, uint8_t& Response_Len, uint8_t& AdditionalRes)
    {
        POLL_RES eResult = POLL_RES::Unknown;
        AdditionalRes = 0xFD;
        Response_Len = 1;

        switch (firstByte) {
        case 0x33:
            eResult = POLL_RES::PortError;
            Response_Len = 1;
            break;

        case 0x1B:
        case static_cast<uint8_t>(POLL_RES::EscrowPos):
        case static_cast<uint8_t>(POLL_RES::BillStacked):
        case static_cast<uint8_t>(POLL_RES::BillReturned):
            AdditionalRes = secondByte;
            eResult = static_cast<POLL_RES>(firstByte);
            Response_Len = 2;
            break;

        case 0x47:
            eResult = static_cast<POLL_RES>((static_cast<uint16_t>(secondByte) << 8) | static_cast<uint16_t>(firstByte));
            Response_Len = 2;
            switch (eResult) {
            case POLL_RES::StackMotorFail:
            case POLL_RES::TransportMotorFail:
            case POLL_RES::InitialCassetteStatusFail:
            case POLL_RES::OpticCanalFail:
            case POLL_RES::MagneticCanalFail:
            case POLL_RES::StartTrayFailure:
                break;
            default:
                static DWORD dwLastUnknownStateNo = 0;
                if (dwLastUnknownStateNo != static_cast<DWORD>(eResult)) {
                    dwLastUnknownStateNo = static_cast<DWORD>(eResult);
                    m_log->trace(std::format("{}() Poll: Unknown group 47 state: 0x{} ", __FUNCTION__, static_cast<int>(eResult)));
                }
                eResult = POLL_RES::Group47UnknownFailure;
                break;
            }
            break;

        case static_cast<uint8_t>(POLL_RES::Rejecting):
            eResult = static_cast<POLL_RES>((static_cast<uint16_t>(secondByte) << 8) | static_cast<uint16_t>(firstByte));
            Response_Len = 2;

            switch (eResult) {
            case POLL_RES::Rejecting:
            case POLL_RES::RejInsertion:
            case POLL_RES::RejMagnetic:
            case POLL_RES::RejBillInHead:
            case POLL_RES::RejMultiplying:
            case POLL_RES::RejConveying:
            case POLL_RES::RejIdentification:
            case POLL_RES::RejNotebase:
            case POLL_RES::RejIAS:
            case POLL_RES::RejInhibit:
            case POLL_RES::RejDensity:
            case POLL_RES::RejOperation:
            case POLL_RES::RejLength:
            case POLL_RES::RejUV:
            case POLL_RES::RejTape:
            case POLL_RES::RejEntryCassetteSns:
            case POLL_RES::RejFastConveying:
            case POLL_RES::RejTrayClosed:
            case POLL_RES::RejTimeout:
            case POLL_RES::RejFastFeed:
                break;
            default:
                static DWORD dwLastRejectUnknownReason = 0;
                if (dwLastRejectUnknownReason != static_cast<DWORD>(eResult)) {
                    dwLastRejectUnknownReason = static_cast<DWORD>(eResult);
                    m_log->trace(std::format("{}() Poll: Unknown reject reason (group 0x1C): 0x{} ", __FUNCTION__, static_cast<int>(eResult)));
                }
                eResult = POLL_RES::RejUnknown;
                break;
            }

            if (eResult == POLL_RES::RejInhibit) {
                AdditionalRes = thirdByte;
                Response_Len = 3;
            }
            else {
                AdditionalRes = 0xFE;
                Response_Len = 2;
            }
            break;

        case static_cast<uint8_t>(POLL_RES::Undefined):
            Response_Len = 1;
            m_log->trace(std::format("{}() Poll: DBA: Command not defined in firmware", __FUNCTION__));
            break;

        default:
            eResult = static_cast<POLL_RES>(firstByte);
            Response_Len = 1;
            break;
        }

        return eResult;
    }

    DorsHW::RESULT DorsHW::Reset() {
        std::lock_guard<std::mutex> lock(m_portMutex);
        m_log->trace(std::format("{}() Выполняется функция ",  __FUNCTION__));;
        std::promise<RESULT> resultPromise;
        auto resultFuture = resultPromise.get_future();

        uint8_t command[] = { 0x02, m_ucCommandCounter, 0x06, static_cast<uint8_t>(DEV_CMD::RESET), 0x00, 0x00 };
        uint8_t result_array[256];
        std::span<uint8_t> resultSpan(result_array);
        size_t resultSize = 0;

        //auto res = SendCommand(std::span(command), sizeof(command), resultSpan, resultSize);

        auto task = Task{
        std::span(command),
        static_cast<int>(sizeof(command)),
        resultSpan,
        &resultSize,
        RESPONSE_TIME,
        true,
        [&resultPromise](RESULT res) {
            resultPromise.set_value(res);
        },
        m_nextTaskId++ // taskId будет перезаписан
        };
        EnqueueCommand(task);

        auto res = resultFuture.get();

        if (res == RESULT::Ok) {
            if (resultSize < 6) {
                m_log->warn(std::format("{}() Reset: Invalid response length: {}", __FUNCTION__, resultSize));
                return RESULT::Err_Unknown;
            }
            if (!m_port->IsAck(resultSpan, resultSize)) {
                m_log->warn(std::format("{}() Reset: ACK not received", __FUNCTION__));
                return RESULT::Err_Ack;
            }
        }
        return res;
    }

    DorsHW::RESULT DorsHW::GetStatus(uint32_t& billEnabled, uint32_t& security) {
        std::lock_guard<std::mutex> lock(m_portMutex);
        std::promise<RESULT> resultPromise;
        auto resultFuture = resultPromise.get_future();

        uint8_t command[] = { 0x02, m_ucCommandCounter, 0x06, static_cast<uint8_t>(DEV_CMD::GET_STATUS), 0x00, 0x00 };
        uint8_t result_array[256];
        std::span<uint8_t> resultSpan(result_array);
        size_t resultSize = 0;

        auto task = Task{
        std::span(command),
        static_cast<int>(sizeof(command)),
        resultSpan,
        &resultSize,
        RESPONSE_TIME,
        true,
        [&resultPromise](RESULT res) {
            resultPromise.set_value(res);
        },
        m_nextTaskId++ // taskId будет перезаписан
        };
        EnqueueCommand(task);

        auto res = resultFuture.get();

        if (res == RESULT::Ok) {
            if (resultSize < 11) {
                m_log->warn(std::format("{}() GetStatus: Invalid response length: {}", __FUNCTION__, resultSize));
                return RESULT::Err_Unknown;
            }
            billEnabled = (resultSpan[3] << 24) | (resultSpan[4] << 16) | (resultSpan[5] << 8);
            security = (resultSpan[6] << 24) | (resultSpan[7] << 16) | (resultSpan[8] << 8);
        }
        return res;
    }

    DorsHW::RESULT DorsHW::EnableBillTypes(uint32_t billTypes, uint32_t billsWithEscrow) {
        std::lock_guard<std::mutex> lock(m_portMutex);
        println( __FUNCTION__, " ", billTypes, " ", billsWithEscrow);
        m_log->trace(std::format("{}() Выполняется функция. billTypes: {}; billsWithEscrow: {}", __FUNCTION__, billTypes, billsWithEscrow));
        std::promise<RESULT> resultPromise;
        auto resultFuture = resultPromise.get_future();

        uint8_t command[] = { 0x02, m_ucCommandCounter, 0x0C, static_cast<uint8_t>(DEV_CMD::ENABLE_BILL_TYPES), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        command[4] = (billTypes >> 16) & 0xFF;
        command[5] = (billTypes >> 8) & 0xFF;
        command[6] = billTypes & 0xFF;
        command[7] = (billsWithEscrow >> 16) & 0xFF;
        command[8] = (billsWithEscrow >> 8) & 0xFF;
        command[9] = billsWithEscrow & 0xFF;
        uint8_t result_array[256];
        std::span<uint8_t> resultSpan(result_array);
        size_t resultSize = 0;

        auto task = Task{
        std::span(command),
        static_cast<int>(sizeof(command)),
        resultSpan,
        &resultSize,
        RESPONSE_TIME,
        false,
        [&resultPromise](RESULT res) {
            resultPromise.set_value(res);
        },
        m_nextTaskId++ // taskId будет перезаписан
        };
        EnqueueCommand(task);

        auto res = resultFuture.get();

        if (res == RESULT::Ok) {
            if (resultSize < 6) {
                m_log->warn(std::format("{}() EnableBillTypes: Invalid response length", __FUNCTION__));
                return RESULT::Err_Unknown;
            }
            if (!m_port->IsAck(resultSpan, resultSize)) {
                m_log->warn(std::format("{}() EnableBillTypes: ACK not received", __FUNCTION__));
                return RESULT::Err_Ack;
            }
        }
        return res;
    }

    DorsHW::RESULT DorsHW::Stack() {
        std::lock_guard<std::mutex> lock(m_portMutex);
        m_log->trace(std::format("{}() Выполняется функция ",  __FUNCTION__));;
        std::promise<RESULT> resultPromise;
        auto resultFuture = resultPromise.get_future();

        uint8_t command[] = { 0x02, m_ucCommandCounter, 0x06, static_cast<uint8_t>(DEV_CMD::STACK), 0x00, 0x00 };
        uint8_t result_array[256];
        std::span<uint8_t> resultSpan(result_array);
        size_t resultSize = 0;

        auto task = Task{
        std::span(command),
        static_cast<int>(sizeof(command)),
        resultSpan,
        &resultSize,
        RESPONSE_TIME,
        false,
        [&resultPromise](RESULT res) {
            resultPromise.set_value(res);
        },
        m_nextTaskId++ // taskId будет перезаписан
        };
        EnqueueCommand(task);

        auto res = resultFuture.get();

        if (res == RESULT::Ok) {
            if (resultSize < 6) {
                m_log->warn(std::format("{}() Stack: Invalid response length: ", __FUNCTION__, resultSize));
                return RESULT::Err_Unknown;
            }
            if (!m_port->IsAck(resultSpan, resultSize)) {
                m_log->warn(std::format("{}() Stack: ACK not received", __FUNCTION__));
                return RESULT::Err_Ack;
            }
        }
        return res;
    }

    DorsHW::RESULT DorsHW::Return() {
        std::lock_guard<std::mutex> lock(m_portMutex);
        m_log->trace(std::format("{}() Выполняется функция ",  __FUNCTION__));;
        std::promise<RESULT> resultPromise;
        auto resultFuture = resultPromise.get_future();

        uint8_t command[] = { 0x02, m_ucCommandCounter, 0x06, static_cast<uint8_t>(DEV_CMD::RETURN), 0x00, 0x00 };
        uint8_t result_array[256];
        std::span<uint8_t> resultSpan(result_array);
        size_t resultSize = 0;

        auto task = Task{
        std::span(command),
        static_cast<int>(sizeof(command)),
        resultSpan,
        &resultSize,
        RESPONSE_TIME,
        true,
        [&resultPromise](RESULT res) {
            resultPromise.set_value(res);
        },
        m_nextTaskId++ // taskId будет перезаписан
        };
        EnqueueCommand(task);

        auto res = resultFuture.get();

        if (res == RESULT::Ok) {
            if (resultSize < 6) {
                m_log->warn(std::format("{}() Return: Invalid response length: " , __FUNCTION__, resultSize));
                return RESULT::Err_Unknown;
            }
            if (!m_port->IsAck(resultSpan, resultSize)) {
                m_log->warn(std::format("{}() Return: ACK not received", __FUNCTION__));
                return RESULT::Err_Ack;
            }
        }
        return res;
    }

    DorsHW::RESULT DorsHW::Hold()
    {
        std::lock_guard<std::mutex> lock(m_portMutex);
        m_log->trace(std::format("{}() Выполняется функция ",  __FUNCTION__));;
        std::promise<RESULT> resultPromise;
        auto resultFuture = resultPromise.get_future();

        uint8_t command[] = { 0x02, m_ucCommandCounter, 0x06, static_cast<uint8_t>(DEV_CMD::HOLD), 0x00, 0x00 };
        uint8_t result_array[256];
        std::span<uint8_t> resultSpan(result_array);
        size_t resultSize = 0;

        auto task = Task{
       std::span(command),
       static_cast<int>(sizeof(command)),
       resultSpan,
       &resultSize,
       RESPONSE_TIME,
       false,
       [&resultPromise](RESULT res) {
           resultPromise.set_value(res);
       },
       m_nextTaskId++ // taskId будет перезаписан
        };
        EnqueueCommand(task);

        auto res = resultFuture.get();

        if (res == RESULT::Ok) {
            if (resultSize < 6) {
                m_log->warn(std::format("{}() Hold: Invalid response length: ", __FUNCTION__, resultSize));
                return RESULT::Err_Unknown;
            }
            if (!m_port->IsAck(resultSpan, resultSize)) {
                m_log->warn(std::format("{}() Hold: ACK not received", __FUNCTION__));
                return RESULT::Err_Ack;
            }
        }
        return res;
    }

    DorsHW::RESULT DorsHW::Identification(CDbaIdentification& dbaIdn) {
        std::lock_guard<std::mutex> lock(m_portMutex);
        m_log->trace(std::format("{}() Выполняется функция ",  __FUNCTION__));
        std::promise<RESULT> resultPromise;
        auto resultFuture = resultPromise.get_future();

        uint8_t command[] = { 0x02, m_ucCommandCounter, 0x06, static_cast<uint8_t>(DEV_CMD::IDENTIFICATION), 0x00, 0x00 };
        uint8_t result_array[256];
        std::span<uint8_t> resultSpan(result_array);
        size_t resultSize = 0;

        auto task = Task{
            std::span(command),
            static_cast<int>(sizeof(command)),
            resultSpan,
            &resultSize,
            RESPONSE_TIME,
            true,
            [&resultPromise](RESULT res) {
                resultPromise.set_value(res);
            },
            m_nextTaskId++ // taskId будет перезаписан
        };
        EnqueueCommand(task);

        auto res = resultFuture.get();

        if (res == RESULT::Ok) {
            if (resultSize < 47) {
                m_log->warn(std::format("{}() Неверная длина {} ответа команды", __FUNCTION__, resultSize));
                return RESULT::Err_Unknown;
            }

            dbaIdn = CDbaIdentification(resultSpan.subspan(3));
            m_log->warn(std::format("{}() - {}" , __FUNCTION__, dbaIdn.Print()));
        }

        m_log->warn(std::format("{}()  finished with result: {}",__FUNCTION__, ResultToString(res)));
        return res;
    }

    DorsHW::RESULT DorsHW::Identification(std::string& strPartNumber, std::string& strSerialNumber) {
        CIdentification idn;
        auto res = Identification(idn);
        if (res == RESULT::Ok) {
            strPartNumber = idn.strPartNumber;
            strSerialNumber = idn.strSerialNumber;
        }
        return res;
    }

    DorsHW::RESULT DorsHW::Identification(CIdentification& idn) {
        std::lock_guard<std::mutex> lock(m_portMutex);
        m_log->trace(std::format("{}() Выполняется функция ",  __FUNCTION__));
        std::promise<RESULT> resultPromise;
        auto resultFuture = resultPromise.get_future();


        uint8_t command[] = { 0x02, m_ucCommandCounter, 0x06, static_cast<uint8_t>(DEV_CMD::IDENTIFICATION), 0x00, 0x00 };
        uint8_t result_array[256];
        std::span<uint8_t> resultSpan(result_array);
        size_t resultSize = 0;

        auto task = Task{
            std::span(command),
            static_cast<int>(sizeof(command)),
            resultSpan,
            &resultSize,
            RESPONSE_TIME,
            true,
            [&resultPromise](RESULT res) {
                resultPromise.set_value(res);
            },
            m_nextTaskId++ // taskId будет перезаписан
        };
        EnqueueCommand(task);       

        auto res = resultFuture.get();

        if (res == RESULT::Ok) {
            if (resultSize < 39) {
                m_log->warn(std::format("{}() : Неверная длина {} ответа команды" ,__FUNCTION__, resultSize));
                return RESULT::Err_Unknown;
            }
            else if (resultSize >= 47) {
                // D210BA
                idn = CIdentification(CDbaIdentification(resultSpan.subspan(3)));
            }
            m_log->warn(std::format("{}() {}" ,__FUNCTION__, idn.Print()));
        }


        m_log->warn(std::format("{}()  finished with result : {}",__FUNCTION__, ResultToString(res)));
        return res;
    }

    DorsHW::RESULT DorsHW::GetBillTable(std::span<uint8_t, 120> bills) {
        std::lock_guard<std::mutex> lock(m_portMutex);
        std::promise<RESULT> resultPromise;
        auto resultFuture = resultPromise.get_future();
        uint8_t command[] = { 0x02, m_ucCommandCounter, 0x06, static_cast<uint8_t>(DEV_CMD::GET_BILL_TABLE), 0x00, 0x00 };
        uint8_t result_array[256];
        std::span<uint8_t> resultSpan(result_array);
        size_t resultSize = 0;

        auto task = Task{
            std::span(command),
            static_cast<int>(sizeof(command)),
            resultSpan,
            &resultSize,
            RESPONSE_TIME,
            true,
            [&resultPromise](RESULT res) {
                resultPromise.set_value(res);
            },
            m_nextTaskId++ // taskId будет перезаписан
        };
        EnqueueCommand(task);

        auto res = resultFuture.get();

        if (res == RESULT::Ok) {
            if (resultSize < 124) {
                m_log->warn(std::format("{}() : GetBillTable: Invalid response length: {}", __FUNCTION__, resultSize));
                return RESULT::Err_Unknown;
            }
            std::copy(resultSpan.begin() + 3, resultSpan.begin() + 123, bills.begin());
        }
        return res;
    }


    DorsHW::RESULT DorsHW::Reboot() {
        std::lock_guard<std::mutex> lock(m_portMutex);

        m_log->trace(std::format("{}() Выполняется функция ",  __FUNCTION__));;
        std::promise<RESULT> resultPromise;
        auto resultFuture = resultPromise.get_future();

        uint8_t command[] = { 0x02, m_ucCommandCounter, 0x06, static_cast<uint8_t>(DEV_CMD::REBOOT), 0x00, 0x00 };
        uint8_t result_array[256];
        std::span<uint8_t> resultSpan(result_array);
        size_t resultSize = 0;

        auto task = Task{
            std::span(command),
            static_cast<int>(sizeof(command)),
            resultSpan,
            &resultSize,
            RESPONSE_TIME,
            false,
            [&resultPromise](RESULT res) {
                resultPromise.set_value(res);
            },
            m_nextTaskId++ // taskId будет перезаписан
        };
        EnqueueCommand(task);

        auto res = resultFuture.get();

        if (res == RESULT::Ok) {
            if (resultSize >= 6) {
                if (IsAck(resultSpan, resultSize)) {
                    // Перезагрузка длится около 15 сек.
                    std::this_thread::sleep_for(std::chrono::seconds(15));
                }
                else {
                    res = RESULT::Err_Ack;
                }
            }
            else {
                res = RESULT::Err_Unknown;
            }
        }

        m_log->warn(std::format("{}() - finished with result: {}", __FUNCTION__, ResultToString(res)));
        return res;
    }

    DorsHW::RESULT DorsHW::cassette_control(bool enable) {
        std::lock_guard<std::mutex> lock(m_portMutex);

        m_log->trace(std::format("{}() Выполняется функция ",  __FUNCTION__));;
        std::promise<RESULT> resultPromise;
        auto resultFuture = resultPromise.get_future();

        uint8_t b_enable = enable ? 0x01 : 0x00;
        uint8_t command[] = { 0x02, m_ucCommandCounter, 0x07, static_cast<uint8_t>(DEV_CMD::CASSETTE_CONTROL), b_enable, 0x00, 0x00 };
        uint8_t result_array[256];
        std::span<uint8_t> resultSpan(result_array);
        size_t resultSize = 0;

        auto task = Task{
            std::span(command),
            static_cast<int>(sizeof(command)),
            resultSpan,
            &resultSize,
            RESPONSE_TIME,
            true,
            [&resultPromise](RESULT res) {
                resultPromise.set_value(res);
            },
            m_nextTaskId++ // taskId будет перезаписан
        };
        EnqueueCommand(task);

        auto res = resultFuture.get();

        if (res == RESULT::Ok) {
            if (resultSize < 6) {
                m_log->warn(std::format("{}() - Неверная длина ({}) ответа команды", __FUNCTION__, resultSize));
                return RESULT::Err_Unknown;
            }
            if (!m_port->IsAck(resultSpan, resultSize)) {
                m_log->warn(std::format("{}() - ACK не получен", __FUNCTION__ ));
                return RESULT::Err_Ack;
            }
        }

        m_log->warn(std::format("{}() - finished with result: {}", __FUNCTION__, ResultToString(res)));
        return res;
    }

    DorsHW::RESULT DorsHW::CassetteHighLevel(bool bHigh)
    {
        std::lock_guard<std::mutex> lock(m_portMutex);
        std::promise<RESULT> resultPromise;
        auto resultFuture = resultPromise.get_future();

        uint8_t command[] = { 0x02, m_ucCommandCounter, 0x07, static_cast<uint8_t>(DEV_CMD::CASSETTE_HIGH_LEVEL), 0x00, 0x00, 0x00 };
        command[4] = bHigh ? 0x01 : 0x00;
        uint8_t result_array[256];
        std::span<uint8_t> resultSpan(result_array);
        size_t resultSize = 0;

        auto task = Task{
           std::span(command),
           static_cast<int>(sizeof(command)),
           resultSpan,
           &resultSize,
           RESPONSE_TIME,
           false,
           [&resultPromise](RESULT res) {
               resultPromise.set_value(res);
           },
           m_nextTaskId++ // taskId будет перезаписан
        };
        EnqueueCommand(task);

        auto res = resultFuture.get();

        return res;
    }

    DorsHW::RESULT DorsHW::states_stack_transfer_enable(bool enable) {
        std::lock_guard<std::mutex> lock(m_portMutex);

        m_log->trace(std::format("{}() Выполняется функция ",  __FUNCTION__));
        std::promise<RESULT> resultPromise;
        auto resultFuture = resultPromise.get_future();

        uint8_t b_enable = enable ? 0x01 : 0x00;
        uint8_t command[] = { 0x02, m_ucCommandCounter, 0x07, static_cast<uint8_t>(DEV_CMD::STATES_STACK_TRANSFER_ENABLE), b_enable, 0x00, 0x00 };
        uint8_t result_array[256];
        std::span<uint8_t> resultSpan(result_array);
        size_t resultSize = 0;

        auto task = Task{
            std::span(command),
            static_cast<int>(sizeof(command)),
            resultSpan,
            &resultSize,
            RESPONSE_TIME,
            true,
            [&resultPromise](RESULT res) {
                resultPromise.set_value(res);
            },
            m_nextTaskId++ // taskId будет перезаписан
        };
        EnqueueCommand(task);

        auto res = resultFuture.get();

        if (res == RESULT::Ok) {
            if (resultSize < 6) {
                m_log->warn(std::format("{}() - Неверная длина ({}) ответа команды", __FUNCTION__, resultSize));
                return RESULT::Err_Unknown;
            }
            if (!m_port->IsAck(resultSpan, resultSize)) {
                m_log->warn(std::format("{}() - ACK не получен", __FUNCTION__));
                return RESULT::Err_Ack;
            }
        }

        m_log->warn(std::format("{}() - finished with result: {}", __FUNCTION__, ResultToString(res)));
        return res;
    }

    void DorsHW::EnqueueCommand(Task& task) {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_taskQueue.push(task);
        tempVarIsCurReq = true;
        //println("\t Количество команд в очереди : ", m_taskQueue.size());
        m_queueCv.notify_all();
    }

    void DorsHW::ExecutorThread() {
        while (m_running) {
            //if (!m_taskQueue.empty()) {
            //if (!m_bResetOperationInProgress)
            //{
            Task task;
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_queueCv.wait(lock, [this] { return !m_running || !m_taskQueue.empty(); });
                if (!m_running && m_taskQueue.empty()) break;
                if (m_taskQueue.empty()) continue;
                task = m_taskQueue.front();
                m_taskQueue.pop();
            }
            // Выполнение команды
            RESULT res = (RESULT) m_port->sendCommand(task.command, task.commandSize, task.result, *task.resultSize, task.timeout, task.sendAck);

            //if (task.command[3] == static_cast<uint8_t>(DEV_CMD::RESET)) {
            //    m_bResetOperationInProgress = true; // Сброс операции сброса
            // Запускаем таймер в отдельном потоке. Иммитиреум, что ресет операции выполняется 20 секунд
           /* std::jthread timerThread([this] {
                std::this_thread::sleep_for(std::chrono::milliseconds(20000));
                println("Timer THREAD: ", std::this_thread::get_id(), "\t Завершилась команда сброса");
                m_bResetOperationInProgress = false;
                });*/
                //}

                // Вызов callback для обработки результата
            if (task.callback) {
                task.callback(res);
                if (m_taskQueue.size() == 0) tempVarIsCurReq = false;
            }
            //}
            //}
        }
    }

    void DorsHW::StartExecutor() {
        if (!m_running) {
            m_running = true;
            m_executorThread = std::make_unique<std::jthread>([this]() {
                ExecutorThread();
                });

#ifdef _WIN32
            if (!::SetThreadPriority(m_executorThread->native_handle(), THREAD_PRIORITY_TIME_CRITICAL)) {
                unsigned long dwResult = ::GetLastError();
                m_log->warn(std::format("{}() - НЕВОЗМОЖНО ИЗМЕНИТЬ ПРИОРИТЕТ ПРОЦЕССА: {}", __FUNCTION__, dwResult));
                std::cout <<  __FUNCTION__ << "() - НЕВОЗМОЖНО ИЗМЕНИТЬ ПРИОРИТЕТ ПРОЦЕССА: " << dwResult;
            }
#else
            struct sched_param param {};
            param.sched_priority = sched_get_priority_max(SCHED_FIFO);
            if (pthread_setschedparam(m_executorThread.native_handle(), SCHED_FIFO, &param) != 0) {
                std::cout <<  __FUNCTION__ << "() - НЕВОЗМОЖНО УСТАНОВИТЬ ПРИОРИТЕТ ПОТОКА: " << errno;
                m_log->warn(std::format("{}() - НЕВОЗМОЖНО ИЗМЕНИТЬ ПРИОРИТЕТ ПРОЦЕССА: {}", __FUNCTION__, dwResult));
            }
#endif
        }
    }

    void DorsHW::StopExecutor() {
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_log->trace(std::format("{}() - Останавливаем поток для обработки запроса", __FUNCTION__));
            m_running = false;
            m_queueCv.notify_all();
        }
    }


    bool DorsHW::_isRejCode(POLL_RES state) {
        switch (state) {
        case POLL_RES::Rejecting:
        case POLL_RES::RejInsertion:
        case POLL_RES::RejMagnetic:
        case POLL_RES::RejBillInHead:
        case POLL_RES::RejMultiplying:
        case POLL_RES::RejConveying:
        case POLL_RES::RejIdentification:
        case POLL_RES::RejNotebase:
        case POLL_RES::RejIAS:
        case POLL_RES::RejInhibit:
        case POLL_RES::RejDensity:
        case POLL_RES::RejOperation:
        case POLL_RES::RejLength:
        case POLL_RES::RejUV:
        case POLL_RES::RejTape:
        case POLL_RES::RejEntryCassetteSns:
        case POLL_RES::RejFastConveying:
        case POLL_RES::RejTrayClosed:
        case POLL_RES::RejTimeout:
        case POLL_RES::RejFastFeed:
        case POLL_RES::RejUnknown:
        case POLL_RES::RejTrayLengthShort:
            return true;
        default:
            return false;
        }
    }

    bool DorsHW::_isInoperative(POLL_RES state) {
        switch (state) {
        using enum DorsHW::POLL_RES; // C++20 — не нужно писать DorsHW::
        case DropCassetteFull:
        case DropCassetteOutOfPosition:
        case ValidatorJammed:
        case DropCassetteJammed:
        case StackMotorFail:
        case TransportMotorFail:
        case InitialCassetteStatusFail:
        case OpticCanalFail:
        case MagneticCanalFail:
        case Group47UnknownFailure:
        case StartTrayFailure:
        case PortError:
            return true;
        default:
            return false;
        }
    }

    bool DorsHW::_isHWError(POLL_RES state) {
        switch (state) {
        case POLL_RES::ValidatorJammed:
        case POLL_RES::DropCassetteJammed:
        case POLL_RES::StackMotorFail:
        case POLL_RES::TransportMotorFail:
        case POLL_RES::InitialCassetteStatusFail:
        case POLL_RES::OpticCanalFail:
        case POLL_RES::MagneticCanalFail:
        case POLL_RES::StartTrayFailure:
        case POLL_RES::Group47UnknownFailure:
        case POLL_RES::Cheated:
        case POLL_RES::FishingDetected:
            return true;
        default:
            return false;
        }
    }

    bool DorsHW::_isOnline(POLL_RES state)
    {
        switch (state)
        {
        case POLL_RES::PortError:
        case POLL_RES::CrcError:
        case POLL_RES::Unknown:
        case POLL_RES::IllegalCommand:
        case POLL_RES::Nak:
            return false;
        }
        return true;
    }

    bool DorsHW::_isPowerUp(POLL_RES state)
    {
        switch (state)
        {
        case POLL_RES::PowerUp:
        case POLL_RES::PowerUpWithBillInValidator:
        case POLL_RES::PowerUpWithBillInStacker:
            return true;
        }
        return false;
    }

    bool DorsHW::_isNoteDetected(POLL_RES state)
    {
        switch (state)
        {
        case POLL_RES::PowerUpWithBillInValidator:
        case POLL_RES::PowerUpWithBillInStacker:
        case POLL_RES::Accepting:
        case POLL_RES::Stacking:
        case POLL_RES::Returning:
        case POLL_RES::Holding:
        case POLL_RES::Rejecting:	// Новое состояние по протоколу об. дан. DORS 210BA
        case POLL_RES::RejInsertion:
        case POLL_RES::RejMagnetic:
        case POLL_RES::RejBillInHead:
        case POLL_RES::RejMultiplying:
        case POLL_RES::RejConveying:
        case POLL_RES::RejIdentification:
        case POLL_RES::RejNotebase: 	// Новое название по протоколу об. дан. DORS 210BA
        case POLL_RES::RejIAS:	// Новое название по протоколу об. дан. DORS 210BA
        case POLL_RES::RejInhibit:
        case POLL_RES::RejDensity:	// Новое название по протоколу об. дан. DORS 210BA
        case POLL_RES::RejOperation:
        case POLL_RES::RejLength:
        case POLL_RES::RejUV: 	// Новое название по протоколу об. дан. DORS 210BA
        case POLL_RES::RejUnknown:
        case POLL_RES::ValidatorJammed:
        case POLL_RES::DropCassetteJammed:
        case POLL_RES::Pause:
        case POLL_RES::EscrowPos:
        case POLL_RES::BillReturned:
        case POLL_RES::RejTape:
        case POLL_RES::RejEntryCassetteSns:
        case POLL_RES::RejFastConveying:
        case POLL_RES::RejTrayClosed:
        case POLL_RES::RejTimeout:
        case POLL_RES::RejFastFeed:
            return true;
        }
        return false;
    }

    int DorsHW::_getPollDataLength(POLL_RES state)
    {
        switch (state)
        {
        case POLL_RES::Rejecting:
        case POLL_RES::RejInhibit:
        case POLL_RES::RejInsertion:
        case POLL_RES::RejMagnetic:
        case POLL_RES::RejBillInHead:
        case POLL_RES::RejMultiplying:
        case POLL_RES::RejConveying:
        case POLL_RES::RejIdentification:
        case POLL_RES::RejNotebase:
        case POLL_RES::RejIAS:
        case POLL_RES::RejDensity:
        case POLL_RES::RejOperation:
        case POLL_RES::RejLength:
        case POLL_RES::RejUV:
        case POLL_RES::RejTape:
        case POLL_RES::RejEntryCassetteSns:
        case POLL_RES::RejFastConveying:
        case POLL_RES::RejTrayClosed:
        case POLL_RES::RejTimeout:
        case POLL_RES::RejFastFeed:
        case POLL_RES::RejTrayLengthShort:
        case POLL_RES::RejUnknown:
            return 3;
        case POLL_RES::StackMotorFail:
        case POLL_RES::TransportMotorFail:
        case POLL_RES::InitialCassetteStatusFail:
        case POLL_RES::OpticCanalFail:
        case POLL_RES::MagneticCanalFail:
        case POLL_RES::Group47UnknownFailure:
        case POLL_RES::EscrowPos:
        case POLL_RES::BillStacked:
        case POLL_RES::BillReturned:
        case POLL_RES::StartTrayFailure:
        case POLL_RES::PowerNoiseFailure:
            return 2;
        case POLL_RES::PortError:
        case POLL_RES::CrcError:
        case POLL_RES::Unknown:
        case POLL_RES::IllegalCommand:
        case POLL_RES::Nak:
            return 0;
        default:
            return 1;
        }
    }

    const std::set< DorsHW::POLL_RES >& DorsHW::GetRejectStates(void)
    {
        static std::set< DorsHW::POLL_RES > g_result{
            POLL_RES::Rejecting,	// Новое состояние по протоколу об. дан. DORS 210BA
            POLL_RES::RejInsertion,
            POLL_RES::RejMagnetic,
            POLL_RES::RejBillInHead,
            POLL_RES::RejMultiplying,
            POLL_RES::RejIdentification,
            POLL_RES::RejConveying,
            POLL_RES::RejNotebase, 	// Новое название по протоколу об. дан. DORS 210BA
            POLL_RES::RejIAS,	// Новое название по протоколу об. дан. DORS 210BA
            POLL_RES::RejInhibit,
            POLL_RES::RejDensity,	// Новое название по протоколу об. дан. DORS 210BA
            POLL_RES::RejOperation,
            POLL_RES::RejLength,
            POLL_RES::RejUV, 	// Новое название по протоколу об. дан. DORS 210BA
            POLL_RES::RejTape,
            POLL_RES::RejEntryCassetteSns,
            POLL_RES::RejFastConveying,
            POLL_RES::RejTrayClosed,
            POLL_RES::RejTimeout,
            POLL_RES::RejFastFeed,
            POLL_RES::RejUnknown,
            POLL_RES::RejTrayLengthShort
        };

        return g_result;
    }

    const std::set<DorsHW::POLL_RES>& DorsHW::GetErrorStates()
    {
        static std::set< DorsHW::POLL_RES > err_result{
            POLL_RES::DropCassetteJammed,	
            POLL_RES::ValidatorJammed,
            POLL_RES::DropCassetteOutOfPosition,
            POLL_RES::StackMotorFail,
            POLL_RES::TransportMotorFail,
            POLL_RES::InitialCassetteStatusFail,
            POLL_RES::OpticCanalFail,
            POLL_RES::MagneticCanalFail, 	
            POLL_RES::Group47UnknownFailure,	
            POLL_RES::StartTrayFailure,
            POLL_RES::PortError
        };

        return err_result;
    }

}
