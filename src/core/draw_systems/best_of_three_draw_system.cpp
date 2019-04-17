#include "core/actions/match_actions.hpp"
#include "core/draw_systems/best_of_three_draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "core/stores/tournament_store.hpp"

std::unique_ptr<DrawSystem> BestOfThreeDrawSystem::clone() const {
    return std::make_unique<BestOfThreeDrawSystem>(*this);
}

std::string BestOfThreeDrawSystem::getName() const {
    return "Best-of-Three";
}

std::vector<std::unique_ptr<Action>> BestOfThreeDrawSystem::initCategory(const TournamentStore &tournament, const CategoryStore &category, const std::vector<PlayerId> &playerIds, unsigned int seed) {
    mMatches.clear();
    mPlayers = playerIds;

    std::vector<std::unique_ptr<Action>> actions;
    MatchId::Generator generator(seed);

    if (mPlayers.size() != 2)
        return actions;

    std::default_random_engine randomEng(seed);
    std::uniform_int_distribution dist(0, 1);

    size_t whitePlayer = dist(randomEng);
    size_t bluePlayer = 1 - whitePlayer;

    // Algorithm described at https://stackoverflow.com/questions/6648512/scheduling-algorithm-for-a-round-robin-tournament
    std::vector<std::optional<PlayerId>> shiftedIds;
    if (mPlayers.size() % 2 != 0)
        shiftedIds.push_back(std::nullopt);
    for (size_t i = 0; i < mPlayers.size(); ++i)
        shiftedIds.emplace_back(mPlayers[i]);

    for (size_t i = 0; i < 3; ++i) {
        auto action = std::make_unique<AddMatchAction>(MatchId::generate(category, generator), category.getId(), MatchType::KNOCKOUT, "Best-of-Three", false, playerIds[whitePlayer], playerIds[bluePlayer]);
        mMatches.push_back(action->getMatchId());
        actions.push_back(std::move(action));
    }

    return std::move(actions);
}

std::vector<std::unique_ptr<Action>> BestOfThreeDrawSystem::updateCategory(const TournamentStore &tournament, const CategoryStore &category) const {
    return {};
}

bool BestOfThreeDrawSystem::isFinished(const TournamentStore &tournament, const CategoryStore &category) const {
    for (auto matchId : mMatches) {
        const MatchStore & match = category.getMatch(matchId);
        if (match.getStatus() != MatchStatus::FINISHED)
            return false;
    }

    return true;
}


std::vector<std::pair<std::optional<unsigned int>, PlayerId>> BestOfThreeDrawSystem::getResults(const TournamentStore &tournament, const CategoryStore &category) const {
    return {};
    // std::vector<std::pair<std::optional<unsigned int>, PlayerId>> results;

    // auto status = category.getStatus(MatchType::KNOCKOUT);
    // if (status.startedMatches > 0 || status.notStartedMatches > 0) { // not finished
    //     for (auto playerId : mPlayers)
    //         results.emplace_back(std::nullopt, playerId);
    //     return results;
    // }

    // const Ruleset &ruleset = category.getRuleset();

    // std::unordered_map<PlayerId, PoolPlayerRank> ranks;
    // for (auto playerId : mPlayers) {
    //     PoolPlayerRank rank;
    //     rank.playerId = playerId;
    //     rank.wonMatches = 0;
    //     rank.ippons = 0;
    //     rank.wazaris = 0;
    //     rank.hansokuMakes = 0;
    //     rank.shidos = 0;
    //     rank.weight = tournament.getPlayer(playerId).getWeight().value_or(PlayerWeight(0.0));
    //     ranks[playerId] = std::move(rank);
    // }

    // for (auto matchId : mMatches) {
    //     const auto &match = category.getMatch(matchId);

    //     auto &whiteRank = ranks.at(match.getWhitePlayer().value());
    //     auto &blueRank = ranks.at(match.getBluePlayer().value());

    //     auto winner = ruleset.getWinner(match);
    //     if (winner == MatchStore::PlayerIndex::WHITE)
    //         whiteRank.wonMatches += 1;
    //     else if (winner == MatchStore::PlayerIndex::BLUE)
    //         blueRank.wonMatches += 1;

    //    const MatchStore::Score & whiteScore = match.getWhiteScore();
    //    whiteRank.ippons += whiteScore.ippon;
    //    whiteRank.wazaris += whiteScore.wazari;
    //    whiteRank.shidos += whiteScore.shido;
    //    whiteRank.hansokuMakes += static_cast<size_t>(whiteScore.hansokuMake);

    //    const MatchStore::Score & blueScore = match.getBlueScore();
    //    blueRank.ippons += blueScore.ippon;
    //    blueRank.wazaris += blueScore.wazari;
    //    blueRank.shidos += blueScore.shido;
    //    blueRank.hansokuMakes += static_cast<size_t>(blueScore.hansokuMake);
    // }

    // std::vector<PoolPlayerRank> sortedRanks;
    // for (const auto &pair : ranks)
    //     sortedRanks.push_back(pair.second);
    // std::sort(sortedRanks.begin(), sortedRanks.end());

    // for (size_t i = 0; i < sortedRanks.size(); ++i)
    //     results.emplace_back(i+1, sortedRanks[i].playerId);
    // return results;

    // return {};
}

bool BestOfThreeDrawSystem::hasFinalBlock() const {
    return false;
}
