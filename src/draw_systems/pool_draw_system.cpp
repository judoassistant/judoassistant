#include "draw_systems/pool_draw_system.hpp"
#include "actions/match_actions.hpp"
#include "rulesets/ruleset.hpp"
#include "stores/category_store.hpp"

std::unique_ptr<DrawSystem> PoolDrawSystem::clone() const {
    return std::make_unique<PoolDrawSystem>(*this);
}

std::string PoolDrawSystem::getName() const {
    return "Pool";
}

std::vector<std::unique_ptr<Action>> PoolDrawSystem::initCategory(const std::vector<PlayerId> &playerIds, const TournamentStore &tournament, const CategoryStore &category) {
    mMatches.clear();

    std::vector<std::unique_ptr<Action>> actions;

    // TODO: Sort the matches to maximize recovery
    for (size_t i = 0; i < playerIds.size(); ++i) {
        for (size_t j = i+1; j < playerIds.size(); ++j) {
            auto action = std::make_unique<AddMatchAction>(tournament, category.getId(), MatchType::KNOCKOUT, "Pool", false, playerIds[i], playerIds[j]);
            mMatches.push_back(action->getMatchId());
            actions.push_back(std::move(action));
        }
    }

    mPlayers = playerIds;

    return std::move(actions);
}

std::vector<std::unique_ptr<Action>> PoolDrawSystem::updateCategory(const TournamentStore & tournament, const CategoryStore & category) const {
    return {};
}

bool PoolDrawSystem::isFinished(const TournamentStore &tournament, const CategoryStore &category) const {
    for (auto matchId : mMatches) {
        const MatchStore & match = category.getMatch(matchId);
        if (match.getStatus() != MatchStatus::FINISHED)
            return false;
    }

    return true;
}

std::vector<std::pair<std::optional<unsigned int>, PlayerId>> PoolDrawSystem::getResults(const TournamentStore &tournament, const CategoryStore &category) const {
    // TODO: Implement
    std::vector<std::pair<std::optional<unsigned int>, PlayerId>> results;
    for (auto playerId : mPlayers)
        results.emplace_back(std::make_optional(0), playerId);
    return results;
}

bool PoolDrawSystem::hasFinalBlock() const {
    return false;
}
