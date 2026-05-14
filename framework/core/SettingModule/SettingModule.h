#pragma once
#include <mutex>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>
#include <sstream>
#include <cstdint>
#include <filesystem>
#include "../Logger/ILogger.hpp"
#include <regex>


static const std::vector<int>& allowedLogLevels()
{
    static const std::vector<int> levels = {
        LOGLEVEL0,
        LOGLEVELEVENT,
        LOGLEVELPERSISTENT,
        LOGLEVEL1,
        LOGLEVELMSG,
        SPILEVEL,
        LOGLEVEL_TRANSMIT,
        WARNLEVEL_TRANSMIT,
        LOGLEVEL_COMMPORT_PROBLEMS
    };
    return levels;
}


/**
 * @brief Стратегии поведения при включении устройства с банкнотой в приемнике
 */
enum PowerUpWithBillStrategy : uint16_t {
    /** Остановить приём */
    STOP_ACCEPT = 0,
    /** Разрешить приём */
    ALLOW_ACCEPT = 1,
    /** Разрешить приём и отправлять в накопитель */
    ALLOW_ACCEPT_BILL_STACKER = 2
};

inline std::string toString(PowerUpWithBillStrategy strategy) {
    switch (strategy) {
        case STOP_ACCEPT: return "STOP_ACCEPT";
        case ALLOW_ACCEPT: return "ALLOW_ACCEPT";
        case ALLOW_ACCEPT_BILL_STACKER: return "ALLOW_ACCEPT_BILL_STACKER";
        default: return "UNKNOWN";
    }
}


constexpr uint16_t MAX_CASSETTE_HIGH_LEVEL = 100;
constexpr uint16_t MIN_CASSETTE_HIGH_LEVEL = 50;
constexpr uint16_t DEFAULT_CASSETTE_HIGH_LEVEL = 80;
constexpr const char* BASIC_XFS_VERSION = "3.40";
constexpr const char* APP_VERSION = "3.40.24";
/**
 * @brief Модуль настроек приложения (singleton)
 *
 * Класс отвечает за чтение/запись JSON-файла настроек,
 * предоставляет методы доступа по точечной нотации ("section.key")
 * и хранит некоторые проинициализированные параметры в виде полей.
 */
class SettingModule
{
    /** Исключение модуля настроек */
    class SettingModuleException : public std::runtime_error {
    public:
        /**
         * @param message Сообщение об ошибке
         */
        SettingModuleException(const std::string& message) : std::runtime_error(message) {}
    };

public:

    struct LogLevelRange {
        int lo{};
        int hi{}; // inclusive
    };

    /** Конструктор по умолчанию */
    SettingModule() = default;
    SettingModule(const SettingModule&) = delete;
    SettingModule& operator=(const SettingModule&) = delete;

    /**
     * @brief Уничтожение singleton-экземпляра
     *
     * Потокобезопасно освобождает единственный экземпляр класса.
     */
    static void DestroyInstance() {
        // Защищаем уничтожение экземпляра мьютексом
        std::lock_guard<std::mutex> lock(m_instanceMutex);
        delete pinstance_;
        pinstance_ = nullptr;
    }

    /**
     * @brief Получить singleton-экземпляр SettingModule
     *
     * Если файл настроек не существует, может быть создан с указанным путем.
     *
     * @param filePath Путь к JSON-файлу настроек (по умолчанию "settings.json")
     * @return Указатель на экземпляр SettingModule
     */
    static SettingModule* GetInstance(const std::string& filePath = "settings.json");

    /**
* @brief Создать файл настроек при его отсутствии
*
* Если файл уже существует — операция пропускается.
*
* @param initialContent Начальное содержимое JSON (по умолчанию пустой объект)
*/
    void create(const nlohmann::json& initialContent = nlohmann::json::object());

    /**
     * @brief Прочитать весь JSON из файла
     * @return Объект nlohmann::json с содержимым файла
     * @throws SettingModuleException при ошибке чтения/парсинга
     */
    nlohmann::json read() const;

    /**
     * @brief Прочитать значение по пути в точечной нотации
     * @param path Путь, например: "section.key"
     * @return Значение в виде nlohmann::json
     */
    nlohmann::json readByKey(const std::string& path) const;

