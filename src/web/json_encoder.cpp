#include "web/json_encoder.hpp"

// rapidjson::Value jsonFromString(const std::string &str, rapidjson::Document::AllocatorType& allocator) {
//     rapidjson::Value json;
//     json.SetString(str.c_str(), str.size(), allocator);
//     return json;
// }

// void LoadedTournament::generateSyncJson(GenerateSyncJsonCallback callback) {
//     boost::asio::dispatch(mStrand, [this, callback](){
//         rapidjson::Document document;
//         document.SetObject();
//         auto &allocator = document.GetAllocator();

//         document.AddMember("name", jsonFromString(mTournament->getName(), allocator), allocator);

//         rapidjson::Value players(rapidjson::kArrayType);
//         for (const auto &p : mTournament->getPlayers())
//             players.PushBack(generatePlayerJson(*(p.second), allocator), allocator);
//         document.AddMember("players", players, allocator);

//         rapidjson::Value categories(rapidjson::kArrayType);
//         for (const auto &p : mTournament->getCategories())
//             categories.PushBack(generateCategoryJson(*(p.second), allocator), allocator);
//         document.AddMember("categories", categories, allocator);

//         auto buffer = std::make_shared<rapidjson::StringBuffer>();
//         rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer);
//         document.Accept(writer);
//         boost::asio::dispatch(mContext, std::bind(callback, std::move(buffer)));
//     });
// }

// rapidjson::Value LoadedTournament::generatePlayerJson(const PlayerStore &player, rapidjson::Document::AllocatorType& allocator) {
//     rapidjson::Value res;
//     res.SetObject();

//     res.AddMember("id", player.getId().getValue(), allocator);
//     res.AddMember("firstName", jsonFromString(player.getFirstName(), allocator), allocator);
//     res.AddMember("lastName", jsonFromString(player.getLastName(), allocator), allocator);

//     if (player.getCountry().has_value())
//         res.AddMember("country", jsonFromString(player.getCountry()->toString(), allocator), allocator);
//     else
//         res.AddMember("country", rapidjson::Value(), allocator);

//     if (player.getRank().has_value())
//         res.AddMember("rank", jsonFromString(player.getRank()->toString(), allocator), allocator);
//     else
//         res.AddMember("rank", rapidjson::Value(), allocator);

//     if (player.getSex().has_value())
//         res.AddMember("sex", jsonFromString(player.getSex()->toString(), allocator), allocator);
//     else
//         res.AddMember("sex", rapidjson::Value(), allocator);

//     res.AddMember("club", jsonFromString(player.getClub(), allocator), allocator);

//     rapidjson::Value categories(rapidjson::kArrayType);
//     for (const auto &categoryId : player.getCategories())
//         categories.PushBack(categoryId.getValue(), allocator);
//     res.AddMember("categories", categories, allocator);

//     // rapidjson::Value matches(rapidjson::kArrayType);
//     // for (const auto &p : player.getMatches()) {
//     //     rapidjson::Value match;
//     //     match.SetObject();
//     //     match.AddMember("categoryId", p.first.getValue());
//     //     match.AddMember("matchId", p.second.getValue());
//     //     matches.PushBack(match);
//     // }
//     // res.AddMember("matches", matches, allocator);

//     return res;
// }

// rapidjson::Value LoadedTournament::generateCategoryJson(const CategoryStore &category, rapidjson::Document::AllocatorType& allocator) {
//     rapidjson::Value res;
//     res.SetObject();

//     res.AddMember("id", category.getId().getValue(), allocator);
//     res.AddMember("name", jsonFromString(category.getName(), allocator), allocator);

//     rapidjson::Value players(rapidjson::kArrayType);
//     for (const auto &playerId : category.getPlayers())
//         players.PushBack(playerId.getValue(), allocator);
//     res.AddMember("players", players, allocator);

//     // rapidjson::Value matches(rapidjson::kArrayType);
//     // for (const auto &p : category.getMatches()) {
//     //     rapidjson::Value match;
//     //     match.SetObject();
//     //     match.AddMember("categoryId", p.first.getValue());
//     //     match.AddMember("matchId", p.second.getValue());
//     //     matches.PushBack(match);
//     // }
//     // res.AddMember("matches", matches, allocator);

//     return res;
// }

