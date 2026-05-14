#include "Zipper.hpp"
#include <mutex>
#include <condition_variable>

std::chrono::system_clock::time_point Zipper::to_sys(fs::file_time_type tp) {
    using namespace std::chrono;
    return time_point_cast<system_clock::duration>(
        tp - fs::file_time_type::clock::now() + system_clock::now()
    );
}

bool Zipper::add_file_to_zip(const fs::path& zipPath, const fs::path& filePath, const std::string& nameInZip)
{
    // Создаём/открываем zip и добавляем файл.
    // В miniz проще всего: читать существующий zip в память и писать обратно.
    // Для логов обычно приемлемо, если архивов не гигантских.
    // Альтернатива: делать архивы "по одному файлу" или "по дню", чтобы zip не разрастался бесконечно.

    logger_->trace(std::format("{}() - Добавление файла '{}' в архив '{}'",
        __FUNCTION__, filePath.string(), zipPath.string()));

    mz_zip_archive zip{};
    std::vector<std::uint8_t> buf;

    const bool zipExists = fs::exists(zipPath);

    if (zipExists) {
        // прочитать целиком
        logger_->trace(std::format("{}() - Архив '{}' уже существует, выполняется пересборка",
            __FUNCTION__, zipPath.string()));

        auto sz = (size_t)fs::file_size(zipPath);
        buf.resize(sz);
        std::FILE* f = std::fopen(zipPath.string().c_str(), "rb");
        if (!f)
        {
            logger_->warn(std::format("{}() - Не удалось открыть архив '{}'",
                __FUNCTION__, zipPath.string()));
            return false;
        }
        if (std::fread(buf.data(), 1, sz, f) != sz) {
            std::fclose(f);
            logger_->warn(std::format("{}() - Не удалось прочитать архив '{}'",
                __FUNCTION__, zipPath.string()));
            return false; }
        std::fclose(f);

        if (!mz_zip_reader_init_mem(&zip, buf.data(), buf.size(), 0))
        {
            return false;
        }

        // распаковать в writer (пересборка)
        mz_zip_archive writer{};
        if (!mz_zip_writer_init_file(&writer, zipPath.string().c_str(), 0)) {
            logger_->warn(std::format("{}() - Ошибка инициализации записи архива '{}'",
                __FUNCTION__, zipPath.string()));
            mz_zip_reader_end(&zip);
            return false;
        }

        // копируем все существующие файлы
        mz_uint num = mz_zip_reader_get_num_files(&zip);
        logger_->trace(std::format("{}() - В архиве найдено {} файлов",
            __FUNCTION__, num));
        for (mz_uint i = 0; i < num; ++i) {
            mz_zip_archive_file_stat st{};
            if (!mz_zip_reader_file_stat(&zip, i, &st)) continue;
            if (st.m_is_directory) continue;

            // извлечь файл в память и добавить обратно
            size_t uncomp = 0;
            void* p = mz_zip_reader_extract_to_heap(&zip, i, &uncomp, 0);
            if (!p) continue;
            mz_zip_writer_add_mem(&writer, st.m_filename, p, uncomp, MZ_BEST_COMPRESSION);
            mz_free(p);
        }

        // добавить новый файл
        if (!mz_zip_writer_add_file(&writer, nameInZip.c_str(), filePath.string().c_str(), nullptr, 0, MZ_BEST_COMPRESSION)) {
            logger_->warn(std::format("{}() - Не удалось добавить файл '{}' в архив '{}'",
                __FUNCTION__, filePath.string(), zipPath.string()));
            mz_zip_writer_end(&writer);
            mz_zip_reader_end(&zip);
            return false;
        }

        mz_zip_writer_finalize_archive(&writer);
        mz_zip_writer_end(&writer);
        mz_zip_reader_end(&zip);

        logger_->trace(std::format("{}() - Файл успешно добавлен в архив '{}'",
            __FUNCTION__, zipPath.string()));
        return true;
    }
    else {
        // новый zip
        logger_->trace(std::format("{}() - Создание нового архива '{}'",
            __FUNCTION__, zipPath.string()));

        if (!mz_zip_writer_init_file(&zip, zipPath.string().c_str(), 0))
        {
            logger_->warn(std::format("{}() - Не удалось создать архив '{}'",
                __FUNCTION__, zipPath.string()));
            return false;
        }

        if (!mz_zip_writer_add_file(&zip, nameInZip.c_str(), filePath.string().c_str(), nullptr, 0, MZ_BEST_COMPRESSION)) {
            logger_->warn(std::format("{}() - Не удалось добавить файл '{}' в новый архив '{}'",
                __FUNCTION__, filePath.string(), zipPath.string()));
            mz_zip_writer_end(&zip);
            return false;
        }

        mz_zip_writer_finalize_archive(&zip);
        mz_zip_writer_end(&zip);
        logger_->trace(std::format("{}() - Архив '{}' успешно создан",
            __FUNCTION__, zipPath.string()));
        return true;
    }
}