    /**
     * @brief Прочитать значение по пути или записать и вернуть значение по умолчанию
     *
     * Если по указанному пути значение отсутствует, создаётся элемент со значением
     * defaultData и сохраняется в файл.
     *
     * @tparam T Тип значения по умолчанию
     * @param path Путь в точечной нотации
     * @param defaultData Значение по умолчанию
     * @return Значение (существующее или вставленное по умолчанию)
     */
    template <typename T>
    nlohmann::json readByKeyWithDefaultValue(const std::string& path, const T& defaultData) {
        nlohmann::json content = read();

        // Разбиваем путь по точкам
        std::vector<std::string> keys;
        std::stringstream ss(path);
        std::string key;
        while (std::getline(ss, key, '.')) {
            keys.push_back(key);
        }

        nlohmann::json* current = &content;
        for (size_t i = 0; i < keys.size(); ++i) {
            const auto& k = keys[i];

            // Если ключ отсутствует и это не последний уровень — создаем пустой объект
            if (!current->contains(k)) {
                if (i < keys.size() - 1) {
                    (*current)[k] = nlohmann::json::object();
                }
                else {
                    // Последний уровень — создаем с defaultData
                    (*current)[k] = defaultData;
                    saveToFile(content);
                    return (*current)[k];
                }
            }

            current = &(*current)[k];
        }

        // Если значение уже существует — просто возвращаем
        return *current;
    }


    /**
     * @brief Обновить значение по указанному пути (создаёт промежуточные объекты при необходимости)
     * @param path Путь в точечной нотации
     * @param value Новое значение в формате nlohmann::json
     */
    void update(const std::string& path, const nlohmann::json& value);

    /**
     * @brief Удалить элемент по пути
     * @param path Путь в точечной нотации
     */
    void remove(const std::string& path);

