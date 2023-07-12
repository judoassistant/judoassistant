#pragma once

#include <boost/asio/traits/set_error_member.hpp>
#include <boost/system/detail/error_code.hpp>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <set>
#include <iostream>
#include <optional>

enum class LogLevel { INFO, WARN, ERROR };
struct LoggerField;

// Logger defines a logger instance.
class Logger {
public:
    Logger();

    // TODO: Implement variadic fields
    // TODO: Implement colors when logging to terminal
    void info(const std::string &msg);
    void info(const std::string &msg, const LoggerField& field);
    void warn(const std::string &msg);
    void warn(const std::string &msg, const LoggerField& field);
    void error(const std::string &msg);
    void error(const std::string &msg, const LoggerField& field);
    void debug(const std::string &msg);
};

struct LoggerField {
    LoggerField(const std::string &key, const std::string &value) {
        this->key = key;
        this->value = value;
    }

    LoggerField(const std::exception &e) {
        this->key = "error";
        this->value = e.what();
    }

    LoggerField(boost::system::error_code error_code) {
        std::stringstream ss;
        ss << "Error Code " << error_code.value() << ": " << error_code.message();

        this->key = "error";
        this->value = ss.str();
    }

    template <typename T>
    LoggerField(const std::string &key, const T &value) {
        std::stringstream ss;
        ss << value;

        this->key = key;
        this->value = ss.str();
    }

    std::string key;
    std::string value;
};
