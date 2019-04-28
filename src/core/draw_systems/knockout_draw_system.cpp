#include "core/actions/match_actions.hpp"
#include "core/draw_systems/knockout_draw_system.hpp"
#include "core/log.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"

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

std::vector<std::unique_ptr<AddMatchAction>> KnockoutDrawSystem::initCategory(const TournamentStore &tournament, const CategoryStore &category, const std::vector<PlayerId> &playerIds, unsigned int seed) {
    assert(playerIds.size() == category.getPlayers().size()); // This draw system is not made to be composed
    mMatches.clear();
    mPlayers = playerIds;

    MatchId::Generator generator(seed);
    std::vector<std::unique_ptr<AddMatchAction>> actions;

    if (mPlayers.size() <= 1)
        return actions;

    std::default_random_engine randomEngine(seed);
    std::shuffle(mPlayers.begin(), mPlayers.end(), randomEngine);

    // Size of round 1
    size_t nodeCount = 1;
    size_t rounds = 1;
    while (nodeCount < (mPlayers.size() + 1)/2) {
        nodeCount *= 2;
        ++rounds;
    }

    mInitialRoundSize = nodeCount;

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
            auto matchType = (round + 2 < rounds ? MatchType::ELIMINATION : MatchType::FINAL);

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

std::vector<std::unique_ptr<Action>> KnockoutDrawSystem::updateCategory(const TournamentStore &tournament, const CategoryStore &category) const {
    const auto &ruleset = category.getRuleset();
    std::vector<std::unique_ptr<Action>> actions;

    if (mPlayers.size() <= 1)
        return actions;

    // Iterate over all matches from 2nd round and onwards
    size_t prevLayer = 0; // Index to first node of previous layer
    size_t currentLayer = mInitialRoundSize; // Index to first node of current layer
    size_t roundSize = mInitialRoundSize / 2;

    while (currentLayer < mMatches.size()) {
        bool shouldBreak = true; // The loop can break early if no matches has finished in this round
        for (size_t i = currentLayer; i < currentLayer + roundSize; ++i) {
            auto matchId = mMatches[i];
            const auto &match = category.getMatch(matchId);

            if (match.isBye()) {
                shouldBreak = false;
                continue;
            }

            if (match.getStatus() == MatchStatus::FINISHED) {
                shouldBreak = false;
                continue;
            }

            {
                // check white player
                MatchId whiteChildId = mMatches[(i-currentLayer)*2+prevLayer];
                const auto &whiteChild = category.getMatch(whiteChildId);
                std::optional<PlayerId> whitePlayer = std::nullopt;

                if (whiteChild.getStatus() == MatchStatus::FINISHED)
                    whitePlayer = whiteChild.getPlayer(ruleset.getWinner(whiteChild).value());

                if (whitePlayer != match.getWhitePlayer())
                    actions.push_back(std::make_unique<SetMatchPlayerAction>(category.getId(), matchId, MatchStore::PlayerIndex::WHITE, whitePlayer));
            }

            {
                // check blue player
                MatchId blueChildId = mMatches[(i-currentLayer)*2+prevLayer + 1];
                const auto &blueChild = category.getMatch(blueChildId);
                std::optional<PlayerId> bluePlayer = std::nullopt;

                if (blueChild.getStatus() == MatchStatus::FINISHED)
                    bluePlayer = blueChild.getPlayer(ruleset.getWinner(blueChild).value());

                if (bluePlayer != match.getBluePlayer())
                    actions.push_back(std::make_unique<SetMatchPlayerAction>(category.getId(), matchId, MatchStore::PlayerIndex::BLUE, bluePlayer));
            }
        }

        prevLayer = currentLayer;
        currentLayer += roundSize;
        roundSize /= 2;

        if (shouldBreak)
            break;
    }

    return actions;
}

std::vector<std::pair<PlayerId, std::optional<unsigned int>>> KnockoutDrawSystem::getResults(const TournamentStore &tournament, const CategoryStore &category) const {
    std::vector<std::pair<PlayerId, std::optional<unsigned int>>> results;

    if (mPlayers.size() <= 1)
        return results;

    const auto &status = category.getStatus(MatchType::ELIMINATION) + category.getStatus(MatchType::FINAL);
    if (!status.isFinished()) // Not finished
        return results;

    const auto &ruleset = category.getRuleset();

    // Iterate over each layer(starting from the final) and add the losers.
    // Special case the root where we also add the winner
    unsigned int pos = 1;
    size_t layer_size = 1;
    size_t next_layer = 1;

    for (size_t i = 0; i < mMatches.size(); ++i) {
        if (i == next_layer) {
            pos += layer_size;
            if (i == 1)
                pos += 1;
            layer_size *= 2;
            next_layer = i + layer_size;
        }

        size_t j = mMatches.size() - i - 1;
        const auto &match = category.getMatch(mMatches[j]);

        if (match.isBye())
            continue;

        assert(match.getStatus() == MatchStatus::FINISHED);

        MatchStore::PlayerIndex winner = ruleset.getWinner(match).value();
        MatchStore::PlayerIndex loser = (winner == MatchStore::PlayerIndex::WHITE ? MatchStore::PlayerIndex::BLUE : MatchStore::PlayerIndex::WHITE);
        if (i != 0) {  // Special case root(add winner if we're at the root)
            results.emplace_back(match.getPlayer(loser).value(), pos);
        }
        else {
            results.emplace_back(match.getPlayer(winner).value(), pos);
            results.emplace_back(match.getPlayer(loser).value(), pos+1);
        }
    }

    return results;
}

bool KnockoutDrawSystem::hasFinalBlock() const {
    return true;
}
