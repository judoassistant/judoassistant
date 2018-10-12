#include "actions/player_actions.hpp"
#include "actions/category_actions.hpp"
#include "actions/match_actions.hpp"
#include "stores/tournament_store.hpp"
#include "exception.hpp"

AddPlayerAction::AddPlayerAction(TournamentStore & tournament, const std::string & firstName, const std::string & lastName, std::optional<uint8_t> age, std::optional<PlayerRank> rank, const std::string &club, std::optional<float> weight, std::optional<PlayerCountry> country)
    : mId(tournament.generateNextPlayerId())
    , mFirstName(firstName)
    , mLastName(lastName)
    , mAge(age)
    , mRank(rank)
    , mClub(club)
    , mWeight(weight)
    , mCountry(country)
{}

void AddPlayerAction::redoImpl(TournamentStore & tournament) {
    if (tournament.containsPlayer(mId))
        throw ActionExecutionException("Failed to redo AddPlayerAction. Player already exists.");

    tournament.beginAddPlayers({mId});
    tournament.addPlayer(std::make_unique<PlayerStore>(mId, mFirstName, mLastName, mAge, mRank, mClub, mWeight, mCountry));
    tournament.endAddPlayers();
}

void AddPlayerAction::undoImpl(TournamentStore & tournament) {
    tournament.beginErasePlayers({mId});
    tournament.erasePlayer(mId);
    tournament.endErasePlayers();
}

ErasePlayersAction::ErasePlayersAction(TournamentStore & tournament, std::vector<PlayerId> playerIds)
    : mPlayerIds(playerIds) // TODO: Use std::move where appropriate
{}

void ErasePlayersAction::redoImpl(TournamentStore & tournament) {
    std::unordered_set<CategoryId, CategoryId::Hasher> categoryIds;

    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId)) continue;

        mErasedPlayerIds.push_back(playerId);

        const PlayerStore &player = tournament.getPlayer(playerId);
        categoryIds.insert(player.getCategories().begin(), player.getCategories().end());
    }

    tournament.beginErasePlayers(mErasedPlayerIds);
    for (auto categoryId : categoryIds) {
        auto action = std::make_unique<ErasePlayersFromCategoryAction>(tournament, categoryId, mErasedPlayerIds); // lazily give the action all playerIds and let it figure the rest out on its own
        action->redo(tournament);
        mActions.push(std::move(action));
    }


    for (auto playerId : mErasedPlayerIds)
        mPlayers.push(std::move(tournament.erasePlayer(playerId)));
    tournament.endErasePlayers();
}

void ErasePlayersAction::undoImpl(TournamentStore & tournament) {
    tournament.beginAddPlayers(mErasedPlayerIds);
    while (!mPlayers.empty()) {
        tournament.addPlayer(std::move(mPlayers.top()));
        mPlayers.pop();
    }
    tournament.endAddPlayers();
    mErasedPlayerIds.clear();

    while (!mActions.empty()) {
        mActions.top()->undo(tournament);
        mActions.pop();
    }
}
