#include <cmath>

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
    mMatches.clear();
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

// Rank a list of player by the number of won matches. Only mutual matches (fought between players both in the list) are counted.
void PoolDrawSystem::orderByWinsWithinGroup(const CategoryStore &category, ResultList &results, const size_t begin, const size_t end) const {
    if (end - begin < 2)
        return; // Nothing to do

    const Ruleset &ruleset = category.getRuleset();

    std::unordered_set<PlayerId> playerIds;
    for (size_t i = begin; i != end; ++i) {
        playerIds.insert(results[i].first);
    }

    // Find number of mutual won matches
    std::unordered_map<PlayerId,unsigned int> wonMatchesByPlayerId;
    for (auto matchId : mMatches) {
        const auto &match = category.getMatch(matchId);

        const bool isMutualMatch = playerIds.find(match.getWhitePlayer().value()) != playerIds.end()
            && playerIds.find(match.getBluePlayer().value()) != playerIds.end();
        if (!isMutualMatch)
            continue;

        const MatchStore::PlayerIndex winnerIndex = ruleset.getWinner(match).value();
        const PlayerId winningPlayer = match.getPlayer(winnerIndex).value();
        wonMatchesByPlayerId[winningPlayer]++;
    }

    // Rank by mutual won matches
    sort(results.begin() + begin, results.begin() + end, [&](const auto &a, const auto &b) {
        return wonMatchesByPlayerId[a.first] > wonMatchesByPlayerId[b.first];
    });

    results[begin].second = begin+1;
    for (size_t i = begin+1; i != end; ++i) {
        const bool tiedWithPrevious = wonMatchesByPlayerId[results[i].first] == wonMatchesByPlayerId[results[i-1].first];
        results[i].second = tiedWithPrevious
            ? results[i-1].second.value()
            : i+1;
    }
}

void PoolDrawSystem::orderByRemainingCriteria(const TournamentStore &tournament, const CategoryStore &category, ResultList &results, size_t begin, size_t end) const {
    if (end - begin < 2)
        return; // Nothing to do

    std::unordered_set<PlayerId> playerIds;
    for (size_t i = begin; i != end; ++i) {
        playerIds.insert(results[i].first);
    }

    bool allPlayersHaveWeight = true;
    std::unordered_map<PlayerId, float> playerWeights;
    for (const auto playerId : playerIds) {
        const auto weight = tournament.getPlayer(playerId).getWeight();
        if (!weight.has_value()) {
            allPlayersHaveWeight = false;
            break;
        }

        playerWeights[playerId] = weight.value().toFloat();
    }

    const Ruleset &ruleset = category.getRuleset();
    std::unordered_map<PlayerId, std::chrono::milliseconds> winDuration;
    std::unordered_map<PlayerId, unsigned int> ippons;
    std::unordered_map<PlayerId, unsigned int> wazaris;
    for (auto matchId : mMatches) {
        const MatchStore &match = category.getMatch(matchId);
        const MatchStore::PlayerIndex winner = ruleset.getWinner(match).value();
        const PlayerId winningPlayerId = match.getPlayer(winner).value();

        winDuration[winningPlayerId] += match.getDuration();

        const auto &winnerScore = match.getScore(winner);
        if (winnerScore.ippon)
            ippons[winningPlayerId] += 1;
        else
            wazaris[winningPlayerId] += 1;
    }

    const auto comp = [&](const auto &a, const auto &b) {
        const PlayerId first = a.first;
        const PlayerId second = b.first;
        if (ippons[first] != ippons[second])
            return ippons[first] > ippons[second];
        if (wazaris[first] != wazaris[second])
            return wazaris[first] > wazaris[second];
        if (winDuration[first] != winDuration[second])
            return winDuration[first] < winDuration[second];

        if (allPlayersHaveWeight) {
            // Only compare by weight if all players have weight registered
            const auto playerWeightsEqual = std::fabs(playerWeights[first] - playerWeights[second]) < 0.005;
            if (!playerWeightsEqual) {
                return playerWeights[first] < playerWeights[second];
            }
        }

        return first < second;
    };

    std::sort(results.begin() + begin, results.begin() + end, comp);

    size_t rank = begin+1;
    for (auto it = results.begin() + begin; it != results.begin() + end; ++it, ++rank) {
        it->second = rank;
    }
}

DrawSystem::ResultList PoolDrawSystem::getResults(const TournamentStore &tournament, const CategoryStore &category) const {
    if (mPlayers.size() <= 1)
        return {};

    const auto &status = category.getStatus(MatchType::ELIMINATION);
    if (!status.isFinished())
        return {};

    ResultList results;
    for (size_t i = 0; i < mPlayers.size(); ++i) {
        const auto playerId = mPlayers[i];
        results.emplace_back(playerId, i+1);
    }

    // First order by number of won matches
    orderByWinsWithinGroup(category, results, 0, results.size());

    // Find ties and order by number of mutual won matches within each tied group
    {
        size_t i = 0;
        for (size_t j = 1; j <= results.size(); ++j) {
            if (j != results.size() && results[j].second == results[j-1].second)
                continue;

            // There is a tie for indices [i:j)
            orderByWinsWithinGroup(category, results, i, j);

            // Start new group
            i = j;
        }
    }

    // Find ties and order by remaining criteria
    {
        size_t i = 0;
        for (size_t j = 1; j <= results.size(); ++j) {
            if (j != results.size() && results[j].second == results[j-1].second)
                continue;

            // There is a tie for indices [i:j)
            orderByRemainingCriteria(tournament, category, results, i, j);

            // Start new group
            i = j;
        }
    }

    return results;
}

bool PoolDrawSystem::hasFinalBlock() const {
    return false;
}

DrawSystemIdentifier PoolDrawSystem::getIdentifier() const {
    return DrawSystemIdentifier::POOL;
}

