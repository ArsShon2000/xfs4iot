#include "PersistentDatasHandler.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <filesystem>
#include <ranges>
#include <regex>

PersistentDatasHandler* PersistentDatasHandler::pinstance_ = nullptr;
std::mutex PersistentDatasHandler::m_sendMutex;
std::mutex PersistentDatasHandler::m_instanceMutex;

void PersistentDatasHandler::create(const nlohmann::json& initialContent) {
	// Копируем начальное содержимое
	nlohmann::json content = initialContent;
// Удаляем SIGNATURE_KEY, если он есть, чтобы подпись не включала его
//if (content.contains(SIGNATURE_KEY)) {
//	content.erase(SIGNATURE_KEY);
//}
//// Вычисляем подпись для содержимого без SIGNATURE_KEY
//content[SIGNATURE_KEY] = calculateSignature(content);

	m_file.close();
	m_file.open(pinstance_->m_filePath, std::ios::binary | std::ios::in | std::ios::out);
	//m_filePath = PersistentDatasHandler::createPersistentFile();
	// Сохраняем JSON в 
	if (saveToFile(content))
	{
		m_log->trace(std::format("{}(): Файл персистент успешно был сохранен", __FUNCTION__), LOGLEVELPERSISTENT);
		SettingModule::GetInstance()->setPersistentFileName(m_filePath);
	}
	else
	{
		m_log->error(std::format("{}(): Не удалось сохранить файл персистент", __FUNCTION__), LOGLEVELPERSISTENT);
	}
}

nlohmann::json PersistentDatasHandler::read() const {
	// Загружаем содержимое файла
	nlohmann::json content;
	// Попробуем прочитать данные из dorsPersistentDatas...json
	try
	{
		content = loadFromFile();
	}
	catch (...)
	{
		// Если возникнет ошибка залогируем и создадим dorsPersistentDatas...json с данными по умолчанию.
		m_log->critical(std::format("{}(): Ошибка при записи при чтения dorsPersistentDatas...json, возможно было изменение вне драйвера", __FUNCTION__), LOGLEVELPERSISTENT);

		pinstance_->m_filePath = PersistentDatasHandler::createPersistentFile();
		nlohmann::json initialContent = {
//{"signature", pinstance_->m_encryptionKey},
		   {"FRAUD_ATTEMPTS_COUNT", 0}
		};
		pinstance_->create(initialContent);
		content = loadFromFile();
	}
//// Проверяем наличие подписи
//if (!content.contains(SIGNATURE_KEY)) {
//	throw PersistentDatasHandlerException("Signature not found in JSON file");
//}
//// Извлекаем и удаляем подпись
//std::string storedSignature = content[SIGNATURE_KEY].get<std::string>();
//content.erase(SIGNATURE_KEY);
//// Проверяем подпись
//if (!verifySignature(content, storedSignature)) {
//	throw PersistentDatasHandlerException("Invalid signature: JSON file may have been tampered");
//}
	return content;
}

void PersistentDatasHandler::update(const std::string& path, const nlohmann::json& value) {
	// Читаем текущее содержимое
	nlohmann::json content = read();
	// Разбиваем путь на ключи
	std::vector<std::string> keys;
	std::stringstream ss(path);
	std::string key;
	while (std::getline(ss, key, '.')) {
		keys.push_back(key);
	}
	// Обновляем JSON по пути
	nlohmann::json* current = &content;
	for (size_t i = 0; i < keys.size() - 1; ++i) {
		if (!current->contains(keys[i])) {
			(*current)[keys[i]] = nlohmann::json::object();
		}
		current = &(*current)[keys[i]];
	}
	(*current)[keys.back()] = value;
//// Обновляем подпись
//content[SIGNATURE_KEY] = calculateSignature(content);
	// Сохраняем JSON
	if (saveToFile(content))
	{
		m_log->trace(std::format("{}(): {} = {} успешно записан", __FUNCTION__, parsePath(path), value.dump()), LOGLEVELPERSISTENT);
	}
	else
	{
		std::cout << std::format("\n\n\n\n##########################################################\n {}(): Ошибка при записи {} = {}\n##########################################################\n\n\n\n", __FUNCTION__, path, value.dump()) << std::endl;
		m_log->warn(std::format("{}(): Ошибка при записи {} = {}", __FUNCTION__, parsePath(path), value.dump()), LOGLEVELPERSISTENT);
	}
}

