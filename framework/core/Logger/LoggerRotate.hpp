#pragma once
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/file_helper.h>

#include <filesystem>
#include <regex>
#include <string>
#include <ctime>
#include <mutex>
#include <algorithm>
#include <cstdio>

namespace fs = std::filesystem;

namespace mylog {

    inline std::string two(int v) {
        char buf[8]{};
        std::snprintf(buf, sizeof(buf), "%02d", v);
        return buf;
    }

    inline std::string today_ymd_local() {
        std::time_t t = std::time(nullptr);
        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        return std::to_string(1900 + tm.tm_year) + "-" + two(tm.tm_mon + 1) + "-" + two(tm.tm_mday);
    }

    template <typename Mutex>
    class daily_size_sink final : public spdlog::sinks::base_sink<Mutex> {
    public:
        // base_path_without_date_ext: например "logs/app"
        // max_size_bytes: лимит (по умолчанию ставьте 50MB снаружи)
        daily_size_sink(std::string base_path_without_date_ext,
            std::size_t max_size_bytes,
            bool truncate_first_file = false)
            : base_(std::move(base_path_without_date_ext))
            , max_size_(max_size_bytes)
            , truncate_first_(truncate_first_file)
        {
            fs::path basep(base_);
            dir_ = basep.has_parent_path() ? basep.parent_path() : fs::path(".");
            stem_ = basep.filename().string(); // "app"
            fs::create_directories(dir_);

            current_date_ = today_ymd_local();
            open_or_resume_today_();
        }

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override {
            // 1) новый календарный день -> новый файл дня (или продолжение, если уже есть)
            const auto now_date = today_ymd_local();
            if (now_date != current_date_) {
                current_date_ = now_date;
                current_index_ = 0;
                open_or_resume_today_();
            }

            // 2) превысили размер -> новый файл с индексом
            if (max_size_ > 0 && file_opened_) {
                if (file_.size() >= max_size_) {
                    rotate_size_();
                }
            }

            spdlog::memory_buf_t formatted;
            spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
            file_.write(formatted);
        }

        void flush_() override {
            file_.flush();
        }

    private:
        fs::path dir_;
        std::string base_;   // "logs/app"
        std::string stem_;   // "app"
        std::string current_date_; // YYYY-MM-DD
        int current_index_{ 0 };

        std::size_t max_size_{ 0 };
        bool truncate_first_{ false };

        spdlog::details::file_helper file_;
        bool file_opened_{ false };

    private:
        fs::path make_path_(const std::string& ymd, int index) const {
            // app_YYYY-MM-DD.log или app_YYYY-MM-DD_001.log
            std::string name = stem_ + "_" + ymd;
            if (index > 0) {
                char buf[16]{};
                std::snprintf(buf, sizeof(buf), "_%03d", index);
                name += buf;
            }
            name += ".log";
            return dir_ / name;
        }

        int find_max_index_for_date_(const std::string& ymd) const {
            const std::regex re("^" + stem_ + "_" + ymd + R"((?:_(\d{3}))?\.log$)");
            int max_idx = -1;

            for (const auto& e : fs::directory_iterator(dir_)) {
                if (!e.is_regular_file()) continue;
                const auto fname = e.path().filename().string();
                std::smatch m;
                if (std::regex_match(fname, m, re)) {
                    if (m[1].matched) max_idx = (max_idx > std::stoi(m[1].str())) ? max_idx : std::stoi(m[1].str());
                    else max_idx = max_idx > 0 ? max_idx : 0;
                }
            }
            return max_idx;
        }

        void open_file_(const fs::path& p, bool truncate) {
            file_.close();
            file_.open(p.string(), truncate);
            file_opened_ = true;
        }

        void open_or_resume_today_() {
            // Resume-логика после перезапуска:
            // берем самый большой индекс за today;
            // если он < max_size -> append; иначе следующий индекс.
            int max_idx = find_max_index_for_date_(current_date_);
            if (max_idx < 0) {
                current_index_ = 0;
                open_file_(make_path_(current_date_, 0), truncate_first_);
                return;
            }

            fs::path last = make_path_(current_date_, max_idx);
            std::error_code ec;
            auto sz = fs::file_size(last, ec);

            if (!ec && sz < max_size_) {
                current_index_ = max_idx;
                open_file_(last, false /*append*/);
            }
            else {
                current_index_ = max_idx + 1;
                open_file_(make_path_(current_date_, current_index_), true /*truncate*/);
            }
        }

        void rotate_size_() {
            ++current_index_;
            open_file_(make_path_(current_date_, current_index_), true /*truncate*/);
        }
    };

    using daily_size_sink_mt = daily_size_sink<std::mutex>;
    using daily_size_sink_st = daily_size_sink<spdlog::details::null_mutex>;

} // namespace mylog