    /**
     * @brief Инициализирует внутренние параметры из файла настроек
     *
     * Заполняет поля: m_cassetteCapacity, m_cassetteHighLevel, m_port,
     * m_powerUpWithBillStrategy, m_resetCassetteCounters, m_delayTimeAcceptanceStop.
     */
    void initializeParameters(std::shared_ptr<ILogger> pLogger) {
        setLogger(pLogger);
        nlohmann::json settings = read();
        if (!settings.is_null()) {
            // Инициализация ёмкости кассеты
            if (settings.contains("CASSETTE_CAPACITY") && settings["CASSETTE_CAPACITY"].is_number_unsigned()) {
                m_cassetteCapacity = settings["CASSETTE_CAPACITY"].get<uint32_t>();
                if (m_cassetteCapacity <= 0)
                {
                    m_cassetteCapacity = 1000;  // значение по умолчанию
                    setCassetteCapacity(m_cassetteCapacity);
                }
            }
            else {
                m_cassetteCapacity = 1000; // значение по умолчанию
                setCassetteCapacity(m_cassetteCapacity);
            }

            // Инициализация порога высокого уровня кассеты
            if (settings.contains("CASSETTE_HIGH_LEVEL") && settings["CASSETTE_HIGH_LEVEL"].is_number_unsigned()) {
                m_cassetteHighLevel = settings["CASSETTE_HIGH_LEVEL"].get<uint32_t>();
                if (m_cassetteHighLevel < MIN_CASSETTE_HIGH_LEVEL || MAX_CASSETTE_HIGH_LEVEL > 100) {
                    m_cassetteHighLevel = DEFAULT_CASSETTE_HIGH_LEVEL; // По умолчанию 80%
                    setCassetteHighLevel(m_cassetteHighLevel);
                }
            }
            else {
                m_cassetteHighLevel = DEFAULT_CASSETTE_HIGH_LEVEL; // По умолчанию 80%
                setCassetteHighLevel(m_cassetteHighLevel);
            }

            // COM порт
            if (settings.contains("PORT") && settings["PORT"].is_string()) {
                m_port = settings["PORT"].get<std::string>();
            }
            else {
                m_port = ""; // значение по умолчанию
                setPort(m_port);
            }

            // стратегия при включении с купюрой в приемнике
            if (settings.contains("POWER_UP_WITH_BILL_STRATEGY") && settings["POWER_UP_WITH_BILL_STRATEGY"].is_string()) {
                std::string strategy = settings["POWER_UP_WITH_BILL_STRATEGY"].get<std::string>();
                if (strategy == "STOP_ACCEPT") {
                    m_powerUpWithBillStrategy = PowerUpWithBillStrategy::STOP_ACCEPT;
                }
                else if (strategy == "ALLOW_ACCEPT") {
                    m_powerUpWithBillStrategy = PowerUpWithBillStrategy::ALLOW_ACCEPT;
                }
                else if (strategy == "ALLOW_ACCEPT_BILL_STACKER") {
                    m_powerUpWithBillStrategy = PowerUpWithBillStrategy::ALLOW_ACCEPT_BILL_STACKER;
                }
                else {
                    m_powerUpWithBillStrategy = PowerUpWithBillStrategy::ALLOW_ACCEPT; // значение по умолчанию
                }
            }
            else {
                m_powerUpWithBillStrategy = PowerUpWithBillStrategy::ALLOW_ACCEPT; // значение по умолчанию
                setPowerUpWithBillStrategy(PowerUpWithBillStrategy::ALLOW_ACCEPT);
            }

            // сброс статистики принятых банкнот при подаче WFS_CMD_CIM_END_EXCHANGE без параметров
            if (settings.contains("RESET_CASSETTE_COUNTERS") && settings["RESET_CASSETTE_COUNTERS"].is_number_integer()) {
                m_resetCassetteCounters = settings["RESET_CASSETTE_COUNTERS"].get<uint16_t>();
            }
            else {
                m_resetCassetteCounters = 0; // значение по умолчанию
                setResetCassetteCounters(m_resetCassetteCounters);
            }

            // время задержки (в мс) подачи команды завершения приема банкнот на устройство после отработки WFS_CMD_CIM_CASH_IN_END
            if (settings.contains("DELAY_TIME_ACCEPTANCE_STOP") && settings["DELAY_TIME_ACCEPTANCE_STOP"].is_number_unsigned()) {
                uint16_t delay = settings["DELAY_TIME_ACCEPTANCE_STOP"].get<uint16_t>();
                if (delay >= 0 && delay <= 15000) {
                    m_delayTimeAcceptanceStop = delay;
                }
                else {
                    m_delayTimeAcceptanceStop = 2000; // По умолчанию 2000 мс
                    setDelayTimeAcceptanceStop(m_delayTimeAcceptanceStop);
                }
            }
            else {
                m_delayTimeAcceptanceStop = 2000; // По умолчанию 2000 мс
                setDelayTimeAcceptanceStop(m_delayTimeAcceptanceStop);
            }

            // режим непрерывного приёма
            if (settings.contains("CONJOINT_CASH_IN") && settings["CONJOINT_CASH_IN"].is_number_unsigned()) {
                m_conjointCashInMode = settings["CONJOINT_CASH_IN"].get<uint32_t>();
            }
            else {
                m_conjointCashInMode = false; // По умолчанию false
                setConjointCashInMode(m_conjointCashInMode);
            }

            // режим Отключение манипуляции CU вне инкассации
            if (settings.contains("DISABLE_CU_MANIPULATED") && settings["DISABLE_CU_MANIPULATED"].is_number_unsigned()) {
                m_disableCuManipulated = settings["DISABLE_CU_MANIPULATED"].get<uint16_t>();
            }
            else {
                m_disableCuManipulated = 1; // значение по умолчанию
                setDisableCuManipulated(m_disableCuManipulated);
            }

            // режим Отключение манипуляции CU вне инкассации
            if (settings.contains("ARCHIVE_DAYS") && settings["ARCHIVE_DAYS"].is_number_unsigned()) {
                m_archiveDays = settings["ARCHIVE_DAYS"].get<uint16_t>();
            }
            else {
                m_archiveDays = 1; // значение по умолчанию
                setArchiveDays(m_archiveDays);
            }

            // режим Отключение манипуляции CU вне инкассации
            if (settings.contains("MAX_FOLDER_MB_FOR_LOGS") && settings["MAX_FOLDER_MB_FOR_LOGS"].is_number_unsigned()) {
                m_maxFolderMB = settings["MAX_FOLDER_MB_FOR_LOGS"].get<uint16_t>();
            }
            else {
                m_maxFolderMB = 1; // значение по умолчанию
                setMaxFolderMB(m_maxFolderMB);
            }

            // размер лог-файла в мб
            if (settings.contains("LOG_FILE_SIZE") && settings["LOG_FILE_SIZE"].is_number_unsigned()) {
                m_logFileSize = settings["LOG_FILE_SIZE"].get<uint16_t>();
            }
            else {
                m_logFileSize = 50; // значение по умолчанию
            }

            // версия XFS
            if (settings.contains("XFS_VERSION") && settings["XFS_VERSION"].is_string()) {
                m_xfsVersion = settings["XFS_VERSION"].get<std::string>();
            }
            else {
                m_xfsVersion = BASIC_XFS_VERSION; // значение по умолчанию
                setXFSVersion(m_xfsVersion);
            }

            // логическое имя сервиса
            if (settings.contains("LOGICAL_SERVICE_NAME") && settings["LOGICAL_SERVICE_NAME"].is_string()) {
                m_logicalServiceName = settings["LOGICAL_SERVICE_NAME"].get < std::string>();
            }
            else {
                m_logicalServiceName = "CIM"; // значение по умолчанию
                setLogicalServiceName(m_logicalServiceName);
            }

            // хост для подключения
            if (settings.contains("HOST") && settings["HOST"].is_string()) {
                m_host = settings["HOST"].get<std::string>();
            }
            else {
                m_host = "http://127.0.0.1"; // значение по умолчанию
                setHost(m_host);
            }

            // порт для подключения
            if (settings.contains("WS_PORT") && settings["WS_PORT"].is_number_unsigned()) {
                m_wsPort = settings["WS_PORT"].get<uint16_t>();
            }
            else {
                m_wsPort = 8080; // значение по умолчанию
                setWSPort(m_wsPort);
            }

            // использование SSL
            if (settings.contains("USE_SSL") && settings["USE_SSL"].is_boolean()) {
                m_useSSL = settings["USE_SSL"].get<bool>();
            }
            else {
                m_useSSL = true; // значение по умолчанию
                setUseSSL(m_useSSL);
            }

            // имя окна для передачи сообщения
            if (settings.contains("WIND_NAME") && settings["WIND_NAME"].is_string()) {
                m_windName = settings["WIND_NAME"].get<std::string>();
            }
            else {
                m_windName = "CIM_CIM_CIM-DBA"; // значение по умолчанию
                setWindName(m_windName);
            }


            // На этапе тестирования
            if (!settings.contains("APP_VERSION") || !settings["APP_VERSION"].is_string() || settings["APP_VERSION"].get<std::string>() != "3.40.20") {

                try {
                    nlohmann::json content = read();
                    nlohmann::json updatedContent = updateByPath(content, "APP_VERSION", APP_VERSION);
                    saveToFile(updatedContent);
                }
                catch (...) {
                    // Игнорируем ошибки на этапе тестирования
                }
            }

            // имя окна для передачи сообщения
            if (settings.contains("PERSISTENT_FILE_NAME") && settings["PERSISTENT_FILE_NAME"].is_string()) {
                m_persistentFileName = settings["PERSISTENT_FILE_NAME"].get<std::string>();
            }
            else {
                m_persistentFileName = ""; // значение по умолчанию
                setWindName(m_persistentFileName);
            }

            // имя окна для передачи сообщения
            if (settings.contains("MULTI_DEVICE") && settings["MULTI_DEVICE"].is_boolean()) {
                m_isMultiDeviceMode = settings["MULTI_DEVICE"].get<bool>();
            }
            else {
                m_isMultiDeviceMode = false; // значение по умолчанию
                setMultiDeviceMode(m_isMultiDeviceMode);
            }

            // разрешённые уровни логирования
            if (settings.contains("ENABLED_LOGLEVELS") && settings["ENABLED_LOGLEVELS"].is_array())
            {
                const auto& jLevels = settings["ENABLED_LOGLEVELS"];

                // если массив пустой — ставим дефолт 0-100
                if (jLevels.empty())
                {
                    setEnabledLogLevels(nlohmann::json::array({ 0, "1-100" }));
                }
                else
                {
                    // парсим и нормализуем через ваш setter
                    if (!setEnabledLogLevels(jLevels))
                    {
                        // если битое значение в конфиге — ставим дефолт
                        setEnabledLogLevels(nlohmann::json::array({ 0, "1-100" }));
                        // либо так, если хотите ровно как в initialContent:
                        // setEnabledLogLevels(initialContent["ENABLED_LOGLEVELS"]);
                    }
                }
            }
            else
            {
                // если ключа нет или не массив — создаём дефолт
                setEnabledLogLevels(nlohmann::json::array({ 0, "1-100" }));
            }
        }
    }