void PersistentDatasHandler::remove(const std::string& path) {
	// Читаем текущее содержимое
	nlohmann::json content = read();
	// Разбиваем путь на ключи
	std::vector<std::string> keys;
	std::stringstream ss(path);
	std::string key;
	while (std::getline(ss, key, '.')) {
		keys.push_back(key);
	}
	// Находим элемент для удаления
	nlohmann::json* current = &content;
	for (size_t i = 0; i < keys.size() - 1; ++i) {
		if (!current->contains(keys[i])) {
			throw PersistentDatasHandlerException("Path not found: " + path);
		}
		current = &(*current)[keys[i]];
	}
	// Удаляем ключ
	if (current->contains(keys.back())) {
		current->erase(keys.back());
	}
	else {
		throw PersistentDatasHandlerException("Key not found: " + keys.back());
	}
//// Обновляем подпись
//content[SIGNATURE_KEY] = calculateSignature(content);
	// Сохраняем JSON
	if (saveToFile(content))
	{
		m_log->trace(std::format("{}(): удаление по ключю {} успешно завершен", __FUNCTION__, path), LOGLEVELPERSISTENT);
	}
	else
	{
		m_log->warn(std::format("{}(): Ошибка при удалении по ключю {}", __FUNCTION__, path), LOGLEVELPERSISTENT);
	}
}

std::string PersistentDatasHandler::calculateSignature(const nlohmann::json& content) const {
	// Сериализуем JSON
	std::string serialized = content.dump();
	// Добавляем ключ подписи, если он не пустой
	if (!m_signKey.empty()) {
		serialized += m_signKey;
	}
	// Вычисляем SHA-256 хэш
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256(reinterpret_cast<const unsigned char*>(serialized.c_str()), serialized.size(), hash);
	// Форматируем хэш в строку
	std::stringstream ss;
	ss << std::hex << std::setfill('0');
	for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
		ss << std::setw(2) << static_cast<unsigned int>(hash[i]);
	}
	return ss.str();
}

bool PersistentDatasHandler::verifySignature(const nlohmann::json& content, const std::string& storedSignature) const {
	// Проверяем подпись, сравнивая вычисленную с сохранённой
	return calculateSignature(content) == storedSignature;
}

