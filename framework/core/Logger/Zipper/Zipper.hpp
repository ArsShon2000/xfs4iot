#include <filesystem>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <miniz/miniz.h>
#include "../ILogger.hpp"

//#include "./miniz/miniz.h" 

namespace fs = std::filesystem;

class Zipper {

public:
    explicit Zipper(std::shared_ptr<ILogger> logger) : logger_(logger) {}

    void maintain_logs(const fs::path& logDir, int archiveDays, std::uintmax_t maxFolderMB);

    std::jthread start_log_maintenance_thread(std::filesystem::path logDir,
        int archiveDays,
        std::uintmax_t maxFolderMB);

private:
    struct FileEntry {
        fs::path path;
        fs::file_time_type mtime;
        std::uintmax_t size;
    };

    std::chrono::system_clock::time_point to_sys(fs::file_time_type tp);

    bool add_file_to_zip(const fs::path& zipPath, const fs::path& filePath, const std::string& nameInZip);

    std::vector<FileEntry> list_files(const fs::path& dir);

    std::uintmax_t total_size_bytes(const std::vector<FileEntry>& files);

    // 1) Архивировать старше N дней
    void archive_older_than(const fs::path& logDir, int days);

    // 2) Если размер > maxMB — удалять самые старые файлы
    void enforce_size_limit(const fs::path& logDir, std::uintmax_t maxMB);

    // Единая точка запуска
    std::chrono::system_clock::time_point next_2350_local();

    std::shared_ptr<ILogger> logger_;

};