std::vector<Zipper::FileEntry> Zipper::list_files(const fs::path& dir)
{
    std::vector<FileEntry> out;
    if (!fs::exists(dir)) return out;

    for (const auto& de : fs::directory_iterator(dir)) {
        if (!de.is_regular_file()) continue;
        FileEntry e;
        e.path = de.path();
        e.mtime = de.last_write_time();
        e.size = de.file_size();
        out.push_back(std::move(e));
    }
    return out;
}

std::uintmax_t Zipper::total_size_bytes(const std::vector<FileEntry>& files)
{
    std::uintmax_t sum = 0;
    for (auto& f : files) sum += f.size;
    return sum;
}

// 1) Архивировать старше N дней
void Zipper::archive_older_than(const fs::path& logDir, int days)
{
    logger_->trace(std::format("{}() - Архивирование файлов старше {} дней в каталоге '{}'",
        __FUNCTION__, days, logDir.string()));

    using namespace std::chrono;

    auto files = list_files(logDir);

    const auto now = system_clock::now();
    const auto cutoff = now - hours(24LL * days);

    // Архивируем только НЕ zip
    for (const auto& f : files) {
        if (f.path.extension() == ".zip") continue;

        auto mtime_sys = to_sys(f.mtime);
        if (mtime_sys > cutoff) continue;


        logger_->trace(std::format("{}() - Архивируется файл '{}'",
            __FUNCTION__, f.path.string()));

        // Пример: один архив на месяц (можешь сделать по дню)
        // archive-YYYY-MM-dd.zip
        std::time_t tt = system_clock::to_time_t(mtime_sys);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif
        char name[64];
        std::snprintf(name, sizeof(name), "archive-%04d-%02d-%02d.zip", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

        fs::path zipPath = logDir / name;

        // имя внутри архива (просто имя файла)
        std::string nameInZip = f.path.filename().string();

        if (add_file_to_zip(zipPath, f.path, nameInZip)) {
            // удалить оригинал только если успешно добавили в архив
            std::error_code ec;
            fs::remove(f.path, ec);

            if (ec) {
                logger_->warn(std::format("{}() - Не удалось удалить исходный файл '{}'",
                    __FUNCTION__, f.path.string()));
            }
        }
    }
}

// 2) Если размер > maxMB — удалять самые старые файлы
void Zipper::enforce_size_limit(const fs::path& logDir, std::uintmax_t maxMB)
{
    logger_->trace(std::format("{}() - Контроль размера каталога '{}', лимит {} МБ",
        __FUNCTION__, logDir.string(), maxMB));

    auto files = list_files(logDir);

    // сортируем по времени (старые первые)
    std::sort(files.begin(), files.end(), [](const FileEntry& a, const FileEntry& b) {
        return a.mtime < b.mtime;
        });

    std::uintmax_t limit = maxMB * 1024ULL * 1024ULL;
    std::uintmax_t sum = total_size_bytes(files);

    logger_->trace(std::format("{}() - Текущий размер каталога: {} байт",
        __FUNCTION__, sum));

    for (const auto& f : files) {
        if (sum <= limit) break;

        std::error_code ec;
        fs::remove(f.path, ec);
        if (!ec) {
            logger_->trace(std::format("{}() - Удалён файл '{}'",
                __FUNCTION__, f.path.string()));
            sum -= f.size;
        }
        else {
            logger_->warn(std::format("{}() - Не удалось удалить файл '{}'",
                __FUNCTION__, f.path.string()));
        }
    }
}

// Единая точка запуска
void Zipper::maintain_logs(const fs::path& logDir, int archiveDays, std::uintmax_t maxFolderMB)
{
    logger_->trace(std::format("{}() - Запуск обслуживания логов. Каталог='{}', хранение={} дней, лимит={} МБ",
        __FUNCTION__, logDir.string(), archiveDays, maxFolderMB));
    archive_older_than(logDir, archiveDays);
    enforce_size_limit(logDir, maxFolderMB);
    logger_->trace(std::format("{}() - Обслуживание логов завершено",
        __FUNCTION__));
}

std::chrono::system_clock::time_point Zipper::next_2350_local()
{
    using namespace std::chrono;

    const auto now = system_clock::now();
    std::time_t tt = system_clock::to_time_t(now);
    std::tm tm{};

#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif

    tm.tm_hour = 23;
    tm.tm_min = 50;
    tm.tm_sec = 0;

    auto candidate = system_clock::from_time_t(std::mktime(&tm));
    if (candidate <= now)
        candidate += hours(24);

    return candidate;
}

std::jthread Zipper::start_log_maintenance_thread(std::filesystem::path logDir, int archiveDays, std::uintmax_t maxFolderMB)
{
    return std::jthread([=, this](std::stop_token st) {
        std::condition_variable_any cv;
        std::mutex m;
        using namespace std::chrono;

        while (!st.stop_requested()) {
            auto tp = next_2350_local();

            logger_->trace(std::format("Следующая проверка будет завтра в 23:50 (local)"));

            std::unique_lock lk(m);
            cv.wait_until(lk, tp, [&] { return st.stop_requested(); });
            if (st.stop_requested()) break;

            maintain_logs(logDir, archiveDays, maxFolderMB);
        }

        });
}