bool PersistentDatasHandler::saveToFile(const nlohmann::json& content) const {
	const std::string backupPath = m_filePath + ".bak";

	// 1. Читаем текущее содержимое (для диагностики и бэкапа)
	nlohmann::json beforeContent;
	try {
		beforeContent = loadFromFile();
		std::ofstream backup(backupPath, std::ios::binary | std::ios::trunc);
		if (backup.is_open()) {
//std::string oldData = encrypt(beforeContent.dump());
			std::string oldData = beforeContent.dump();
			backup.write(oldData.data(), oldData.size());
			backup.close();
		}
	}
	catch (...) {
		beforeContent = nlohmann::json::object();
	}

	// 2. Сериализуем и шифруем новый JSON
	std::string serialized = content.dump();
//std::string encrypted = encrypt(serialized);
	std::string encrypted = serialized;

	// 3. Перезаписываем файл
	{
		std::lock_guard<std::mutex> lock(m_fileMutex);
		
		std::ofstream out(
			m_filePath,
			std::ios::binary | std::ios::out | std::ios::trunc);

		if (!out.is_open()) {
			throw PersistentDatasHandlerException(
				"Failed to open file for writing: " + m_filePath);
		}

		out.write(encrypted.data(), static_cast<std::streamsize>(encrypted.size()));

		if (!out.good()) {
			throw PersistentDatasHandlerException(
				"Failed to write file: " + m_filePath);
		}

		out.flush();

		if (!out.good()) {
			throw PersistentDatasHandlerException(
				"Failed to flush file: " + m_filePath);
		}
	}

	// 4. Проверяем целостность новой записи
	bool isValid = true; // потом сделаю falsre
//try {
//	nlohmann::json afterContent = loadFromFile();
//	if (afterContent.contains(SIGNATURE_KEY)) {
//		std::string sig = afterContent[SIGNATURE_KEY];
//		afterContent.erase(SIGNATURE_KEY);
//		if (verifySignature(afterContent, sig)) {
//			isValid = true;
//		}
//	}
//}
//catch (const std::exception& e) {
//	std::cerr << " Ошибка проверки после записи: " << e.what() << std::endl;
//	m_log->trace(std::format("{}()  Ошибка проверки после записи: {}", __FUNCTION__, e.what()), LOGLEVELPERSISTENT);
//}

	// 5. Если запись некорректна — откатываем изменения
	if (!isValid) {
		std::cerr << "❌ Повреждение JSON при сохранении!" << std::endl;
		std::cerr << "----------------------------------------" << std::endl;
		std::cerr << "До:\n" << beforeContent.dump(4) << std::endl;
		std::cerr << "----------------------------------------" << std::endl;
		std::cerr << "После:\n" << content.dump(4) << std::endl;
		m_log->trace(std::format("{}() ❌ Повреждение JSON при сохранении!", __FUNCTION__), LOGLEVELPERSISTENT); ;
		m_log->trace(std::format("{}() До:\n {}", __FUNCTION__, beforeContent.dump(4)), LOGLEVELPERSISTENT);
		m_log->trace(std::format("{}() После:\n {}", __FUNCTION__, content.dump(4)), LOGLEVELPERSISTENT);

		std::cerr << "----------------------------------------" << std::endl;
		std::cerr << "🔁 Попытка восстановить из резервной копии..." << std::endl;
		m_log->trace(std::format("{}() 🔁 Попытка восстановить из резервной копии...", __FUNCTION__), LOGLEVELPERSISTENT);

		// Восстанавливаем из .bak
		try {
			if (std::filesystem::exists(backupPath)) {
				std::filesystem::copy_file(
					backupPath,
					m_filePath,
					std::filesystem::copy_options::overwrite_existing
				);
				std::cerr << "✅ Восстановлено из резервной копии: " << backupPath << std::endl;
				m_log->trace(std::format("{}() ✅ Восстановлено из резервной копии: {}", __FUNCTION__, backupPath), LOGLEVELPERSISTENT);
			}
			else {
				std::cerr << "⚠️ Резервная копия не найдена, файл повреждён!" << std::endl;
				m_log->critical(std::format("{}() ⚠️ Резервная копия не найдена, файл повреждён!", __FUNCTION__), LOGLEVELPERSISTENT);;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "❌ Ошибка при восстановлении: " << e.what() << std::endl;
			m_log->critical(std::format("{}() ❌ Ошибка при восстановлении: {}", __FUNCTION__, e.what()), LOGLEVELPERSISTENT);


			m_log->trace(std::format("{}() 🔁 Попытка создания нового файла", __FUNCTION__), LOGLEVELPERSISTENT);
			nlohmann::json initialContent = {
//{"signature", m_encryptionKey},
				//{"DISABLE_CU_MANIPULATED", 1},
				{"FRAUD_ATTEMPTS_COUNT", 0}
			};
			pinstance_->create(initialContent);
		}

	}
	else {
		// Успешно — можно удалить старый .bak (если не нужно хранить)
		std::error_code ec;
		std::filesystem::remove(backupPath, ec);
	}
	return isValid;
}


nlohmann::json PersistentDatasHandler::loadFromFile() const {
	// Читаем зашифрованные данные из файла
	std::string encrypted;
	{
		std::lock_guard<std::mutex> lock(m_fileMutex);
		if (!m_file.is_open()) {
			throw PersistentDatasHandlerException("File is not open: " + m_filePath);
		}
		// Перемещаем указатель в начало файла
		m_file.seekg(0);
		// Читаем всё содержимое файла
		encrypted.assign((std::istreambuf_iterator<char>(m_file)), std::istreambuf_iterator<char>());
		// Проверяем, не пустой ли файл
		/*if (encrypted.empty()) {
			throw PersistentDatasHandlerException("File is empty: " + m_filePath);
		}*/
		// Проверяем, нет ли ошибок чтения
		if (m_file.fail()) {
			throw PersistentDatasHandlerException("Failed to read file: " + m_filePath);
		}
	}
//// Расшифровываем данные
//std::string decrypted = decrypt(encrypted); 
	// Парсим JSON
	try {
		return nlohmann::json::parse(encrypted); //потом верну decrypted
	}
	catch (const nlohmann::json::parse_error& e) {
		throw PersistentDatasHandlerException("Failed to parse JSON: " + std::string(e.what()));
	}
}

std::string PersistentDatasHandler::encrypt(const std::string& plaintext) const {
	// Инициализация контекста шифрования
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	if (!ctx) {
		throw PersistentDatasHandlerException("Failed to create EVP_CIPHER_CTX");
	}
	// Генерация случайного IV (вектор инициализации)
	unsigned char iv[AES_BLOCK_SIZE];
	if (RAND_bytes(iv, AES_BLOCK_SIZE) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		throw PersistentDatasHandlerException("Failed to generate IV");
	}
	// Инициализация шифрования
	if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
		reinterpret_cast<const unsigned char*>(m_encryptionKey.c_str()), iv) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		throw PersistentDatasHandlerException("Failed to initialize encryption");
	}
	// Подготовка буферов
	std::vector<unsigned char> ciphertext(plaintext.size() + AES_BLOCK_SIZE);
	int len = 0;
	int ciphertext_len = 0;
	// Шифрование
	if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
		reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.size()) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		throw PersistentDatasHandlerException("Encryption failed");
	}
	ciphertext_len += len;
	// Финализация
	if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		throw PersistentDatasHandlerException("Finalizing encryption failed");
	}
	ciphertext_len += len;
	EVP_CIPHER_CTX_free(ctx);
	// Формируем результат: IV + зашифрованный текст
	std::string result(reinterpret_cast<char*>(iv), AES_BLOCK_SIZE);
	result.append(reinterpret_cast<char*>(ciphertext.data()), ciphertext_len);
	return result;
}

