#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <vector>

#include "web/gateways/meta_service_gateway.hpp"
#include "web/gateways/meta_service_gateway_mapper.hpp"
#include "web/models/tournament_meta.hpp"

std::string MetaServiceGatewayMapper::documentToString(const rapidjson::Document &document) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    return buffer.GetString();
}

rapidjson::Value MetaServiceGatewayMapper::mapString(const std::string &str, rapidjson::Document::AllocatorType &allocator) {
    return rapidjson::Value(str.c_str(), str.size(), allocator);
}

TournamentMeta mapTournamentValue(const rapidjson::Value &tournament) {
    TournamentMeta mappedTournament;
    mappedTournament.shortName = tournament["shortName"].GetString();
    mappedTournament.name = tournament["name"].GetString();
    mappedTournament.location = tournament["location"].GetString();
    mappedTournament.date = tournament["date"].GetString();
    mappedTournament.owner = tournament["owner"].GetInt();
    return mappedTournament;
}

std::vector<TournamentMeta> MetaServiceGatewayMapper::mapTournamentListResponse(const std::string &body) {
    rapidjson::Document document;
    document.Parse(body.c_str());

    std::vector<TournamentMeta> mappedTournaments;
    for (auto& tournament : document.GetArray()) {
        TournamentMeta mappedTournament = mapTournamentValue(tournament);
        mappedTournaments.push_back(std::move(mappedTournament));
    }

    return mappedTournaments;
}

TournamentMeta MetaServiceGatewayMapper::mapTournamentResponse(const std::string &body) {
    rapidjson::Document document;
    document.Parse(body.c_str());

    return mapTournamentValue(document);
}

std::string MetaServiceGatewayMapper::mapTournamentCreateRequest(const std::string &shortName) {
    rapidjson::Document document;
    auto &allocator = document.GetAllocator();

    document.SetObject();
    document.AddMember("shortName", mapString(shortName, allocator), allocator);
    return documentToString(document);
}

std::string MetaServiceGatewayMapper::mapTournamentUpdateRequest(const TournamentMeta &tournament) {
    rapidjson::Document document;
    auto &allocator = document.GetAllocator();

    document.SetObject();
    document.AddMember("shortName", mapString(tournament.shortName, allocator), allocator);
    document.AddMember("name", mapString(tournament.name, allocator), allocator);
    document.AddMember("location", mapString(tournament.location, allocator), allocator);
    document.AddMember("date", mapString(tournament.date, allocator), allocator);
    return documentToString(document);
}

std::string MetaServiceGatewayMapper::mapUserAuthenticateRequest(const std::string &email, const std::string &password) {
    rapidjson::Document document;
    auto &allocator = document.GetAllocator();

    document.SetObject();
    document.AddMember("email", mapString(email, allocator), allocator);
    document.AddMember("password", mapString(password, allocator), allocator);
    return documentToString(document);
}

UserMeta MetaServiceGatewayMapper::mapUserAuthenticateResponse(const std::string &body) {
    rapidjson::Document document;
    document.Parse(body.c_str());

    UserMeta mappedUser;
    mappedUser.id = document["id"].GetInt();
    mappedUser.email = document["email"].GetString();
    mappedUser.isAdmin = document["isAdmin"].GetBool();
    return mappedUser;
}


