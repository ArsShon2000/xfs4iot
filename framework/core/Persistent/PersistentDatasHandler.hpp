#ifndef __JSONHANDLER_H__
#define __JSONHANDLER_H__

#include <string>
#include <nlohmann/json.hpp>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <stdexcept>
#include <mutex>
#include <iostream>
#include <fstream> // Для std::fstream
#include <optional>
#include <chrono>
#include "../Logger/ILogger.hpp"
#include "../SettingModule/SettingModule.h"

/**
 * @file PersistentDatasHandler.h
 * @brief Менеджер постоянных данных приложения (шифрование, подпись, хранение в файле).
 *
 * Класс `PersistentDatasHandler` обеспечивает сохранение и загрузку
 * конфигурационных/постоянных данных в зашифрованный файл в формате JSON.
 * Поддерживается вычисление SHA-256 подписи, шифрование AES-256-CBC, а также
 * удобные методы для чтения/обновления отдельных значений по точечному пути
 * (формат пути: "root.child.key").
 */

/**
 * @class PersistentDatasHandler
 * @brief Управление постоянными данными с шифрованием и подписью.
 *
 * Обеспечивает:
 *  - создание начального защищённого JSON-файла;
 *  - безопасное чтение и валидацию подписи;
 *  - обновление и удаление значений по путям;
 *  - получение значений с поддержкой значений по умолчанию;
 */
class PersistentDatasHandler {
public:
    /**
     * @class PersistentDatasHandlerException
     * @brief Исключение, выбрасываемое методом PersistentDatasHandler при ошибках.
     */
    class PersistentDatasHandlerException : public std::runtime_error {
    public:
        /**
         * @brief Конструктор исключения.
         * @param message Текст ошибки.
         */
        PersistentDatasHandlerException(const std::string& message) : std::runtime_error(message) {}
    };

    PersistentDatasHandler() = default;
    PersistentDatasHandler(const PersistentDatasHandler&) = delete;
    PersistentDatasHandler& operator=(const PersistentDatasHandler&) = delete;

    /**
     * @brief Деструктор. Закрывает файл, если он открыт.
     */
    ~PersistentDatasHandler() {
        // Защищаем доступ к файлу мьютексом для потокобезопасности
        std::lock_guard<std::mutex> lock(m_fileMutex);
        if (m_file.is_open()) {
            m_file.close(); // Закрываем файл, чтобы освободить ресурсы
        }
    }

    /**
     * @brief Создать начальный зашифрованный JSON-файл.
     * @param initialContent JSON с начальными данными (без поля подписи).
     */
    void create(const nlohmann::json& initialContent);

    static std::string createPersistentFile(const std::filesystem::path& directory = "./persistent/")
    {
        using namespace std::chrono;

        // Текущее время
        auto now = system_clock::now();
        auto sec = time_point_cast<std::chrono::seconds>(now);
        auto ms = duration_cast<milliseconds>(now - sec).count();

        std::time_t tt = system_clock::to_time_t(sec);
        std::tm tm{};

#ifdef _WIN32
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif

        std::ostringstream oss;
        oss << "dorsPersistentDatas-("
            << std::put_time(&tm, "%Y-%m-%d %H");

        oss << "-" << std::put_time(&tm, "%M-%S");

        oss << "." << std::setw(3) << std::setfill('0') << ms
            << ").json";

        std::filesystem::create_directories(directory);

        std::filesystem::path fullPath = directory / oss.str();

        // Создаём пустой JSON-файл
        std::ofstream file(fullPath);
        if (!file)
            throw std::runtime_error("Failed to create file: " + fullPath.string());

        file << "{}"; // можно оставить пустым или записать базовую структуру
        file.close();

        return fullPath.string();
    }

    /**
     * @brief Прочитать и вернуть содержимое файла (проверяет подпись).
     * @return Расшифрованный JSON с удалённым полем подписи.
     * @throws PersistentDatasHandlerException при ошибке чтения/проверки подписи.
     */
    nlohmann::json read() const;

    /**
     * @brief Обновить или добавить значение по указанному пути.
     * @param path Точечный путь (root.child.key) к полю.
     * @param value JSON-значение для сохранения.
     *
     * После обновления автоматически пересчитывается подпись и данные сохраняются в файл.
     */
    void update(const std::string& path, const nlohmann::json& value);

