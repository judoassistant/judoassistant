#include "core/actions/match_actions.hpp"
#include "core/draw_systems/pool_draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "core/stores/tournament_store.hpp"

std::unique_ptr<DrawSystem> PoolDrawSystem::clone() const {
    return std::make_unique<PoolDrawSystem>(*this);
}

std::string PoolDrawSystem::getName() const {
    return "Pool";
}

std::vector<std::unique_ptr<AddMatchAction>> PoolDrawSystem::initCategory(const TournamentStore &tournament, const CategoryStore &category, const std::vector<PlayerId> &playerIds, unsigned int seed) {
    mMatches.clear();
    mPlayers = playerIds;

    std::vector<std::unique_ptr<AddMatchAction>> actions;
    MatchId::Generator generator(seed);

    if (mPlayers.size() <= 1)
        return actions;

    std::default_random_engine random_eng(seed);
    std::shuffle(mPlayers.begin(), mPlayers.end(), random_eng);

    // Algorithm described at https://stackoverflow.com/questions/6648512/scheduling-algorithm-for-a-round-robin-tournament
    std::vector<std::optional<PlayerId>> shiftedIds;
    if (mPlayers.size() % 2 != 0)
        shiftedIds.push_back(std::nullopt);
    for (size_t i = 0; i < mPlayers.size(); ++i)
        shiftedIds.emplace_back(mPlayers[i]);

    for (size_t round = 0; round < shiftedIds.size()-1; ++round) {
        for (size_t i = 0; i < shiftedIds.size()/2; ++i) {
            size_t j = shiftedIds.size() - i - 1;
            if (!shiftedIds[i] || !shiftedIds[j]) continue;
            auto action = std::make_unique<AddMatchAction>(MatchId::generate(category, generator), category.getId(), MatchType::ELIMINATION, "Pool", false, shiftedIds[i], shiftedIds[j]);
            mMatches.push_back(action->getMatchId());
            actions.push_back(std::move(action));
        }

        // shift ids
        std::optional<PlayerId> temp = shiftedIds.back();
        for (size_t i = 1; i < shiftedIds.size(); ++i)
            std::swap(shiftedIds[i], temp);
    }

    return std::move(actions);
}

std::vector<std::unique_ptr<Action>> PoolDrawSystem::updateCategory(const TournamentStore &tournament, const CategoryStore &category) const {
    return {};
}

struct PoolPlayerRank {
    PlayerId playerId;
    size_t wonMatches;
    size_t ippons;
    size_t wazaris;
    size_t hansokuMakes;
    size_t shidos;
    PlayerWeight weight;

    bool operator<(const PoolPlayerRank &other) const {
        if (wonMatches != other.wonMatches)
            return wonMatches > other.wonMatches;
        if (ippons != other.ippons)
            return ippons > other.ippons;
        if (wazaris != other.wazaris)
            return wazaris > other.wazaris;
        if (hansokuMakes != other.hansokuMakes)
            return hansokuMakes < other.hansokuMakes;
        if (shidos != other.shidos)
            return shidos < other.shidos;
        return weight < other.weight;
    }
};

std::vector<std::pair<std::optional<unsigned int>, PlayerId>> PoolDrawSystem::getResults(const TournamentStore &tournament, const CategoryStore &category) const {
    std::vector<std::pair<std::optional<unsigned int>, PlayerId>> results;

    if (!isFinished(tournament, category)) {
        for (auto playerId : mPlayers)
            results.emplace_back(std::nullopt, playerId);
        return results;
    }

    const Ruleset &ruleset = category.getRuleset();

    std::unordered_map<PlayerId, PoolPlayerRank> ranks;
    for (auto playerId : mPlayers) {
        PoolPlayerRank rank;
        rank.playerId = playerId;
        rank.wonMatches = 0;
        rank.ippons = 0;
        rank.wazaris = 0;
        rank.hansokuMakes = 0;
        rank.shidos = 0;
        rank.weight = tournament.getPlayer(playerId).getWeight().value_or(PlayerWeight(0.0));
        ranks[playerId] = std::move(rank);
    }

    for (auto matchId : mMatches) {
        const auto &match = category.getMatch(matchId);

        auto &whiteRank = ranks.at(match.getWhitePlayer().value());
        auto &blueRank = ranks.at(match.getBluePlayer().value());

        auto winner = ruleset.getWinner(match);
        if (winner == MatchStore::PlayerIndex::WHITE)
            whiteRank.wonMatches += 1;
        else if (winner == MatchStore::PlayerIndex::BLUE)
            blueRank.wonMatches += 1;

       const MatchStore::Score & whiteScore = match.getWhiteScore();
       whiteRank.ippons += whiteScore.ippon;
       whiteRank.wazaris += whiteScore.wazari;
       whiteRank.shidos += whiteScore.shido;
       whiteRank.hansokuMakes += static_cast<size_t>(whiteScore.hansokuMake);

       const MatchStore::Score & blueScore = match.getBlueScore();
       blueRank.ippons += blueScore.ippon;
       blueRank.wazaris += blueScore.wazari;
       blueRank.shidos += blueScore.shido;
       blueRank.hansokuMakes += static_cast<size_t>(blueScore.hansokuMake);
    }

    std::vector<PoolPlayerRank> sortedRanks;
    for (const auto &pair : ranks)
        sortedRanks.push_back(pair.second);
    std::sort(sortedRanks.begin(), sortedRanks.end());

    for (size_t i = 0; i < sortedRanks.size(); ++i)
        results.emplace_back(i+1, sortedRanks[i].playerId);
    return results;

    return {};
}

bool PoolDrawSystem::hasFinalBlock() const {
    return false;
}

bool PoolDrawSystem::isFinished(const TournamentStore &tournament, const CategoryStore &category) const {
    if (mPlayers.size() == category.getPlayers().size()) { // in case the pool draw system is used on its own
        auto status = category.getStatus(MatchType::ELIMINATION);
        return status.finishedMatches == mMatches.size();
    }

    for (auto matchId : mMatches) {
        const auto &match = category.getMatch(matchId);
        if (match.getStatus() != MatchStatus::FINISHED)
            return false;
    }

    return true;
}

