#include "draw_systems/knockout_draw_system.hpp"
#include "actions/match_actions.hpp"
#include "rulesets/ruleset.hpp"
#include "stores/category_store.hpp"

std::unique_ptr<DrawSystem> KnockoutDrawSystem::clone() const {
    return std::make_unique<KnockoutDrawSystem>(*this);
}

std::string KnockoutDrawSystem::getName() const {
    return "Knockout";
}

std::string matchTitle(size_t round, size_t depth) {
    if (round + 1 == depth)
        return "Final";
    if (round + 2 == depth)
        return "Semi-Final";
    if (round + 3 == depth)
        return "Quarter-Final";

    if (round == 0)
        return "1st Round";
    if (round == 1)
        return "2nd Round";
    if (round == 2)
        return "3rd Round";
    return std::to_string(round+1) + "th Round";
}

std::vector<std::unique_ptr<Action>> KnockoutDrawSystem::initCategory(const std::vector<PlayerId> &playerIds, const TournamentStore &tournament, const CategoryStore &category, unsigned int seed) {
    mMatches.clear();
    mPlayers = playerIds;

    MatchId::Generator generator(seed);
    std::vector<std::unique_ptr<Action>> actions;

    if (mPlayers.size() <= 1)
        return actions;

    std::default_random_engine random_eng(seed);
    std::shuffle(mPlayers.begin(), mPlayers.end(), random_eng);

    // Size of round 1
    size_t nodeCount = 1;
    size_t rounds = 1;
    while (nodeCount < (mPlayers.size() + 1)/2) {
        nodeCount *= 2;
        ++rounds;
    }

    // Create the list of players for the first round and add
    // (2*nodeCount-playerCount) byes
    std::vector<std::optional<PlayerId>> nodePlayers;
    nodePlayers.reserve(2*nodeCount);
    size_t j = 0;
    size_t byesNeeded = 2 * nodeCount - mPlayers.size();
    for (size_t i = 0; i < 2*nodeCount; ++i) {
        if (byesNeeded > 0 && i % 2 == 1) {
            nodePlayers.push_back(std::nullopt);
            --byesNeeded;
        }
        else {
            nodePlayers.push_back(mPlayers[j++]);
        }
    }

    assert(j == mPlayers.size());
    assert(byesNeeded == 0);

    // Create matches for all rounds
    for (size_t round = 0;; ++round) {
        for (size_t i = 0; i < nodeCount; ++i) {
            auto whiteId = nodePlayers[2*i];
            auto blueId = nodePlayers[2*i+1];
            bool isBye = (round == 0 && !(whiteId && blueId));
            auto matchType = (round + 2 < rounds ? MatchType::KNOCKOUT : MatchType::FINAL);

            auto action = std::make_unique<AddMatchAction>(MatchId::generate(category, generator), category.getId(), matchType, matchTitle(round, rounds), isBye, whiteId, blueId);
            mMatches.push_back(action->getMatchId());
            actions.push_back(std::move(action));
        }

        if (nodeCount == 1) // The final was just added
            break;

        // Update nodePlayers arrays
        std::vector<std::optional<PlayerId>> updatedNodePlayers;
        updatedNodePlayers.reserve(nodeCount/2);
        assert(updatedNodePlayers.empty());
        for (size_t i = 0; i < nodeCount; ++i) {
            auto whiteId = nodePlayers[2*i];
            auto blueId = nodePlayers[2*i+1];
            if (whiteId && !blueId)
                updatedNodePlayers.push_back(whiteId);
            else if (!whiteId && blueId)
                updatedNodePlayers.push_back(blueId);
            else
                updatedNodePlayers.push_back(std::nullopt);
        }

        nodePlayers = std::move(updatedNodePlayers);
        nodeCount /= 2;
    }

    return actions;
}

std::vector<std::unique_ptr<Action>> KnockoutDrawSystem::updateCategory(const TournamentStore & tournament, const CategoryStore & category) const {
    return {};
}

bool KnockoutDrawSystem::isFinished(const TournamentStore &tournament, const CategoryStore &category) const {
    for (auto matchId : mMatches) {
        const MatchStore & match = category.getMatch(matchId);
        if (match.getStatus() != MatchStatus::FINISHED)
            return false;
    }

    return true;
}

std::vector<std::pair<std::optional<unsigned int>, PlayerId>> KnockoutDrawSystem::getResults(const TournamentStore &tournament, const CategoryStore &category) const {
    // TODO: Implement
    std::vector<std::pair<std::optional<unsigned int>, PlayerId>> results;
    for (auto playerId : mPlayers)
        results.emplace_back(std::nullopt, playerId);
    return results;
}

bool KnockoutDrawSystem::hasFinalBlock() const {
    return true;
}