std::string PersistentDatasHandler::decrypt(const std::string& ciphertext) const {
	// Проверяем минимальный размер шифрованного текста
	if (ciphertext.size() < AES_BLOCK_SIZE) {
		throw PersistentDatasHandlerException("Invalid ciphertext size");
	}
	// Извлекаем IV
	const unsigned char* iv = reinterpret_cast<const unsigned char*>(ciphertext.data());
	// Инициализация контекста расшифрования
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	if (!ctx) {
		throw PersistentDatasHandlerException("Failed to create EVP_CIPHER_CTX");
	}
	// Инициализация расшифрования
	if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
		reinterpret_cast<const unsigned char*>(m_encryptionKey.c_str()), iv) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		throw PersistentDatasHandlerException("Failed to initialize decryption");
	}
	// Подготовка буферов
	std::vector<unsigned char> plaintext(ciphertext.size());
	int len = 0;
	int plaintext_len = 0;
	// Расшифрование
	if (EVP_DecryptUpdate(ctx, plaintext.data(), &len,
		reinterpret_cast<const unsigned char*>(ciphertext.data() + AES_BLOCK_SIZE),
		ciphertext.size() - AES_BLOCK_SIZE) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		throw PersistentDatasHandlerException("Decryption failed");
	}
	plaintext_len += len;
	// Финализация
	auto res = EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);
	if (res != 1) {
		EVP_CIPHER_CTX_free(ctx);
		throw PersistentDatasHandlerException("Finalizing decryption failed");
	}
	plaintext_len += len;
	EVP_CIPHER_CTX_free(ctx);
	// Возвращаем расшифрованный текст
	return std::string(reinterpret_cast<char*>(plaintext.data()), plaintext_len);
}

