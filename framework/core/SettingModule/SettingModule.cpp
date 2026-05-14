#include "SettingModule.h"
#include <fstream>
#include <sstream>
#include <iostream>

SettingModule* SettingModule::pinstance_ = nullptr;
std::mutex SettingModule::m_fileMutex;
std::mutex SettingModule::m_instanceMutex;

SettingModule* SettingModule::GetInstance(const std::string& filePath)
{
	if (pinstance_ == nullptr)
	{
		pinstance_ = new SettingModule;
		pinstance_->m_filePath = filePath;

		if (!std::filesystem::exists(filePath)) {
			if (pinstance_->m_log) pinstance_->m_log->info("Файла настроек нет или недоступен для чтения. Создается новый файл настроек с данными по умолчанию");
			// Если файл не существует, создаём его
			pinstance_->create(pinstance_->initialContent); // Создаем файл с начальными данными, если его нет
		}
	}

	return pinstance_;
}


void SettingModule::create(const nlohmann::json& initialContent) {
	std::lock_guard<std::mutex> lock(m_fileMutex);
	std::ifstream file(m_filePath);
	if (file.good()) {
		// Файл существует, ничего не делаем
		return;
	}
	file.close();

	nlohmann::json content = initialContent.is_null() ? nlohmann::json::object() : initialContent;  // Если initialContent пустой, создаём пустой объект
	saveToFile(content);
}

nlohmann::json SettingModule::read() const {
	nlohmann::json content;
	{
		std::lock_guard<std::mutex> lock(m_fileMutex);

		std::ifstream file(m_filePath);
		if (!file.is_open()) {
			if (m_log) m_log->critical("Ошибка при открытии файла: " + m_filePath);
			if (m_log) m_log->warn("Файл settings.json недоступен для чтения. Установлено данные по умолчанию");
			//throw SettingModuleException("Failed to open file: " + m_filePath);
			content = initialContent;
			return content;
		}

		try {
			file >> content;    // Чтение и парсинг JSON
		}
		catch (const nlohmann::json::parse_error& e) {
			std::cout << "Данные settings.json некорректны. Восстановлен с данные по умолчанию" << std::endl;
			content = initialContent;
			saveToFile(initialContent);
		}
		file.close();
	}
	return content;
}

nlohmann::json SettingModule::readByKey(const std::string& path) const {
	nlohmann::json content = read();
	return getByPath(content, path);
}

nlohmann::json SettingModule::getByPath(const nlohmann::json& content, const std::string& path) const {
	std::vector<std::string> keys;
	std::stringstream ss(path);
	std::string key;
	while (std::getline(ss, key, '.')) {    // Разделение пути по точкам
		keys.push_back(key);
	}

	nlohmann::json current = content;
	for (const auto& k : keys) {
		if (!current.contains(k)) {
			throw SettingModuleException("Key not found: " + path);
		}
		current = current[k];
	}
	return current;
}


void SettingModule::update(const std::string& path, const nlohmann::json& value) {
	std::lock_guard<std::mutex> lock(m_fileMutex);
	nlohmann::json content = read();
	nlohmann::json updatedContent = updateByPath(content, path, value);
	saveToFile(updatedContent);
}


nlohmann::json SettingModule::updateByPath(nlohmann::json& content, const std::string& path, const nlohmann::json& value) {
	std::vector<std::string> keys;
	std::stringstream ss(path);
	std::string key;
	while (std::getline(ss, key, '.')) {
		keys.push_back(key);
	}

	nlohmann::json* current = &content;
	for (size_t i = 0; i < keys.size() - 1; ++i) {  // Проходим по всем ключам, кроме последнего
		if (!current->contains(keys[i])) {          // Если ключ не существует
			(*current)[keys[i]] = nlohmann::json::object(); // Создаем вложенный объект, если ключ не существует
		}
		current = &(*current)[keys[i]];     // Переходим к следующему уровню
	}
	(*current)[keys.back()] = value;    // Устанавливаем значение для последнего ключа
	return content;
}

void SettingModule::remove(const std::string& path) {
	std::lock_guard<std::mutex> lock(m_fileMutex);
	nlohmann::json content = read();
	nlohmann::json updatedContent = removeByPath(content, path);
	saveToFile(updatedContent);
}


nlohmann::json SettingModule::removeByPath(nlohmann::json content, const std::string& path) const {
	std::vector<std::string> keys;
	std::stringstream ss(path);
	std::string key;
	while (std::getline(ss, key, '.')) {
		keys.push_back(key);
	}

	nlohmann::json* current = &content;
	for (size_t i = 0; i < keys.size() - 1; ++i) {
		if (!current->contains(keys[i])) {
			throw SettingModuleException("Path not found: " + path);
		}
		current = &(*current)[keys[i]];
	}

	if (current->contains(keys.back())) {   // Удаляем последний ключ
		current->erase(keys.back());
	}
	else {
		throw SettingModuleException("Key not found: " + keys.back());
	}
	return content;
}


void SettingModule::saveToFile(const nlohmann::json& content) const {
	std::ofstream file(m_filePath);
	if (!file.is_open()) {
		throw SettingModuleException("Failed to write to file: " + m_filePath);
	}
	file << content.dump(4); // С отступами для читаемости
	file.close();
}