    //getter для параметров
    /** @return Емкость кассеты */
    uint32_t getCassetteCapacity() const { return m_cassetteCapacity; }
    /** @return Верхний уровень кассеты в процентах */
    uint32_t getCassetteHighLevelPercent() const {
        if (m_cassetteHighLevel < 50 || m_cassetteHighLevel > 100)
        {
            return 80;
        }
        return m_cassetteHighLevel;
    }
    /** @brief Получить режим совместного приёма. */
    bool getConjointCashInMode() const { return m_conjointCashInMode; }
    /** @return COM-порт */
    std::string getPort() const {
        return m_port;
    }
    /** @return Стратегия при включении с купюрой в приёмнике */
    PowerUpWithBillStrategy getPowerUpWithBillStrategy() const { return m_powerUpWithBillStrategy; }
    /** @return Флаг сброса счетчиков кассет */
    uint16_t getResetCassetteCounters() const { return m_resetCassetteCounters; }
    /** @return Время задержки в мс перед завершением приёма */
    uint16_t getDelayTimeAcceptanceStop() const { return m_delayTimeAcceptanceStop; }
    /** @return Режим Отключение манипуляции CU вне инкассации */
    uint16_t getDisableCuManipulated() const {
        return m_disableCuManipulated;
    }
    /** @return Размер папки для логов в мб */
    uint16_t getMaxFolderMB() const {
        return m_maxFolderMB;
    }
    /** @return Количество дней для архивации логов */
    uint16_t getArchiveDays() const {
        return m_archiveDays;
    }
    /** @return Версия XFS */
    std::string getXFSVersion() const { return m_xfsVersion; }
    /** @return Логическое имя сервиса */
    std::string getLogicalServiceName() const { return m_logicalServiceName; }
    // @return Размер лог-файла в мб */
    uint16_t getLogFileSize() const { return m_logFileSize; }
    /** @return Хост для подключения */
    std::string getHost() const { return m_host; }
    /** @return Порт для подключения */
    uint16_t getWSPort() const { return m_wsPort; }
    /** @return Использование SSL */
    bool getUseSSL() const { return m_useSSL; }
    /** @return Имя окна для передачи сообщения */
    std::string getWindName() const { return m_windName; }
    /** @return Версия приложения */
    std::string getAppVersion() const { return APP_VERSION; }
    /** @return Имя файла для сохранения данных между запусками */
    std::string getPersistentFileName() const { return m_persistentFileName; }
    /** @return Вектор разрешенных уровней логирования */
    const std::vector<LogLevelRange>& getEnabledLogLevels() const { return m_enabledLogLevels; }
    /** @return Режим многоустройственности */
    bool getMultiDeviceMode() const { return m_isMultiDeviceMode; }

