#pragma once

#include <chrono>
#include <string>

class WebsocketJSONMapper {
public:
    std::string mapSyncClockCommandResponse(std::chrono::milliseconds clock);
};
