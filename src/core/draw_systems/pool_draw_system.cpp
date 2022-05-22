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

std::vector<std::pair<PlayerId, PlayerId>> PoolDrawSystem::createMatchOrderForEvenNumber(const std::vector<PlayerId> &playerIds) {
    assert(playerIds.size() % 2 == 0);
    // Algorithm by Haselgrove and Leech, 1977, A tournament design problem
    // Algorithm described in Suksompong, 2016, Scheduling Asynchronous Round-Robin Tournaments

    std::vector<PlayerId> permutedIds = playerIds;
    std::vector<std::pair<PlayerId, PlayerId>> matchOrder;

    const auto numberOfRounds = playerIds.size() - 1;
    for (size_t round = 0; round < numberOfRounds; ++round) {
        for (size_t i = 0; i < permutedIds.size() / 2; ++i) {
            size_t j = permutedIds.size() - i - 1;

            const PlayerId firstPlayer = permutedIds[i];
            const PlayerId secondPlayer = permutedIds[j];
            matchOrder.emplace_back(firstPlayer, secondPlayer);
        }

        // permute ids
        PlayerId temp = permutedIds.back();
        for (size_t i = 1; i < permutedIds.size(); ++i)
            std::swap(permutedIds[i], temp);
    }

    return matchOrder;
}

std::vector<std::pair<PlayerId, PlayerId>> PoolDrawSystem::createMatchOrderForOddNumber(const std::vector<PlayerId> &playerIds) {
    // Algorithm by Suksompong, 2016, Scheduling Asynchronous Round-Robin Tournaments
    assert(playerIds.size() % 2 == 1);

    const auto k = playerIds.size() / 2;
    const auto numberOfRounds = playerIds.size();

    std::vector<std::pair<PlayerId, PlayerId>> matchOrder;
    for (size_t round = 1; round <= numberOfRounds; ++round) {
        std::vector<std::pair<size_t, size_t>> matchSlots;

        { // Handle player 2k + 1
            const size_t playerIndex = 2 * k + 1;
            const size_t slotIndex = round / 2;
            matchSlots.emplace_back(slotIndex % (k + 1), playerIndex - 1);
        }

        for (size_t i = 1; i <= k; ++i) { // Handle player 2i - 1
            const size_t playerIndex = 2 * i - 1;
            const size_t slotIndex = (round <= 2 * i)
                ? i
                : i + (round - 2 * i);
            matchSlots.emplace_back(slotIndex % (k + 1), playerIndex - 1);
        }

        for (size_t i = 1; i <= k; ++i) { // Handle player 2i
            const size_t playerIndex = 2 * i;
            const size_t slotIndex = i + std::min(round, 2 * k + 3 - 2 * i) - 1;
            matchSlots.emplace_back(slotIndex % (k + 1), playerIndex - 1);
        }

        std::sort(matchSlots.begin(), matchSlots.end());
        for (size_t i = 1; i < matchSlots.size(); i += 2) {
          const size_t firstPlayerIndex = matchSlots[i].second;
          const size_t secondPlayerIndex = matchSlots[i+1].second;
          matchOrder.emplace_back(playerIds[firstPlayerIndex], playerIds[secondPlayerIndex]);
        }
    }

    return matchOrder;
}

std::unique_ptr<AddMatchAction> PoolDrawSystem::createMatch(const TournamentStore &tournament, const CategoryStore &category, const PlayerId firstPlayer, const PlayerId secondPlayer, MatchId::Generator &generator) {
    const auto matchId = MatchId::generate(category, generator);
    const auto matchTitle = mComposited ? "Elimination" : "Pool";

    const bool isFirstPlayerBlue = tournament.getPlayer(firstPlayer).getBlueJudogiHint();
    const PlayerId whitePlayer = isFirstPlayerBlue ? secondPlayer : firstPlayer;
    const PlayerId bluePlayer = isFirstPlayerBlue ? firstPlayer : secondPlayer;

    return std::make_unique<AddMatchAction>(CombinedId(category.getId(), matchId), MatchType::ELIMINATION, matchTitle, false, whitePlayer, bluePlayer);
}

std::vector<std::unique_ptr<AddMatchAction>> PoolDrawSystem::initCategory(const TournamentStore &tournament, const CategoryStore &category, const std::vector<PlayerId> &playerIds, unsigned int seed) {
    // Assign player ids
    mPlayers = playerIds;
    if (mPlayers.size() <= 1)
        return {};


    std::mt19937 random_eng(seed);
    shuffle(mPlayers.begin(), mPlayers.end(), random_eng);

    // Get match order
    std::vector<std::pair<PlayerId, PlayerId>> matchOrder = (mPlayers.size() % 2 == 0)
        ? createMatchOrderForEvenNumber(mPlayers)
        : createMatchOrderForOddNumber(mPlayers);

    // Create matches
    MatchId::Generator generator(seed);

    mMatches.clear();
    std::vector<std::unique_ptr<AddMatchAction>> actions;
    for (const auto &playerPair : matchOrder) {
        auto action = createMatch(tournament, category, playerPair.first, playerPair.second, generator);
        mMatches.push_back(action->getMatchId());
        actions.push_back(std::move(action));
    }

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

        const MatchStore::Score & blueScore = match.getBlueScore();
        blueRank.ippons += blueScore.ippon;
        blueRank.wazaris += blueScore.wazari;
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