    /**
     * @brief Удалить значение по указанному пути.
     * @param path Точечный путь (root.child.key) к удаляемому полю.
     * @throws PersistentDatasHandlerException если путь или ключ не найден.
     */
    void remove(const std::string& path);

    /**
     * @brief Получить singleton-экземпляр PersistentDatasHandler.
     * @param filePath Путь к файлу хранения (по умолчанию "dorsPersistentDatas.json").
     * @param signKey Ключ для подписи (используется при расчёте SHA-256).
     * @param encryptionKey Ключ для AES-256 шифрования (должен быть ровно 32 байта).
     * @return Указатель на singleton-экземпляр.
     * @throws PersistentDatasHandlerException при ошибках создания/открытия файла или неподходящем ключе.
     */
    static PersistentDatasHandler* GetInstance(std::shared_ptr<ILogger> pLogger = nullptr
        , const std::string& filePath = SettingModule::GetInstance()->getPersistentFileName()
        , const std::string& signKey = "SffPJ3DZ"
        , const std::string& encryptionKey = "12345678901234567890123456789012") {

        // Защищаем создание экземпляра мьютексом для потокобезопасности
        std::lock_guard<std::mutex> lock(m_instanceMutex);
        if (pinstance_ == nullptr) {
            pinstance_ = new PersistentDatasHandler();
            pinstance_->setLogger(pLogger);
            pinstance_->m_filePath = filePath;
            pinstance_->m_signKey = signKey;
            pinstance_->m_encryptionKey = encryptionKey;

            // Проверяем, что ключ шифрования имеет длину 32 байта для AES-256
            if (encryptionKey.size() != 32) {
                throw PersistentDatasHandlerException("Encryption key must be 32 bytes long");
            }

            // Проверяем существование файла и открываем его с помощью std::fstream
            {
                bool isFileExists = true;
                {
                    std::lock_guard<std::mutex> fileLock(pinstance_->m_fileMutex);
                    if (!std::filesystem::exists(filePath)) {
                        // Если файл не существует, создаём его
                        isFileExists = false;
                        pinstance_->m_filePath = PersistentDatasHandler::createPersistentFile();
                        /*std::ofstream tempFile(filePath, std::ios::binary);
                        if (!tempFile.is_open()) {
                            throw PersistentDatasHandlerException("Failed to create file: " + std::string(filePath));
                        }
                        tempFile.close();*/
                    }
                }

                // Открываем файл с std::fstream для удержания его открытым
                // Режим std::ios::in | std::ios::out позволяет читать и писать
                {
                    std::lock_guard<std::mutex> fileLock(pinstance_->m_fileMutex);
                    pinstance_->m_file.open(pinstance_->m_filePath, std::ios::binary | std::ios::in | std::ios::out);
                    if (!pinstance_->m_file.is_open()) {
                        throw PersistentDatasHandlerException("Failed to open file: " + std::string(filePath));
                    }
                }

                if (!isFileExists)
                {
                    nlohmann::json initialContent = {
                        {"signature", encryptionKey},
                        {"FRAUD_ATTEMPTS_COUNT", 0}
                    };
                    pinstance_->create(initialContent); 
                    pinstance_->getConfiguredNotes();
                }
            }

            // Читаем содержимое файла для проверки корректности
            nlohmann::json content = pinstance_->read();
            
            std::cout << "Updated JSON: " << content.dump(4) << std::endl;
        }
        return pinstance_;
    }

    /**
     * @brief Уничтожить singleton-экземпляр и освободить ресурсы.
     */
    static void DestroyInstance() {
        // Защищаем уничтожение экземпляра мьютексом
        std::lock_guard<std::mutex> lock(m_instanceMutex);
        delete pinstance_;
        pinstance_ = nullptr;
    }

    /** @brief Получить флаг отключения обработки вытаскивания кассеты вне инкассации. */
    //uint8_t getDisableCuManipulated() const { return m_disableCuManipulated; }
    /** @brief Получить количество попыток мошенничества (threshold). */
    uint8_t getFraudAttemptsCount() const { return m_fraudAttemptsCount; }


    // Setter-методы для локальных параметров с обновлением JSON
    /** @brief Установить флаг отключения обработки вытаскивания кассеты вне инкассации. */
    bool setFraudAttemptsCount(uint16_t fraudAttemptsCount)
    {
        return templateSetter("FRAUD_ATTEMPTS_COUNT", fraudAttemptsCount, m_fraudAttemptsCount);
    }

