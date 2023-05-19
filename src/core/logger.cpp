#include "core/logger.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

Logger::Logger() {

}

std::string logLevelToString(LogLevel level) {
    if (level == LogLevel::INFO)
        return "INFO";
    if (level == LogLevel::WARN)
        return "WARN";
    // level == LogLevel:ERROR
    return "ERROR";
}

void log_message(LogLevel level, const std::string &message) {
    // Encode message to json
    rapidjson::Document document;
    auto &allocator = document.GetAllocator();

    document.SetObject();

    rapidjson::Value level_value(logLevelToString(level).c_str(), allocator);
    document.AddMember("level", level_value, allocator);

    auto message_value = rapidjson::Value(message.c_str(), allocator);
    document.AddMember("message", message_value, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    std::cout << buffer.GetString() << std::endl;
}

// TODO: Implement using variadic fields. This is just a hacky implementation
// because I'm too lazy to look-up documentation.
void log_message(LogLevel level, const std::string &message, const LoggerField &field) {
    // Encode message to json
    rapidjson::Document document;
    auto &allocator = document.GetAllocator();

    document.SetObject();
    rapidjson::Value level_value(logLevelToString(level).c_str(), allocator);
    document.AddMember("level", level_value, allocator);

    auto message_value = rapidjson::Value(message.c_str(), allocator);
    document.AddMember("message", message_value, allocator);

    auto field_key_value = rapidjson::Value(field.key.c_str(), allocator);
    auto field_value = rapidjson::Value(field.value.c_str(), allocator);
    document.AddMember(field_key_value, field_value, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    std::cout << buffer.GetString() << std::endl;
}

void Logger::info(const std::string &msg) {
    log_message(LogLevel::INFO, msg);
}

void Logger::warn(const std::string &msg) {
    log_message(LogLevel::WARN, msg);
}

void Logger::warn(const std::string &msg, const LoggerField& field) {
    log_message(LogLevel::WARN, msg);
}

void Logger::error(const std::string &msg) {
    log_message(LogLevel::ERROR, msg);
}

void Logger::error(const std::string &msg, const LoggerField &field) {
    log_message(LogLevel::ERROR, msg);
}
