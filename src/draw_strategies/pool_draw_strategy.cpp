#include "src/draw_strategies/pool_draw_strategy.hpp"
#include "src/actions/create_match_action.hpp"
#include "src/store_handlers/store_handler.hpp"

void PoolDrawStrategy::initCategory(StoreHandler & store_handler, const std::vector<Id> &players, TournamentStore & tournament, CategoryStore & category) {
    for (size_t i = 0; i < players.size(); ++i) {
        for (size_t j = i; j < players.size(); ++j) {
            std::unique_ptr<CreateMatchAction> action = std::make_unique<CreateMatchAction>(tournament, category, players[0], players[1]);
            mMatchIds.push_back(action->getId());
            store_handler.dispatch(std::move(action));
        }
    }
}

void PoolDrawStrategy::updateCategory(StoreHandler & store_handler, TournamentStore & tournament, CategoryStore & category) {
    // noop
}

bool PoolDrawStrategy::isFinished(TournamentStore & tournament, CategoryStore & category) const {
    Ruleset & ruleset = category.getRuleset();
    for (Id matchId : mMatchIds) {
        MatchStore & match = category.getMatch(matchId);
        if (!ruleset.isFinished(match))
            return false;
    }

    return true;
}

Id PoolDrawStrategy::get_rank(size_t rank) const {
    // TODO
}