  /*  bool setDisableCuManipulated(uint16_t disableCuManipulated)
    {
        return templateSetter("DISABLE_CU_MANIPULATED", disableCuManipulated, m_disableCuManipulated);
    }*/

    bool setManipulatedFlag(uint16_t manipulatedFlag)
    {
        return templateSetter("MANIPULATED_FLAG", manipulatedFlag, m_manipulatedFlag);
    }


    bool setConfiguredNotes(const nlohmann::json& notes);
    nlohmann::json getConfiguredNotes(bool reloadFromFile = false);


     /** @return  шаблонный сеттер */
    template<typename ValueT, typename LocalT>
    bool templateSetter(const std::string& key, const ValueT& value, LocalT& local)
    {
        try
        {
            // Конвертируем входное значение в nlohmann::json — это даёт хорошие правила конверсии между типами
            nlohmann::json j = value;

            // Сохраняем значение в JSON-файл
            update(key, j);

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

    /**
    * @brief Установить логгер для записи ошибок и отладки.
    */
    void setLogger(std::shared_ptr<ILogger> pLogger) {
        m_log = pLogger;
    }

private:
    // Singleton-статические поля
    static PersistentDatasHandler* pinstance_;
    static std::mutex m_sendMutex; // Для потокобезопасности операций отправки
    static std::mutex m_instanceMutex; // Для потокобезопасности создания экземпляра

    // Поля экземпляра
    mutable std::mutex m_fileMutex; // Для потокобезопасности операций с файлом
    mutable std::fstream m_file; // Удерживаем файл открытым для предотвращения удаления
    std::string m_filePath;       ///< Путь к файлу хранения
    std::string m_signKey;        ///< Ключ подписи
    std::string m_encryptionKey;  ///< Ключ для AES-шифрования
    static constexpr const char* SIGNATURE_KEY = "signature";

    //uint16_t m_disableCuManipulated;   // отслеживание извлечения кассеты вне инкассации
    uint16_t m_fraudAttemptsCount;     // количество попыток мошенничества
    uint16_t m_manipulatedFlag;                     // флаг манипуляции
    nlohmann::json configuredNotes;

    static constexpr const char* CONFIGURED_NOTES_PATH = "Notes.ConfiguredNotes";

    bool isValidConfiguredNoteItem(const nlohmann::json& item);
    nlohmann::json filterEnabledConfiguredNotes(const nlohmann::json& notes);

    nlohmann::json getDefaultConfiguredNotes();

    nlohmann::json normalizeConfiguredNotes(
        const nlohmann::json& notes);

    // Логгер для записи ошибок и отладки
    std::shared_ptr<ILogger> m_log;
    /**
     * @brief Проверить подпись JSON.
     * @param content JSON без поля подписи.
     * @param storedSignature Сохранённая подпись для сравнения.
     * @return true если подпись верна.
     */
    bool verifySignature(const nlohmann::json& content, const std::string& storedSignature) const;

    /**
     * @brief Сохранить зашифрованный JSON в файл (производит шифрование и запись).
     * @param content JSON (включая поле подписи) для сохранения.
     */
    bool saveToFile(const nlohmann::json& content) const;

    /**
     * @brief Загрузить и расшифровать JSON из файла.
     * @return Расшифрованный JSON.
     */
    nlohmann::json loadFromFile() const;

    /**
     * @brief Вычислить SHA-256 подпись для переданного JSON (возвращается в hex строке).
     * @param content JSON (без поля подписи) для подписи.
     * @return hex строка SHA-256 подписи.
     */
    std::string calculateSignature(const nlohmann::json& content) const;

    /**
     * @brief Зашифровать строку plaintext с помощью AES-256-CBC. Результат = IV || ciphertext.
     * @param plaintext Входной текст для шифрования.
     * @return Строка, содержащая IV (AES_BLOCK_SIZE байт) и затем зашифрованные данные.
     */
    std::string encrypt(const std::string& plaintext) const;

    /**
     * @brief Расшифровать строку, содержащую IV и ciphertext (формат, возвращаемый encrypt).
     * @param ciphertext Строка IV || ciphertext.
     * @return Расшифрованный текст.
     */
    std::string decrypt(const std::string& ciphertext) const;


    std::string parsePath(const std::string& path);

    void print();
};

#endif // __JSONHANDLER_H__
