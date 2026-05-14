#ifndef SEARCH_DEVICES_H
#define SEARCH_DEVICES_H

#include <vector>
#include <string>

struct SerialPortConfig {   // думаю здесть должны быть персистентные данные 
    static constexpr const char* sysfs_serial = "/sys/class/tty/"; // подкаталоги для каждого TTY-устройства (meta datas)
    static constexpr const char* dev_prefix = "/dev/"; // файловая система, где находятся файлы устройств, физические и виртуальные устройства
    static constexpr const char* ttyDBA = "/dev/ttyDBA"; // symlink
};

// ����� ��� ������ ���������
class SearchDevices {
public:
    [[nodiscard]] static std::vector<std::string> findDevices(const std::vector<std::string>& supportedPIDs = { "55D3", "EA60" });

private:
    [[nodiscard]] static std::string wstring_to_string(const std::wstring& wstr);

#ifdef __linux__
    // Поиск по названию конкретного устройства
    static void lin_search_device_with_name(std::vector<std::string>& ports, const std::vector<std::string>& supportedPIDs, const std::string& device_path);
#endif
};

#endif // SEARCH_DEVICES_H