    bool isLogLevelEnabled(int level) const
    {
        for (const auto& r : m_enabledLogLevels) {
            if (level >= r.lo && level <= r.hi)
                return true;
        }
        return false;
    }

    std::vector<LogLevelRange> filterByAllowedLevels(const std::vector<LogLevelRange>& in)
    {
        std::vector<LogLevelRange> out;
        const auto& allowed = allowedLogLevels();

        // Если хотите — можно заранее отсортировать allowed (но у вас и так по возрастанию)
        for (const auto& r : in)
        {
            // Пересекаем диапазон с дискретным набором allowed: получаем набор точек
            for (int v : allowed)
            {
                if (v >= r.lo && v <= r.hi)
                    out.push_back({ v, v });
            }
        }

        // Сольём соседние точки в диапазоны (например 100 и 101 -> 100-101)
        normalizeLogRanges(out);
        return out;
    }


    //setter для параметров
    /** @return установка COM-порта */
    bool setPort(const std::string& portName) {
        return templateSetter("PORT", portName, m_port);
    }

    /** @return установка емкости кассеты */
    bool setCassetteCapacity(const int& capacity)
    {
        return templateSetter("CASSETTE_CAPACITY", capacity, m_cassetteCapacity);
    }

    /** @return установка верхнего уровня кассеты в процентах */
    bool setCassetteHighLevel(const int& highLevel)
    {
        if (highLevel < 50 || highLevel > 100)
        {
            return false;
        }
        return templateSetter("CASSETTE_HIGH_LEVEL", highLevel, m_cassetteHighLevel);
    }

    /** @return установка стратегии при включении с купюрой в приёмнике */
    bool setPowerUpWithBillStrategy(const PowerUpWithBillStrategy& strategy)
    {
        std::string strategyStr;
        if (strategy == PowerUpWithBillStrategy::STOP_ACCEPT) {
            strategyStr = "STOP_ACCEPT";
        }
        else if (strategy == PowerUpWithBillStrategy::ALLOW_ACCEPT) {
            strategyStr = "ALLOW_ACCEPT";
        }
        else if (strategy == PowerUpWithBillStrategy::ALLOW_ACCEPT_BILL_STACKER) {
            strategyStr = "ALLOW_ACCEPT_BILL_STACKER";
        }
        return templateSetter("POWER_UP_WITH_BILL_STRATEGY", strategyStr, m_powerUpWithBillStrategy);
    }

