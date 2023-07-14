#include <rapidjson/writer.h>
#include <rapidjson/document.h>

#include "web/mappers/websocket_json_mapper.hpp"


std::string WebsocketJSONMapper::mapSyncClockCommandResponse(const std::chrono::milliseconds clock) {
    rapidjson::Document document;

    document.SetObject();
    auto &allocator = document.GetAllocator();

    rapidjson::Value command_type("clock", allocator);
    document.AddMember("type", command_type, allocator);
    document.AddMember("clock", clock.count(), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    return buffer.GetString();
}
