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

std::string colorizeString(const std::string &str, LogColor color) {
    std::string coloredStr;
    if (color == LogColor::BLUE) {
        coloredStr += "\033[0;34m";
    }
    else if (color == LogColor::YELLOW) {
        coloredStr += "\033[1;33m";
    }
    else if (color == LogColor::RED) {
        coloredStr += "\033[0;31m";
    }

    coloredStr += str;
    coloredStr += "\033[0;0m";
    return coloredStr;
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
        return colorizeString(str, LogColor::RED);
    }
    if (level == LogLevel::WARNING) {
        return colorizeString(str, LogColor::YELLOW);
    }
    return colorizeString(str, LogColor::BLUE);
}

void Logger::logMessage(LogLevel level, const std::string &message, const std::vector<LoggerField> &fields) {
    // Encode message to json
    rapidjson::Document document;
    auto &allocator = document.GetAllocator();

    document.SetObject();

    auto messageValue = rapidjson::Value(message.c_str(), allocator);
    document.AddMember("message", messageValue, allocator);

    rapidjson::Value levelValue(logLevelToString(level).c_str(), allocator);
    document.AddMember("level", levelValue, allocator);

    const auto now = std::chrono::system_clock::now();
    const auto unixtime = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    rapidjson::Value timeValue(std::to_string(unixtime).c_str(), allocator);
    document.AddMember("timestamp", timeValue, allocator);

    for (const auto &field : fields) {
        rapidjson::Value fieldKey(field.key.c_str(), allocator);
        rapidjson::Value fieldValue(field.value.c_str(), allocator);
        document.AddMember(fieldKey, fieldValue, allocator);
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
            << colorizeString(buffer.GetString(), LogColor::BLUE)
            << std::endl;
    #endif

    #ifndef DEBUG
        std::cout << buffer.GetString() << std::endl;
    #endif
}