    /** @return установка сброса счетчиков кассет */
    bool setResetCassetteCounters(const uint16_t& resetCounters)
    {
        return templateSetter("RESET_CASSETTE_COUNTERS", resetCounters, m_resetCassetteCounters);
    }

    /** @return установка времени задержки в мс перед завершением приёма */
    bool setDelayTimeAcceptanceStop(const uint16_t& delayTime)
    {
        if (delayTime > 15000)
        {
            return false;
        }
        return templateSetter("DELAY_TIME_ACCEPTANCE_STOP", delayTime, m_delayTimeAcceptanceStop);
    }

    /** @return установка режима Отключение манипуляции CU вне инкассации */
    bool setDisableCuManipulated(const uint16_t& disableCuManipulated)
    {
        return templateSetter("DISABLE_CU_MANIPULATED", disableCuManipulated, m_disableCuManipulated);
    }

    /** @return установка количества дней для архивации логов */
    bool setArchiveDays(const uint16_t& archiveDays)
    {
        return templateSetter("ARCHIVE_DAYS", archiveDays, m_archiveDays);
    }

    /** @return установка размера лог-файла в мб */
    bool setMaxFolderMB(const uint16_t& maxFolderMB)
    {
        return templateSetter("MAX_FOLDER_MB_FOR_LOGS", maxFolderMB, m_maxFolderMB);
    }

    /** @return установка режима совместного приёма */
    bool setConjointCashInMode(const bool& conjointCashInMode)
    {
        return templateSetter("CONJOINT_CASH_IN", conjointCashInMode ? 1 : 0, m_conjointCashInMode);
    }

    /** @return установка версии XFS */
    bool setXFSVersion(const std::string& xfsVersion = BASIC_XFS_VERSION)
    {
        return templateSetter("XFS_VERSION", xfsVersion, m_xfsVersion);
    }

    /** @return установка логического имени сервиса */
    bool setLogicalServiceName(const std::string& serviceName)
    {
        return templateSetter("LOGICAL_SERVICE_NAME", serviceName, m_logicalServiceName);
    }

    /** @return установка хоста для подключения */
    bool setHost(const std::string& host)
    {
        return templateSetter("HOST", host, m_host);
    }

    /** @return установка порта для подключения */
    bool setWSPort(const uint16_t& wsPort)
    {
        return templateSetter("WS_PORT", wsPort, m_wsPort);
    }

    /** @return установка использования SSL */
    bool setUseSSL(const bool& useSSL)
    {
        return templateSetter("USE_SSL", useSSL, m_useSSL);
    }

    /** @return установка логического имени устройства */
    bool setWindName(const std::string& windName)
    {
        return templateSetter("WIND_NAME", windName, m_windName);
    }

    /** @return установка логического имени устройства */
    bool setPersistentFileName(const std::string& persistentFileName)
    {
        return templateSetter("PERSISTENT_FILE_NAME", persistentFileName, m_persistentFileName);
    }

    /** @return установка режим работы с купюроприемником */
    bool setMultiDeviceMode(const nlohmann::json& json)
    {
        return templateSetter("MULTI_DEVICE", json, m_isMultiDeviceMode);
    }

    /** @return установка размера лог-файла в мб */
    bool setEnabledLogLevels(const nlohmann::json& levels)
    {
        return templateSetter("ENABLED_LOGLEVELS", levels, m_enabledLogLevels);
    }

    /** @return  шаблонный сеттер */
    template<typename ValueT, typename LocalT>
    bool templateSetter(const std::string& key, const ValueT& value, LocalT& local)
    {
        try
        {
            // Конвертируем входное значение в nlohmann::json — это даёт хорошие правила конверсии между типами
            nlohmann::json j = value;

            // Сохраняем значение в JSON-файл
            nlohmann::json content = read();
            nlohmann::json updatedContent = updateByPath(content, key, value);
            saveToFile(updatedContent);

            // Пытаемся получить значение нужного локального типа из json
            try
            {
                local = j.get<LocalT>();
            }
            catch (const nlohmann::json::exception&)
            {
                // Если извлечение не удалось — возвращаем false
                return false;
            }

            return true;
        }
        catch (const std::exception&)
        {
            // Любая ошибка при обновлении/сохранении — считаем операцию неуспешной
            return false;
        }
    }

