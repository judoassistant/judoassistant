#pragma once

#include <boost/asio/traits/set_error_member.hpp>
#include <boost/system/detail/error_code.hpp>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <set>
#include <iostream>
#include <optional>

enum class LogLevel { INFO, WARNING, ERROR };
std::string logLevelToString(LogLevel level);

struct LoggerField;

// Logger defines a logger instance.
class Logger {
public:
    template<typename... LoggerFields>
    void info(const std::string &message, LoggerFields... field_args) {
        std::vector<LoggerField> fields = {field_args...};
        log_message(LogLevel::INFO, message, fields);
    }

    template<typename... LoggerFields>
    void warn(const std::string &message, LoggerFields... field_args) {
        std::vector<LoggerField> fields = {field_args...};
        log_message(LogLevel::WARNING, message, fields);
    }

    template<typename... LoggerFields>
    void error(const std::string &message, LoggerFields... field_args) {
        std::vector<LoggerField> fields = {field_args...};
        log_message(LogLevel::ERROR, message, fields);
    }

private:
    void log_message(LogLevel level, const std::string &message, const std::vector<LoggerField> &fields);
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
