#pragma once
#include "ILogger.hpp"

#ifdef _WIN32
// Убираем лишние определения из Windows.h и защитимся от макроса min/max
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#endif

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "LoggerRotate.hpp"
#include "../SettingModule/SettingModule.h"

class LoggerSpdlog : public ILogger
{
public:
    LoggerSpdlog()
#ifdef _WIN32
        // Получаем идентификатор процесса через Windows API и сохраняем его в m_pid для использования в логах
        : m_pid(static_cast<uint32_t>(GetCurrentProcessId()))
#else
        : m_pid(static_cast<uint32_t>(getpid()))
#endif
    {
        try {
            std::size_t maxBytes = 50u * 1024u * 1024u;
            if (auto s = SettingModule::GetInstance()->getLogFileSize(); s > 0) {
                maxBytes = static_cast<std::size_t>(s * 1024u * 1024u);
            }
            // Простой файл-синк без ротации
            auto fileSink = std::make_shared<mylog::daily_size_sink_mt>(
                "logs/dors_log",  // будет logs/dors_log_YYYY-MM-DD*.log
                maxBytes,
                false        // truncate первого файла дня? обычно false
            );
            // Создаем логгер с этим синком
            logger_ = std::make_shared<spdlog::logger>("main", fileSink);

            logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [PID:%P  TID:%t] %v");
            logger_->set_level(spdlog::level::trace);

            // Flush сразу после каждого сообщения debug и выше (info, warn, error)
            logger_->flush_on(spdlog::level::trace);
        }
        catch (const spdlog::spdlog_ex& ex) {
            throw std::runtime_error(std::string("Log init failed: ") + ex.what());
        }
    }

    void trace(const std::string& msg, uint16_t lvl = 0) override { log(spdlog::level::trace, msg, lvl); }
    void debug(const std::string& msg, uint16_t lvl = 0) override { log(spdlog::level::debug, msg, lvl); }
    void info(const std::string& msg, uint16_t lvl = 0) override { log(spdlog::level::info, msg, lvl); }
    void warn(const std::string& msg, uint16_t lvl = 0) override { log(spdlog::level::warn, msg, lvl); }
    void error(const std::string& msg, uint16_t lvl = 0) override { log(spdlog::level::err, msg, lvl); }
    void critical(const std::string& msg, uint16_t lvl = 0) override { log(spdlog::level::critical, msg, lvl); }

private:
    std::shared_ptr<spdlog::logger> logger_;
    // Идентификатор процесса
    uint32_t m_pid;

    // Генерация префикса с Lvl (показываем только Lvl, PID/TID уже в паттерне)
    std::string generateLogPrefix(uint16_t level)
    {
#ifdef _WIN32
        auto tid = static_cast<uint64_t>(GetCurrentThreadId());
#else
        auto tid = static_cast<uint64_t>(pthread_self());
#endif
        return std::string("[Lvl:") + std::to_string(level) + "] ";
    }
    void log(spdlog::level::level_enum lvl, const std::string& msg, uint16_t customLvl)
    {
        // Добавляем customLvl в начало сообщения — будет показан паттерном через %v
        if (SettingModule::GetInstance()->isLogLevelEnabled(customLvl))
            logger_->log(lvl, "{}{}", generateLogPrefix(customLvl), msg);
    }
};