std::string PersistentDatasHandler::parsePath(const std::string& path)
{
	if (path == "CashInStatus.Status") return "Cтатус транзакции приема наличных";
	else if (path == "CashInStatus.NumOfRefused") return "Количество непринятых банкнот в последней транзакции";
	else if (path == "CashInStatus.NoteNumberList") return "Банкноты, обработанные в последней транзакции";
	else if (path == "CashInStatus.UnfitNoteNumberList") return "Список банкнот 4-го уровня, признанных непригодными для использования";
	else if (path == "FRAUD_ATTEMPTS_COUNT") return "Количество попыток мошенничества";
	else if (path == "Notes._unrecognized") return "Нераспознанные банкноты";
	else if (path == "Notes.CashUnit") return "Количество банкнот в cash-in кассете";
	else if (path == "Notes.ConfiguredNotes") return "Разрешенные азрешенные к приему номиналы (в битовой маске)";
	else if (path == "Notes.Denoms") return "Виды валют разрешенные для приема";
	else if (path == "Notes.AllNumOfRefused") return "Количество отбракованных банкнот";

	// Регулярка для Notes.0 ... Notes.23
	static const std::regex notesRegex(R"(Notes\.(\d+))");
	std::smatch match;

	if (std::regex_match(path, match, notesRegex))
	{
		return "Банкнота с id " + match[1].str();
	}

	return path;
}

bool PersistentDatasHandler::isValidConfiguredNoteItem(const nlohmann::json& item)
{
	if (!item.is_object())
		return false;

	if (!item.contains("enabled") || !item["enabled"].is_boolean())
		return false;

	if (!item.contains("cashItem") || !item["cashItem"].is_object())
		return false;

	const auto& cashItem = item["cashItem"];

	if (!cashItem.contains("noteID") || !cashItem["noteID"].is_number_integer())
		return false;

	if (!cashItem.contains("currency") || !cashItem["currency"].is_string())
		return false;

	if (!cashItem.contains("value") || !cashItem["value"].is_number_float())
		return false;

	if (!cashItem.contains("release") || !cashItem["release"].is_number_integer())
		return false;

	return true;
}

nlohmann::json PersistentDatasHandler::filterEnabledConfiguredNotes(const nlohmann::json& notes)
{
	if (!notes.is_object())
	{
		throw PersistentDatasHandlerException(
			"ConfiguredNotes должен быть JSON object"
		);
	}

	nlohmann::json result = nlohmann::json::object();

	for (const auto& [typeName, item] : notes.items())
	{
		if (!isValidConfiguredNoteItem(item))
		{
			throw PersistentDatasHandlerException(
				"Невалидный элемент: " + typeName
			);
		}

		//// false — отсеиваем, в persistent не сохраняем
		//if (!item["enabled"].get<bool>())
		//    continue;

		result[typeName] = item;
	}

	return result;
}

nlohmann::json PersistentDatasHandler::getDefaultConfiguredNotes()
{
	return {
		{
			"typeRUB5",
			{
				{
					"cashItem",
					{
						{"noteID", 1},
						{"currency", "RUB"},
						{"value", 5.0},
						{"release", 1}
					}
				},
				{"enabled", false}
			}
		},
		{
			"typeRUB10",
			{
				{
					"cashItem",
					{
						{"noteID", 2},
						{"currency", "RUB"},
						{"value", 10.0},
						{"release", 1}
					}
				},
				{"enabled", false}
			}
		},
		{
			"typeRUB50",
			{
				{
					"cashItem",
					{
						{"noteID", 3},
						{"currency", "RUB"},
						{"value", 50.0},
						{"release", 1}
					}
				},
				{"enabled", false}
			}
		},
		{
			"typeRUB100",
			{
				{
					"cashItem",
					{
						{"noteID", 4},
						{"currency", "RUB"},
						{"value", 100.0},
						{"release", 1}
					}
				},
				{"enabled", false}
			}
		},
		{
			"typeRUB200",
			{
				{
					"cashItem",
					{
						{"noteID", 5},
						{"currency", "RUB"},
						{"value", 200.0},
						{"release", 1}
					}
				},
				{"enabled", false}
			}
		},
		{
			"typeRUB500",
			{
				{
					"cashItem",
					{
						{"noteID", 6},
						{"currency", "RUB"},
						{"value", 500.0},
						{"release", 1}
					}
				},
				{"enabled", false}
			}
		},
		{
			"typeRUB1000",
			{
				{
					"cashItem",
					{
						{"noteID", 7},
						{"currency", "RUB"},
						{"value", 1000.0},
						{"release", 1}
					}
				},
				{"enabled", false}
			}
		},
		{
			"typeRUB2000",
			{
				{
					"cashItem",
					{
						{"noteID", 8},
						{"currency", "RUB"},
						{"value", 2000.0},
						{"release", 1}
					}
				},
				{"enabled", false}
			}
		},
		{
			"typeRUB5000",
			{
				{
					"cashItem",
					{
						{"noteID", 9},
						{"currency", "RUB"},
						{"value", 5000.0},
						{"release", 1}
					}
				},
				{"enabled", false}
			}
		}
	};
}