    bool templateSetter(const std::string& key,
        const nlohmann::json& value,
        std::vector<LogLevelRange>& local)
    {
        try
        {
            if (!value.is_array())
                return false;

            std::vector<LogLevelRange> parsed;
            parsed.reserve(value.size());

            static const std::regex re(R"(^\s*(\d+)\s*-\s*(\d+)\s*$)");

            for (const auto& item : value)
            {
                if (item.is_number_integer() || item.is_number_unsigned())
                {
                    int v = item.get<int>();
                    parsed.push_back({ v, v });
                    continue;
                }

                if (item.is_string())
                {
                    const std::string s = item.get<std::string>();
                    std::smatch m;
                    if (!std::regex_match(s, m, re))
                        return false;

                    int lo = std::stoi(m[1].str());
                    int hi = std::stoi(m[2].str());
                    if (lo > hi) std::swap(lo, hi);

                    parsed.push_back({ lo, hi });
                    continue;
                }

                return false;
            }

            // Нормализуем вход как раньше
            normalizeLogRanges(parsed);

            // ✅ НОВОЕ: фильтрация по допустимым уровням
            auto filtered = filterByAllowedLevels(parsed);

            // ✅ НОВОЕ: если ничего не совпало — ставим дефолт 0-100
            if (filtered.empty())
            {
                filtered.push_back({ 0, 100 });
            }

            // Присваиваем локальное поле
            local = filtered;

            // Записываем нормализованное (и уже отфильтрованное) представление в файл
            nlohmann::json normalized = rangesToJson(filtered);

            nlohmann::json content = read();
            nlohmann::json updatedContent = updateByPath(content, key, normalized);
            saveToFile(updatedContent);

            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    static void normalizeLogRanges(std::vector<LogLevelRange>& ranges)
    {
        if (ranges.empty())
            return;

        std::sort(ranges.begin(), ranges.end(), [](const LogLevelRange& a, const LogLevelRange& b) {
            if (a.lo != b.lo) return a.lo < b.lo;
            return a.hi < b.hi;
            });

        std::vector<LogLevelRange> merged;
        merged.reserve(ranges.size());
        merged.push_back(ranges[0]);

        for (size_t i = 1; i < ranges.size(); ++i)
        {
            auto& last = merged.back();
            const auto& cur = ranges[i];

            // ВАЖНО: объединяем ТОЛЬКО если пересечение или соприкосновение
            // Это НЕ превращает {1,2,50,80} в 1..80
            if (cur.lo <= last.hi + 1)
            {
                if (cur.hi > last.hi) last.hi = cur.hi;
            }
            else
            {
                merged.push_back(cur);
            }
        }

        ranges = std::move(merged);
    }

    static nlohmann::json rangesToJson(const std::vector<LogLevelRange>& ranges)
    {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& r : ranges)
        {
            if (r.lo == r.hi)
                arr.push_back(r.lo);
            else
                arr.push_back(std::to_string(r.lo) + "-" + std::to_string(r.hi));
        }
        return arr;
    }

    /**
    * @brief Установить логгер для записи ошибок и отладки.
    */
    void setLogger(std::shared_ptr<ILogger> pLogger) {
        m_log = pLogger;
    }

private:
    /** Singleton instance */
    static SettingModule* pinstance_;
    /** Мьютекс для синхронизации операций с файлом */
    static std::mutex m_fileMutex;      // Для потокобезопасности операций с файлом
    /** Мьютекс для создания/удаления инстанса */
    static std::mutex m_instanceMutex;

    /** Путь к файлу настроек */
    std::string m_filePath;

    /** Кэш JSON-настроек */
    nlohmann::json settings;

    /**
    * @brief  Логгер для записи ошибок и отладки
    */
    std::shared_ptr<ILogger> m_log;

    /**
     * @brief Вспомогательный метод для парсинга пути и доступа к JSON
     * @param content JSON-объект для поиска
     * @param path Путь в точечной нотации
     * @return Найденный элемент
     * @throws SettingModuleException если ключ не найден
     */
    nlohmann::json getByPath(const nlohmann::json& content, const std::string& path) const;

    /**
 * @brief Вставить или обновить значение в JSON по пути
 *
 * Проходит по всем частям пути, создавая вложенные объекты при необходимости,
 * и устанавливает значение для последнего ключа.
 *
 * @param content Копия JSON-объекта для модификации
 * @param path Путь в точечной нотации
 * @param value Значение для установки
 * @return Обновлённый JSON
 */
    nlohmann::json updateByPath(nlohmann::json& content, const std::string& path, const nlohmann::json& value);

    /**
 * @brief Удалить элемент JSON по пути в точечной нотации
 *
 * @param content Копия JSON-объекта
 * @param path Путь к удаляемому элементу
 * @return Обновлённый JSON
 * @throws SettingModuleException если путь/ключ не найден
 */
    nlohmann::json removeByPath(nlohmann::json content, const std::string& path) const;

    /**
 * @brief Сохранить JSON в файл настроек
 *
 * Производит блокировку мьютекса для защиты операции записи.
 *
 * @param content JSON для записи
 * @throws SettingModuleException при ошибке открытия/записи файла
 */
    void saveToFile(const nlohmann::json& content) const;

    /** @brief Емкость кассеты */
    uint32_t m_cassetteCapacity;   // емкость кассеты
    /** @brief Верхний уровень кассеты (в единицах) */
    uint32_t m_cassetteHighLevel;  // максимум логической кассеты в процентах относительно емкости кассеты
    /** @brief COM порт */
    std::string m_port;           // COM порт
    /** @brief Стратегия при включении с купюрой в приёмнике */
    PowerUpWithBillStrategy m_powerUpWithBillStrategy;    // стратегия при включении с купюрой в приемнике
    /** @brief Сброс статистики принятых банкнот при EndExchange без параметров */
    uint16_t m_resetCassetteCounters;          //  сброс статистики принятых банкнот при подаче WFS_CMD_CIM_END_EXCHANGE без параметров
    /** @brief Время задержки (мс) перед отправкой команды окончания приёма */
    uint16_t m_delayTimeAcceptanceStop;   // время задержки (в мс) подачи команды завершения приема банкнот на устройство после отработки WFS_CMD_CIM_CASH_IN_END
    /** @brief режим непрерывного приёма */
    bool m_conjointCashInMode = false;
    /** @brief Отключение манипуляции CU вне инкассации  */
    uint16_t m_disableCuManipulated = 0;  /**< Отключение манипуляции CU  вне инкассации */
    /** @brief Версия XFS */
    std::string m_xfsVersion = BASIC_XFS_VERSION;   /**< Версия XFS */
    /** @brief размер лог файла */
    uint16_t m_logFileSize = 50; /**< размер лог файла */
    /** @brief Логическое имя сервиса */
    std::string m_logicalServiceName = "CIM"; /**< Логическое имя сервиса */
    /** @brief Хост для подключения */
    std::string m_host = "http://127.0.0.1";
    /** @brief Порт для подключения */
    uint16_t m_wsPort = 80;  /**< Порт для подключения  */
    /** @brief Использовать SSL */
    bool m_useSSL = true;  /**< Использовать SSL  */
    /** @brief Имя окна */
    std::string m_windName = "CIM_CIM_CIM-DBA"; /**< Имя окна  */
    /** @brief Имя персистентного файла для хранения данных между запусками */
    std::string m_persistentFileName = "";
    /** @brief Список разрешённых уровней логирования (числа и диапазоны) */
    std::vector<LogLevelRange> m_enabledLogLevels{};
    /** @brief Количество дней для хранения архива логов */
    uint16_t m_archiveDays = 30;
    /** @brief Макс. размер папки с логами в МБ */
    uint16_t m_maxFolderMB = 1024;
    /** @brief Использовать SSL */
    bool m_isMultiDeviceMode = false;

    /** @brief Начальное содержимое JSON-файла при создании. */
    nlohmann::json initialContent =
    {
        {"APP_VERSION", APP_VERSION},
        {"ARCHIVE_DAYS", 30},
        {"CASSETTE_CAPACITY", 1000},
        {"CASSETTE_HIGH_LEVEL", DEFAULT_CASSETTE_HIGH_LEVEL},
        {"CONJOINT_CASH_IN", 0},
        {"DELAY_TIME_ACCEPTANCE_STOP", 2000},
        {"DISABLE_CU_MANIPULATED", 1},
        {"ENABLED_LOGLEVELS", nlohmann::json::array({ 0, "1-100" })},
        {"HOST", "http://127.0.0.1"},
        {"LOGICAL_SERVICE_NAME", "CIM"},
        {"LOG_FILE_SIZE", 50},
        {"MAX_FOLDER_MB_FOR_LOGS", 1024},
        {"MULTI_DEVICE", false},
        {"PERSISTENT_FILE_NAME", ""},
        {"PORT", ""},
        {"POWER_UP_WITH_BILL_STRATEGY", "ALLOW_ACCEPT"},
        {"RESET_CASSETTE_COUNTERS", 0},
        {"USE_SSL", true},
        {"WIND_NAME", "CIM_CIM_CIM-DBA"},
        {"WS_PORT", 8080},
        {"XFS_VERSION", BASIC_XFS_VERSION}
    };
};


