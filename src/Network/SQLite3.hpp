#pragma once

#include <sqlite3.h>

#include <string>
#include <string_view>
#include <stdexcept>
#include <utility>
#include <vector>
#include <functional>

class SQLite3Database {
private:
    sqlite3* db_ = nullptr;

    public:
    using Row = std::vector<std::string>;
    using QueryCallback = std::function<void(const Row&)>;
    explicit SQLite3Database(const std::string& path) {
        if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
            std::string err = sqlite3_errmsg(db_);
            sqlite3_close(db_);
            db_ = nullptr;
            throw std::runtime_error("sqlite3_open failed: " + err);
        }
    }

    ~SQLite3Database() {
        if (db_) {
            sqlite3_close(db_);
        }
    }

    SQLite3Database(const SQLite3Database&) = delete;
    SQLite3Database& operator=(const SQLite3Database&) = delete;

    SQLite3Database(SQLite3Database&& other) noexcept
        : db_(std::exchange(other.db_, nullptr)) {}

    SQLite3Database& operator=(SQLite3Database&& other) noexcept {
        if (this != &other) {
            if (db_) {
                sqlite3_close(db_);
            }
            db_ = std::exchange(other.db_, nullptr);
        }
        return *this;
    }

    void execute(std::string_view sql) {
        char* errMsg = nullptr;

        if (sqlite3_exec(
                db_,
                sql.data(),
                nullptr,
                nullptr,
                &errMsg
            ) != SQLITE_OK) {

            std::string err = errMsg ? errMsg : "unknown sqlite error";
            sqlite3_free(errMsg);
            throw std::runtime_error("sqlite3_exec failed: " + err);
        }
    }

    void query(std::string_view sql, const QueryCallback& cb) {
        char* errMsg = nullptr;

        struct Context {
            const QueryCallback* cb;
        } ctx{ &cb };

        auto trampoline = [](void* data, int argc, char** argv, char**) -> int {
            auto* ctx = static_cast<Context*>(data);

            Row row;
            row.reserve(argc);
            for (int i = 0; i < argc; ++i) {
                row.emplace_back(argv[i] ? argv[i] : "NULL");
            }

            (*ctx->cb)(row);
            return 0;
        };

        if (sqlite3_exec(
                db_,
                sql.data(),
                trampoline,
                &ctx,
                &errMsg
            ) != SQLITE_OK) {

            std::string err = errMsg ? errMsg : "unknown sqlite error";
            sqlite3_free(errMsg);
            throw std::runtime_error("sqlite3_exec failed: " + err);
        }
    }

    sqlite3* native_handle() noexcept {
        return db_;
    }

    bool is_open() const noexcept {
        return db_ != nullptr;
    }
};
