#pragma once

#include <chrono>
#include <string>
#include <rapidjson/document.h>

#include "web/models/tournament_meta.hpp"
#include "web/models/user_meta.hpp"
#include "web/web_tournament_store.hpp"
#include "core/stores/match_store.hpp"

class MetaServiceGatewayMapper {
public:
    std::vector<TournamentMeta> mapTournamentListResponse(const std::string &body);
    TournamentMeta mapTournamentGetResponse(const std::string &body);
    std::string mapUserAuthenticateRequest(const std::string &email, const std::string &password);
    UserMeta mapUserAuthenticateResponse(const std::string &body);

private:
    std::string documentToString(const rapidjson::Document &document);
    rapidjson::Value mapString(const std::string &str, rapidjson::Document::AllocatorType &allocator);
};
