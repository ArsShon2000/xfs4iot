#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <format>
#include <filesystem>
#include <cctype>
#include "IPersistentData.hpp"
#include "../Logger/ILogger.hpp"

class FilePersistentData : public IPersistentData
{
public:
    explicit FilePersistentData(
        std::shared_ptr<ILogger> logger,
        std::filesystem::path basePath = "persistent")
        : logger_(std::move(logger))
        , basePath_(std::move(basePath))
    {
        std::filesystem::create_directories(basePath_);
    }

    bool StoreJson(const std::string& name, const nlohmann::json& j) override
    {
        try {
            const auto path = MakePath(name);

            std::ofstream file(path, std::ios::out | std::ios::trunc);
            if (!file.is_open()) {
                logger_->warn(std::format(
                    "{}() - FilePersistentData: Failed to open file for writing. {}",
                    __FUNCTION__, path.string()));
                return false;
            }

            file << j.dump(4);
            return true;
        }
        catch (const std::exception& ex) {
            logger_->warn(std::format(
                "{}() - FilePersistentData: Exception caught on writing data. {}, {}",
                __FUNCTION__, name, ex.what()));
            return false;
        }
    }

    std::shared_ptr<nlohmann::json> LoadJson(const std::string& name) override
    {
        try {
            const auto path = MakePath(name);

            // Если файла нет — создаём пустой JSON объект
            if (!std::filesystem::exists(path))
            {
                logger_->trace(std::format(
                    "{}() - FilePersistentData: Persistent file does not exist. Creating new file: {}",
                    __FUNCTION__,
                    path.string()));

                std::ofstream createFile(path);
                if (!createFile.is_open())
                {
                    logger_->warn(std::format(
                        "{}() - FilePersistentData: Failed to create persistent file. {}",
                        __FUNCTION__,
                        path.string()));

                    return nullptr;
                }

                createFile << "{}";
                createFile.close();
            }

            std::ifstream file(path);
            if (!file.is_open()) {
                logger_->warn(std::format(
                    "{}() - FilePersistentData: Failed to open file for reading. {}",
                    __FUNCTION__,
                    path.string()));
                return nullptr;
            }

            nlohmann::json j;
            file >> j;

            return std::make_shared<nlohmann::json>(std::move(j));
        }
        catch (const std::exception& ex) {
            logger_->warn(std::format(
                "{}() - FilePersistentData: Exception caught on reading persistent data. {}, {}",
                __FUNCTION__,
                name,
                ex.what()));

            return nullptr;
        }
    }

private:
    std::filesystem::path MakePath(const std::string& name) const
    {
        return basePath_ / SafeFileName(name);
    }

    static std::string SafeFileName(std::string name)
    {
        for (char& c : name) {
            if (!std::isalnum(static_cast<unsigned char>(c))) {
                c = '_';
            }
        }

        return name + ".json";
    }

private:
    std::shared_ptr<ILogger> logger_;
    std::filesystem::path basePath_;
};