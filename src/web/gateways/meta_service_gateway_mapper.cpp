#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <vector>

#include "web/gateways/meta_service_gateway.hpp"

std::vector<TournamentMeta> MetaServiceGatewayMapper::mapTournamentListResponse(const std::string &body) {
    rapidjson::Document document;
    document.Parse(body.c_str());

    std::vector<TournamentMeta> mappedTournaments;
    for (auto& tournament : document.GetArray()) {
        TournamentMeta mappedTournament;
        mappedTournament.id = tournament["id"].GetInt64();
        mappedTournament.name = tournament["name"].GetString();
        mappedTournament.location = tournament["location"].GetString();
        mappedTournament.date = tournament["date"].GetString();
        mappedTournaments.push_back(std::move(mappedTournament));
    }

    return mappedTournaments;
}
