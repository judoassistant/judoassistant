#pragma once

#include <chrono>
#include <string>
#include <rapidjson/document.h>

#include "web/models/tournament_meta.hpp"
#include "web/web_tournament_store.hpp"
#include "core/stores/match_store.hpp"

class MetaServiceGatewayMapper {
public:
    std::vector<TournamentMeta> mapTournamentListResponse(const std::string &body);

private:
};
