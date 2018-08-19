#include "src/draw_strategies/pool_draw_strategy.hpp"
#include "src/actions/create_match_action.hpp"

void PoolDrawStrategy::initCategory(const std::vector<Id> &players, std::unique_ptr<TournamentStore> & tournament, std::unique_ptr<CategoryStore> & category) {
    for (size_t i = 0; i < players.size(); ++i) {
        for (size_t j = i; j < players.size(); ++j) {
            std::unique_ptr<CreateMatchAction> action = std::make_unique<CreateMatchAction>(tournament, category, players[0], players[1]);
            mMatchIds.push_back(action->getId());
            tournament->dispatchAction(tournament, std::move(action));
        }
    }
}

void PoolDrawStrategy::updateCategory(std::unique_ptr<TournamentStore> & tournament, std::unique_ptr<CategoryStore> & category) {
    // noop
}

bool PoolDrawStrategy::isFinished(std::unique_ptr<TournamentStore> & tournament, std::unique_ptr<CategoryStore> & category) const {
    std::unique_ptr<Ruleset> & ruleset = category->getRuleset();
    for (Id matchId : mMatchIds) {
        std::unique_ptr<MatchStore> & match = category->getMatch(matchId);
        if (!ruleset->isFinished(match))
            return false;
    }

    return true;
}

Id PoolDrawStrategy::get_rank(size_t rank) const {
    // TODO
}

