#define _WIN32_WINNT 0x0601 // Windows 7 and above
#include "SearchDevices.h"
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <codecvt>
#include <regex>
#include <cstring>
#ifdef _WIN32
#include <guiddef.h> // ��� GUID_DEVINTERFACE_COMPORT
#include <devguid.h>
#include <windows.h>
#include <setupapi.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
#endif

using namespace std::literals;

// ���������� ������ findDevices
[[nodiscard]] std::vector<std::string> SearchDevices::findDevices(const std::vector<std::string>& supportedPIDs)   // Взял и переделал из проекта "CDSlipAdapter" SearchDevices( const std::vector< std::string >& supportedPIDs )
{
    std::vector<std::string> ports;
#ifdef _WIN32
    // Получаем список устройств COM-портов через GUID интерфейса
    GUID guidDevInterfaceComPort = { 0x86e0d1e0, 0x8089, 0x11d0, {0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73} };
    HDEVINFO hDevInfoSet = ::SetupDiGetClassDevsW(&guidDevInterfaceComPort, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE); // Получаем набор устройств, соответствующих GUID ↑
    if (hDevInfoSet != INVALID_HANDLE_VALUE)
    {
        SP_DEVINFO_DATA devInfo = { sizeof(SP_DEVINFO_DATA) }; // Структура для хранения информации об устройстве
        // Перебираем все устройства, соответствующие GUID
        for (DWORD nIndex = 0; ::SetupDiEnumDeviceInfo(hDevInfoSet, nIndex, &devInfo); ++nIndex)
        {
            // Открываем ключ реестра устройства
            HKEY hDeviceKey = ::SetupDiOpenDevRegKey(hDevInfoSet, &devInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE); // Получаем ключ реестра устройства
            if (hDeviceKey != INVALID_HANDLE_VALUE)
            {
                wchar_t portName[64] = { 0 };
                DWORD size = sizeof(portName);
                DWORD type = 0;
                // Получаем имя порта (например, "COM3")
                if (::RegQueryValueExW(hDeviceKey, L"PortName", nullptr, &type, reinterpret_cast<LPBYTE>(portName), &size) == ERROR_SUCCESS && type == REG_SZ) // Проверяем, что значение существует и имеет тип REG_SZ (название порта)
                {
                    std::string port = wstring_to_string(portName);
                    if (port.starts_with("COM"))
                    {
                        wchar_t hardwareId[1024] = { 0 };
                        size = sizeof(hardwareId);
                        // Получаем HardwareID устройства
                        if (::SetupDiGetDeviceRegistryPropertyW(hDevInfoSet, &devInfo, SPDRP_HARDWAREID, &type,
                            reinterpret_cast<PBYTE>(hardwareId), size, &size)) // Проверяем, что HardwareID успешно получен (USB\VID_1A86&PID_55D3&REV_0445 USB\VID_1A86& PID_55D3)
                        {
                            std::wstring wstrProperty(hardwareId);
                            std::string strProperty = wstring_to_string(wstrProperty);
                            std::ranges::transform(strProperty, strProperty.begin(), [](unsigned char c)
                                { return std::toupper(c); });

                            // Извлекаем VID и PID из HardwareID
                            size_t pos = strProperty.find("VID_");
                            std::string vid, pid;
                            if (pos != std::string::npos)
                            {
                                pos += 4;
                                size_t endPos = strProperty.find_first_not_of("0123456789ABCDEF", pos);
                                vid = (endPos == std::string::npos) ? strProperty.substr(pos) : strProperty.substr(pos, endPos - pos);
                                pos = strProperty.find("PID_", endPos);
                                if (pos != std::string::npos)
                                {
                                    pos += 4;
                                    endPos = strProperty.find_first_not_of("0123456789ABCDEF", pos);
                                    pid = (endPos == std::string::npos) ? strProperty.substr(pos) : strProperty.substr(pos, endPos - pos);
                                }
                            }

                            // Проверяем, подходит ли устройство по PID и VID
                            if ((supportedPIDs.empty() || std::ranges::find(supportedPIDs, pid) != supportedPIDs.end()) &&
                                (vid.empty() || vid == "1A86" || vid == "10C4"))
                            {
                                ports.emplace_back(port);
                                std::cout << "Found port (GUID method): " << port << " (VID=" << vid << ", PID=" << pid << ")\n";
                            }
                        }
                    }
                }
                ::RegCloseKey(hDeviceKey);
            }
        }
        ::SetupDiDestroyDeviceInfoList(hDevInfoSet);
    }
    else
    {
        std::cout << "Failed to get device info set (GUID method). Error: " << ::GetLastError() << "\n";
    }

    // Шаг 2: если не найдено ни одного порта, пробуем получить их из реестра SERIALCOMM
    // 
    //if (ports.empty())
    //{
    //	HKEY hKey;
    //	if (::RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    //	{
    //		std::cerr << "Successfully opened SERIALCOMM registry\n";
    //		wchar_t valueName[256];
    //		wchar_t portName[64];
    //		DWORD valueNameSize, portNameSize, type;
    //		DWORD index = 0;
    //		// Перебираем все значения в разделе SERIALCOMM
    //		while (true)
    //		{
    //			valueNameSize = sizeof(valueName) / sizeof(wchar_t);
    //			portNameSize = sizeof(portName);
    //			if (::RegEnumValueW(hKey, index++, valueName, &valueNameSize, nullptr, &type, reinterpret_cast<LPBYTE>(portName), &portNameSize) != ERROR_SUCCESS) // если не удалось получить следующее значение завершаем цикл
    //			{
    //				break;
    //			}
    //			if (type == REG_SZ) // Проверяем, что значение имеет тип REG_SZ (строка)
    //			{
    //				const std::string port = wstring_to_string(portName);
    //				// Добавляем порт, если он начинается с "COM" и ещё не был добавлен
    //				if (port.starts_with("COM") && std::ranges::find(ports, port) == ports.end())
    //				{

    //					FS365::HW::Dors::DorsHW device(
    //						port.c_str(), 921600, 8, boost::asio::serial_port_base::parity::none, boost::asio::serial_port_base::stop_bits::one, NULL, false
    //					);

    //					uint8_t _unused;
    //					FS365::HW::Dors::DorsHW::POLL_RES pollRes = device.Poll(_unused); // Опрос устройства для проверки доступности устр

    //					if (FS365::HW::Dors::DorsHW::POLL_RES::PortError != pollRes) {
    //						ports.emplace_back(port);
    //						std::cerr << "Found port (SERIALCOMM method): " << port << "\n";
    //						break; // Прерываем цикл, если успешно подтвердили устройство
    //					}


    //				}
    //			}
    //		}
    //		::RegCloseKey(hKey);
    //	}
    //	else
    //	{
    //		std::cerr << "Failed to open SERIALCOMM registry. Error: " << ::GetLastError() << "\n";
    //	}
    //}
#else
    // Linux/ARM: Проверка /dev/ttyDBA (По правиле, symlink)
    std::cerr << "Checking for " << SerialPortConfig::ttyDBA << "..." << std::endl;
    if (access(SerialPortConfig::ttyDBA, F_OK) == 0) {
        lin_search_device_with_name(ports, supportedPIDs, SerialPortConfig::ttyDBA);
    }
    else {
        std::cerr << SerialPortConfig::ttyDBA << " not found" << std::endl;
    }

    // Поиск всех ttyUSB* и ttyACM* устройств
    if (ports.empty()) {
        std::vector<std::string> tty_patterns = { "/dev/ttyUSB[0-9]+", "/dev/ttyACM[0-9]+" }; //
        for (const auto& pattern : tty_patterns) {
            for (const auto& entry : std::filesystem::directory_iterator("/dev")) {
                std::string dev_path = entry.path().string();
                if (std::regex_match(dev_path, std::regex(pattern))) {
                    lin_search_device_with_name(ports, supportedPIDs, dev_path);
                }
            }
        }
    }
#endif
    std::ranges::sort(ports);
    return ports;
}