nlohmann::json PersistentDatasHandler::normalizeConfiguredNotes(const nlohmann::json& notes)
{
	if (!notes.is_object())
	{
		throw PersistentDatasHandlerException(
			"ConfiguredNotes должен быть JSON object"
		);
	}

	nlohmann::json normalized = getDefaultConfiguredNotes();

	for (const auto& [typeName, item] : notes.items())
	{
		if (!normalized.contains(typeName))
		{
			throw PersistentDatasHandlerException(
				"Неизвестный тип банкноты: " + typeName
			);
		}

		if (!isValidConfiguredNoteItem(item))
		{
			throw PersistentDatasHandlerException(
				"Invalid ConfiguredNotes item: " + typeName
			);
		}

		const auto& defaultCashItem = normalized[typeName]["cashItem"];
		const auto& inputCashItem = item["cashItem"];

		if (defaultCashItem["noteID"].get<int>() != inputCashItem["noteID"].get<int>() ||
			defaultCashItem["currency"].get<std::string>() != inputCashItem["currency"].get<std::string>() ||
			defaultCashItem["value"].get<double>() != inputCashItem["value"].get<double>() ||
			defaultCashItem["release"].get<int>() != inputCashItem["release"].get<int>())
		{
			throw PersistentDatasHandlerException(
				"ConfiguredNotes cashItem не содержит: " + typeName
			);
		}

		normalized[typeName]["enabled"] = item["enabled"].get<bool>();
	}

	return normalized;
}

bool PersistentDatasHandler::setConfiguredNotes(const nlohmann::json& notes)
{
	try
	{
		nlohmann::json normalizedNotes = normalizeConfiguredNotes(notes);

		update(CONFIGURED_NOTES_PATH, normalizedNotes);

		configuredNotes = normalizedNotes;

		if (m_log)
		{
			m_log->trace(
				std::format(
					"{}(): configuredNotes успешно сохранён. Всего банкнот: {}, enabled: {}",
					__FUNCTION__,
					normalizedNotes.size(),
					std::ranges::count_if(
						normalizedNotes.items(),
						[](const auto& item)
						{
							return item.value().contains("enabled") &&
								item.value()["enabled"].get<bool>();
						}
					)
				),
				LOGLEVELPERSISTENT
			);
		}

		return true;
	}
	catch (const std::exception& e)
	{
		if (m_log)
		{
			m_log->error(
				std::format(
					"{}(): ошибка сохранения configuredNotes: {}",
					__FUNCTION__,
					e.what()
				),
				LOGLEVELPERSISTENT
			);
		}

		return false;
	}
}

nlohmann::json PersistentDatasHandler::getConfiguredNotes(bool reloadFromFile)
{
	try
	{
		if (!reloadFromFile && !configuredNotes.is_null())
		{
			return configuredNotes;
		}

		nlohmann::json content = read();

		if (!content.contains("Notes") ||
			!content["Notes"].is_object() ||
			!content["Notes"].contains("ConfiguredNotes"))
		{
			configuredNotes = getDefaultConfiguredNotes();
			update(CONFIGURED_NOTES_PATH, configuredNotes);
			return configuredNotes;
		}

		configuredNotes = normalizeConfiguredNotes(
			content["Notes"]["ConfiguredNotes"]
		);

		return configuredNotes;
	}
	catch (const std::exception& e)
	{
		if (m_log)
		{
			m_log->error(
				std::format(
					"{}(): ошибка чтения configuredNotes: {}",
					__FUNCTION__,
					e.what()
				),
				LOGLEVELPERSISTENT
			);
		}

		configuredNotes = getDefaultConfiguredNotes();
		return configuredNotes;
	}
}

