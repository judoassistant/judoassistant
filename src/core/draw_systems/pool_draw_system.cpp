#include "core/actions/add_match_action.hpp"
#include "core/draw_systems/pool_draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/log.hpp"
#include "core/shuffle.hpp"

PoolDrawSystem::PoolDrawSystem(bool composited)
        : mComposited(composited)
{}

std::unique_ptr<DrawSystem> PoolDrawSystem::clone() const {
    return std::make_unique<PoolDrawSystem>(*this);
}

std::string PoolDrawSystem::getName() const {
    return "Pool";
}

std::unique_ptr<AddMatchAction> PoolDrawSystem::createMatch(const TournamentStore &tournament, const CategoryStore &category, const PlayerId firstPlayer, const PlayerId secondPlayer, MatchId::Generator &generator) {
    const auto matchId = MatchId::generate(category, generator);
    const auto matchTitle = mComposited ? "Elimination" : "Pool";

    const bool isFirstPlayerBlue = tournament.getPlayer(firstPlayer).getBlueJudogiHint();
    const PlayerId whitePlayer = isFirstPlayerBlue ? secondPlayer : firstPlayer;
    const PlayerId bluePlayer = isFirstPlayerBlue ? firstPlayer : secondPlayer;

    return std::make_unique<AddMatchAction>(CombinedId(category.getId(), matchId), MatchType::ELIMINATION, matchTitle, false, whitePlayer, bluePlayer);
}

std::vector<std::unique_ptr<AddMatchAction>> PoolDrawSystem::createMatchesForEvenNumberOfPlayers(const TournamentStore &tournament, const CategoryStore &category, const std::vector<PlayerId> &playerIds, MatchId::Generator &generator) {
    // Algorithm by Haselgrove and Leech, 1977, A tournament design problem
    // Algorithm described at https://stackoverflow.com/questions/6648512/scheduling-algorithm-for-a-round-robin-tournament

    std::vector<std::optional<PlayerId>> shiftedIds;
    if (playerIds.size() % 2 != 0)
        shiftedIds.push_back(std::nullopt);
    for (size_t i = 0; i < playerIds.size(); ++i)
        shiftedIds.emplace_back(playerIds[i]);

    std::vector<std::unique_ptr<AddMatchAction>> actions;
    for (size_t round = 0; round < shiftedIds.size()-1; ++round) {
        for (size_t i = 0; i < shiftedIds.size()/2; ++i) {
            size_t j = shiftedIds.size() - i - 1;
            if (!shiftedIds[i] || !shiftedIds[j]) continue;

            const PlayerId firstPlayer = shiftedIds[i].value();
            const PlayerId secondPlayer = shiftedIds[j].value();
            actions.push_back(createMatch(tournament, category, firstPlayer, secondPlayer, generator));
        }

        // shift ids
        std::optional<PlayerId> temp = shiftedIds.back();
        for (size_t i = 1; i < shiftedIds.size(); ++i)
            std::swap(shiftedIds[i], temp);
    }

    return actions;
}

std::vector<std::unique_ptr<AddMatchAction>> PoolDrawSystem::createMatchesForOddNumberOfPlayers(const TournamentStore &tournament, const CategoryStore &category, const std::vector<PlayerId> &playerIds, MatchId::Generator &generator) {
    return {}; // TODO: Implement
}

std::vector<std::unique_ptr<AddMatchAction>> PoolDrawSystem::initCategory(const TournamentStore &tournament, const CategoryStore &category, const std::vector<PlayerId> &playerIds, unsigned int seed) {
    mMatches.clear();
    mPlayers = playerIds;

    std::vector<std::unique_ptr<AddMatchAction>> actions;
    MatchId::Generator generator(seed);

    if (mPlayers.size() <= 1)
        return actions;

    std::mt19937 random_eng(seed);
    shuffle(mPlayers.begin(), mPlayers.end(), random_eng);

    if (playerIds.size() % 2 == 0)
        actions = createMatchesForEvenNumberOfPlayers(tournament, category, playerIds, generator);
    else
        actions = createMatchesForOddNumberOfPlayers(tournament, category, playerIds, generator);

    for (const auto &action : actions)
        mMatches.push_back(action->getMatchId());

    return actions;
}

std::vector<std::unique_ptr<Action>> PoolDrawSystem::updateCategory(const TournamentStore &tournament, const CategoryStore &category) const {
    return {};
}

struct PoolPlayerRank {
    PlayerId playerId;
    size_t wonMatches;
    std::chrono::milliseconds durationSum;
    size_t ippons;
    size_t wazaris;

    bool operator<(const PoolPlayerRank &other) const {
        if (wonMatches != other.wonMatches)
            return wonMatches > other.wonMatches;
        if (ippons != other.ippons)
            return ippons > other.ippons;
        if (wazaris != other.wazaris)
            return wazaris > other.wazaris;
        if (durationSum != other.durationSum)
            return durationSum < other.durationSum;
        return playerId < other.playerId; // to ensure total ordering
    }
};

std::vector<std::pair<PlayerId, std::optional<unsigned int>>> PoolDrawSystem::getResults(const TournamentStore &tournament, const CategoryStore &category) const {
    std::vector<std::pair<PlayerId, std::optional<unsigned int>>> results;

    if (mPlayers.size() <= 1)
        return results;

    const auto &status = category.getStatus(MatchType::ELIMINATION);
    if (!status.isFinished()) // elimination finished
        return results;

    const Ruleset &ruleset = category.getRuleset();

    std::unordered_map<PlayerId, PoolPlayerRank> ranks;
    for (auto playerId : mPlayers) {
        PoolPlayerRank rank;
        rank.playerId = playerId;
        rank.wonMatches = 0;
        rank.durationSum = std::chrono::milliseconds(0);
        rank.ippons = 0;
        rank.wazaris = 0;
        // rank.hansokuMakes = 0;
        // rank.shidos = 0;
        // rank.weight = tournament.getPlayer(playerId).getWeight().value_or(PlayerWeight(0.0));
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

        whiteRank.durationSum += match.getDuration();
        blueRank.durationSum += match.getDuration();
        const MatchStore::Score & whiteScore = match.getWhiteScore();
        whiteRank.ippons += whiteScore.ippon;
        whiteRank.wazaris += whiteScore.wazari;
        // whiteRank.shidos += whiteScore.shido;
        // whiteRank.hansokuMakes += static_cast<size_t>(whiteScore.hansokuMake);

        const MatchStore::Score & blueScore = match.getBlueScore();
        blueRank.ippons += blueScore.ippon;
        blueRank.wazaris += blueScore.wazari;
        // blueRank.shidos += blueScore.shido;
        // blueRank.hansokuMakes += static_cast<size_t>(blueScore.hansokuMake);
    }

    std::vector<PoolPlayerRank> sortedRanks;
    for (const auto &pair : ranks)
        sortedRanks.push_back(pair.second);
    std::sort(sortedRanks.begin(), sortedRanks.end());

    for (unsigned int i = 0; i < sortedRanks.size(); ++i)
        results.emplace_back(sortedRanks[i].playerId, i+1);
    return results;
}

bool PoolDrawSystem::hasFinalBlock() const {
    return false;
}

DrawSystemIdentifier PoolDrawSystem::getIdentifier() const {
    return DrawSystemIdentifier::POOL;
}