// ���������� ��������������� �������
[[nodiscard]] std::string SearchDevices::wstring_to_string(const std::wstring& wstr)
{
#ifdef _WIN32
    if (wstr.empty())
        return {};
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size == 0)
        return {};
    std::string str(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, str.data(), size, nullptr, nullptr);
    return str;
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
#endif
}

#ifdef __linux__
void SearchDevices::lin_search_device_with_name(std::vector<std::string>& ports, const std::vector<std::string>& supportedPIDs, const std::string& device_path) {
    char resolved_path[PATH_MAX];
    if (realpath(device_path.c_str(), resolved_path) != nullptr) // получаем полный путь /sys/class/tty/наше устройство с помощью символической ссылки
    {
        std::string real_device = std::filesystem::path(resolved_path).filename().string(); // ttyACM0
        std::cerr << SerialPortConfig::ttyDBA << " resolves to " << real_device << std::endl;

        std::string device_path = std::string(SerialPortConfig::sysfs_serial) + real_device;
        std::string uevent_path = device_path + "/device/uevent"; // путь до метаданных ttyACM0
        if (access(uevent_path.c_str(), F_OK) != 0)
        {
            uevent_path = device_path + "/uevent";
        }

        std::ifstream uevent(uevent_path);
        if (uevent.is_open())
        {
            std::cerr << "Opened uevent file for " << real_device << std::endl;
            std::string line;
            bool is_usb = false;
            std::string pid;

            while (std::getline(uevent, line)) //читаем все метаданные по строкам
            {
                if (line.find("DRIVER=usb-serial") != std::string::npos || line.find("DRIVER=cdc_acm") != std::string::npos)
                {
                    is_usb = true;
                    std::cerr << "Found USB driver" << std::endl;
                }
                else if (line.find("PRODUCT=") != std::string::npos)
                {
                    auto product = line.substr(line.find('=') + 1);
                    auto tokens = std::vector<std::string>{ std::strtok(&product[0], "/"), std::strtok(nullptr, "/") };
                    if (tokens.size() > 1)
                    {
                        pid = tokens[1];
                        pid.erase(std::remove(pid.begin(), pid.end(), '\n'), pid.end());
                        std::transform(pid.begin(), pid.end(), pid.begin(), ::toupper);
                        std::cerr << "Found PID: " << pid << std::endl;
                    }
                }
            }
            uevent.close();

            if (is_usb && !pid.empty() && (supportedPIDs.empty() || std::find(supportedPIDs.begin(), supportedPIDs.end(), pid) != supportedPIDs.end()))
            {
                ports.push_back(SerialPortConfig::ttyDBA); // Добавляем /dev/ttyDBA
            }
            else
            {
                std::cerr << "Device " << SerialPortConfig::ttyDBA << " does not match criteria (is_usb: " << is_usb << ", pid: " << pid << ")" << std::endl;
            }
        }
        else
        {
            std::cerr << "Failed to open uevent file: " << uevent_path << std::endl;
        }
    }
    else
    {
        std::cerr << "Failed to resolve " << SerialPortConfig::ttyDBA << ": " << strerror(errno) << std::endl;
    }
}
#endif
