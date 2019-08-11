#include "core/actions/draw_categories_action.hpp"
#include "core/actions/erase_players_action.hpp"
#include "core/actions/erase_players_from_category_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/tournament_store.hpp"

ErasePlayersAction::ErasePlayersAction(const std::vector<PlayerId> &playerIds)
    : ErasePlayersAction(playerIds, getSeed())
{}

ErasePlayersAction::ErasePlayersAction(const std::vector<PlayerId> &playerIds, unsigned int seed)
    : mPlayerIds(playerIds)
    , mSeed(seed)
{}

std::unique_ptr<Action> ErasePlayersAction::freshClone() const {
    return std::make_unique<ErasePlayersAction>(mPlayerIds, mSeed);
}

void ErasePlayersAction::redoImpl(TournamentStore & tournament) {
    std::unordered_set<CategoryId> categoryIds;

    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId)) continue;

        mErasedPlayerIds.push_back(playerId);

        const PlayerStore &player = tournament.getPlayer(playerId);
        categoryIds.insert(player.getCategories().begin(), player.getCategories().end());
    }

    tournament.beginErasePlayers(mErasedPlayerIds);
    for (auto categoryId : categoryIds) {
        auto action = std::make_unique<ErasePlayersFromCategoryAction>(categoryId, mErasedPlayerIds, mSeed); // lazily give the action all playerIds and let it figure the rest out on its own
        action->redo(tournament);
        mActions.push(std::move(action));
    }

    for (auto playerId : mErasedPlayerIds)
        mPlayers.push(tournament.erasePlayer(playerId));
    tournament.endErasePlayers(mErasedPlayerIds);
}

void ErasePlayersAction::undoImpl(TournamentStore & tournament) {
    tournament.beginAddPlayers(mErasedPlayerIds);
    while (!mPlayers.empty()) {
        tournament.addPlayer(std::move(mPlayers.top()));
        mPlayers.pop();
    }
    tournament.endAddPlayers(mErasedPlayerIds);
    mErasedPlayerIds.clear();

    while (!mActions.empty()) {
        mActions.top()->undo(tournament);
        mActions.pop();
    }
}

std::string ErasePlayersAction::getDescription() const {
    return "Erase players";
}