bool PersistentDatasHandler::setCashUnitNotes(const nlohmann::json& notes)
{
	try
	{
		if (!notes.is_object())
			return false;

		nlohmann::json normalized = nlohmann::json::object();
		for (const auto& [noteId, count] : notes.items())
		{
			if (!count.is_number_integer() && !count.is_number_unsigned())
				return false;

			const auto value = count.get<int>();
			if (value < 0)
				return false;

			normalized[noteId] = value;
		}

		update(CASH_UNIT_NOTES_PATH, normalized);
		cashUnitNotes_ = normalized;
		return true;
	}
	catch (...)
	{
		return false;
	}
}

nlohmann::json PersistentDatasHandler::getCashUnitNotes(bool reloadFromFile)
{
	try
	{
		if (!reloadFromFile && !cashUnitNotes_.is_null())
		{
			return cashUnitNotes_;
		}

		nlohmann::json content = read();
		if (!content.contains("Notes") ||
			!content["Notes"].is_object() ||
			!content["Notes"].contains("CashUnit") ||
			!content["Notes"]["CashUnit"].is_object())
		{
			cashUnitNotes_ = nlohmann::json::object();
			update(CASH_UNIT_NOTES_PATH, cashUnitNotes_);
			return cashUnitNotes_;
		}

		cashUnitNotes_ = content["Notes"]["CashUnit"];
		return cashUnitNotes_;
	}
	catch (...)
	{
		cashUnitNotes_ = nlohmann::json::object();
		return cashUnitNotes_;
	}
}

bool PersistentDatasHandler::addCashUnitNotes(const nlohmann::json& notesDelta)
{
	try
	{
		if (!notesDelta.is_object())
			return false;

		nlohmann::json current = getCashUnitNotes(true);
		if (!current.is_object())
		{
			current = nlohmann::json::object();
		}

		for (const auto& [noteId, count] : notesDelta.items())
		{
			if (!count.is_number_integer() && !count.is_number_unsigned())
				return false;

			const auto delta = count.get<int>();
			if (delta < 0)
				return false;

			const auto oldValue = current.contains(noteId) && current[noteId].is_number_integer()
				? current[noteId].get<int>()
				: 0;

			current[noteId] = oldValue + delta;
		}

		return setCashUnitNotes(current);
	}
	catch (...)
	{
		return false;
	}
}

void PersistentDatasHandler::print()
{
	nlohmann::json content;
	// Попробуем прочитать данные из dorsPersistentDatas...json
	try
	{
		content = loadFromFile();
		std::cout << content.dump(2) << std::endl;
	}
	catch (...)
	{
		m_log->critical(std::format("{}(): Ошибка при чтения dorsPersistentDatas...json, возможно было изменение вне драйвера", __FUNCTION__), LOGLEVELPERSISTENT);
	}
}


// ============================================================================
// CashInStatus
// ============================================================================
nlohmann::json PersistentDatasHandler::getCashInStatus(bool reloadFromFile)
{
	try
	{
		if (!reloadFromFile && !cashInStatus_.is_null())
		{
			return cashInStatus_;
		}

		nlohmann::json content = read();

		if (!content.contains("CashInStatus") ||
			!content["CashInStatus"].is_object())
		{
			cashInStatus_ = {
				{"Status", 0},
				{"NumOfRefused", 0},
				{"Unrecognized", 0},
				{"AcceptedItems", nlohmann::json::object()},
				{"UnfitItems", nlohmann::json::object()},
				{"DisputedItems", nlohmann::json::object()},
				{"CashItemCount", nlohmann::json::object()}
			};

			update(CASH_IN_STATUS_PATH, cashInStatus_);
			return cashInStatus_;
		}

		cashInStatus_ = content["CashInStatus"];
		return cashInStatus_;
	}
	catch (const std::exception& e)
	{
		if (m_log)
		{
			m_log->error(
				std::format("{}(): ошибка чтения CashInStatus: {}", __FUNCTION__, e.what()),
				LOGLEVELPERSISTENT);
		}

		cashInStatus_ = {
			{"Status", 0},
			{"NumOfRefused", 0},
			{"Unrecognized", 0},
			{"AcceptedItems", nlohmann::json::object()},
			{"UnfitItems", nlohmann::json::object()},
			{"DisputedItems", nlohmann::json::object()},
			{"CashItemCount", nlohmann::json::object()}
		};

		return cashInStatus_;
	}
}

