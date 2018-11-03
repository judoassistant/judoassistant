#include "src/draw_systems/pool_draw_system.hpp"
#include "src/actions/match_actions.hpp"
#include "src/store_handlers/store_handler.hpp"

std::unique_ptr<DrawSystem> PoolDrawSystem::clone() const {
    return std::make_unique<PoolDrawSystem>(*this);
}

std::string PoolDrawSystem::getName() const {
    return "Pool";
}

std::vector<std::unique_ptr<Action>> PoolDrawSystem::initCategory(const std::vector<PlayerId> &playerIds, TournamentStore & tournament, CategoryStore & category) {
    mMatchIds.clear();

    std::vector<std::unique_ptr<Action>> actions;

    // TODO: Sort the matches to maximize recovery
    for (size_t i = 0; i < playerIds.size(); ++i) {
        for (size_t j = i+1; j < playerIds.size(); ++j) {
            auto action = std::make_unique<AddMatchAction>(tournament, category.getId(), MatchType::KNOCKOUT, "Pool", false, playerIds[i], playerIds[j]);
            mMatchIds.push_back(action->getMatchId());
            actions.push_back(std::move(action));
        }
    }

    return std::move(actions);
}

std::vector<std::unique_ptr<Action>> PoolDrawSystem::updateCategory(TournamentStore & tournament, CategoryStore & category) const {
    return {};
}

bool PoolDrawSystem::isFinished(TournamentStore & tournament, CategoryStore & category) const {
    const Ruleset & ruleset = category.getRuleset();

    for (auto matchId : mMatchIds) {
        const MatchStore & match = category.getMatch(matchId);
        if (!ruleset.isFinished(match))
            return false;
    }

    return true;
}

std::vector<std::pair<size_t, PlayerId>> PoolDrawSystem::get_results() const {
    return {};
}

bool PoolDrawSystem::hasFinalBlock() const {
    return false;
}
