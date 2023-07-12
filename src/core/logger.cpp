#include <chrono>
#include <ctime>
#include <iomanip>
#include <string>

#include "core/logger.hpp"
#include "core/core.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

enum class LogColor {
    BLUE,
    RED,
    YELLOW,
};

std::string colorize_string(const std::string &str, LogColor color) {
    std::string colored_str;
    if (color == LogColor::BLUE) {
        colored_str += "\033[0;34m";
    }
    else if (color == LogColor::YELLOW) {
        colored_str += "\033[1;33m";
    }
    else if (color == LogColor::RED) {
        colored_str += "\033[0;31m";
    }

    colored_str += str;
    colored_str += "\033[0;0m";
    return colored_str;
}

std::string logLevelToString(LogLevel level) {
    if (level == LogLevel::INFO)
        return "info";
    if (level == LogLevel::WARNING)
        return "warning";
    return "error";
}

std::string logLevelToColoredString(LogLevel level) {
    const std::string str = "[" + logLevelToString(level) + "]";
    if (level == LogLevel::ERROR) {
        return colorize_string(str, LogColor::RED);
    }
    if (level == LogLevel::WARNING) {
        return colorize_string(str, LogColor::YELLOW);
    }
    return colorize_string(str, LogColor::BLUE);
}

void Logger::log_message(LogLevel level, const std::string &message, const std::vector<LoggerField> &fields) {
    // Encode message to json
    rapidjson::Document document;
    auto &allocator = document.GetAllocator();

    document.SetObject();

    auto message_value = rapidjson::Value(message.c_str(), allocator);
    document.AddMember("message", message_value, allocator);

    rapidjson::Value level_value(logLevelToString(level).c_str(), allocator);
    document.AddMember("level", level_value, allocator);

    const auto now = std::chrono::system_clock::now();
    const auto unixtime = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    rapidjson::Value time_value(std::to_string(unixtime).c_str(), allocator);
    document.AddMember("timestamp", time_value, allocator);

    for (const auto &field : fields) {
        rapidjson::Value field_key(field.key.c_str(), allocator);
        rapidjson::Value field_value(field.value.c_str(), allocator);
        document.AddMember(field_key, field_value, allocator);
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    // Print to standard out
    #ifdef DEBUG
        // Pretty print log information for debug builds
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        std::cout << std::put_time(std::localtime(&now_time_t), "%X") << " "
            << logLevelToColoredString(level) << ": "
            << message << " "
            << colorize_string(buffer.GetString(), LogColor::BLUE)
            << std::endl;
    #endif

    #ifndef DEBUG
        std::cout << buffer.GetString() << std::endl;
    #endif
}