bool PersistentDatasHandler::setCashInStatus(const nlohmann::json& cashInStatus)
{
	try
	{
		if (!cashInStatus.is_object())
			return false;

		update(CASH_IN_STATUS_PATH, cashInStatus);
		cashInStatus_ = cashInStatus;

		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool PersistentDatasHandler::setCashInTransactionStatus(int status)
{
	return templateSetter(
		CASH_IN_STATUS_STATUS_PATH,
		status,
		cashInStatus_["Status"]);
}

int PersistentDatasHandler::getCashInTransactionStatus(bool reloadFromFile)
{
	auto status = getCashInStatus(reloadFromFile);

	if (!status.contains("Status") || !status["Status"].is_number_integer())
		return 0;

	return status["Status"].get<int>();
}

bool PersistentDatasHandler::setCashInNumOfRefused(uint16_t count)
{
	return templateSetter(
		CASH_IN_STATUS_REFUSED_PATH,
		count,
		cashInStatus_["NumOfRefused"]);
}

uint16_t PersistentDatasHandler::getCashInUnrecognized(bool reloadFromFile)
{
	auto status = getCashInStatus(reloadFromFile);

	if (!status.contains("Unrecognized") ||
		!status["Unrecognized"].is_number_unsigned())
	{
		return 0;
	}

	return status["Unrecognized"].get<uint16_t>();
}

bool PersistentDatasHandler::setCashInUnrecognized(uint16_t count)
{
	return templateSetter(
		CASH_IN_STATUS_UNRECOGNIZED_PATH,
		count,
		cashInStatus_["Unrecognized"]);
}

uint16_t PersistentDatasHandler::getCashInNumOfRefused(bool reloadFromFile)
{
	auto status = getCashInStatus(reloadFromFile);

	if (!status.contains("NumOfRefused") ||
		!status["NumOfRefused"].is_number_unsigned())
	{
		return 0;
	}

	return status["NumOfRefused"].get<uint16_t>();
}

bool PersistentDatasHandler::setCashInCashItemCount(const nlohmann::json& count)
{
	return templateSetter(
		CASH_IN_STATUS_CASH_ITEM_COUNT_PATH,
		count,
		cashInStatus_["CashItemCount"]);
}

nlohmann::json PersistentDatasHandler::getCashInCashItemCount(bool reloadFromFile)
{
	auto status = getCashInStatus(reloadFromFile);

	if (!status.contains("CashItemCount") ||
		!status["CashItemCount"].is_number_unsigned())
	{
		return 0;
	}

	return status["CashItemCount"].get<uint16_t>();
}

bool PersistentDatasHandler::setCashInAcceptedItems(const nlohmann::json& acceptedItems)
{
	try
	{
		if (!acceptedItems.is_object())
			return false;

		update(CASH_IN_STATUS_ACCEPTED_ITEMS_PATH, acceptedItems);
		cashInStatus_["AcceptedItems"] = acceptedItems;

		return true;
	}
	catch (...)
	{
		return false;
	}
}

nlohmann::json PersistentDatasHandler::getCashInAcceptedItems(bool reloadFromFile)
{
	auto status = getCashInStatus(reloadFromFile);

	if (!status.contains("AcceptedItems") ||
		!status["AcceptedItems"].is_object())
	{
		return nlohmann::json::object();
	}

	return status["AcceptedItems"];
}

bool PersistentDatasHandler::resetCashInStatus()
{
	nlohmann::json emptyStatus = {
		{"Status", 0},
		{"NumOfRefused", 0},
		{"Unrecognized", 0},
		{"AcceptedItems", nlohmann::json::object()},
		{"UnfitItems", nlohmann::json::object()},
		{"DisputedItems", nlohmann::json::object()},
		{"CashItemCount", nlohmann::json::object()}
	};

	return setCashInStatus(emptyStatus);
}

