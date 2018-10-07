#include "actions/player_actions.hpp"
#include "actions/category_actions.hpp"
#include "actions/match_actions.hpp"
#include "stores/tournament_store.hpp"
#include "exception.hpp"

CreatePlayerAction::CreatePlayerAction(TournamentStore & tournament, const std::string & firstName, const std::string & lastName, std::optional<uint8_t> age, std::optional<PlayerRank> rank, const std::string &club, std::optional<float> weight, std::optional<PlayerCountry> country)
    : mId(tournament.generateNextPlayerId())
    , mFirstName(firstName)
    , mLastName(lastName)
    , mAge(age)
    , mRank(rank)
    , mClub(club)
    , mWeight(weight)
    , mCountry(country)
{}

void CreatePlayerAction::redoImpl(TournamentStore & tournament) {
    try {
        tournament.addPlayer(std::make_unique<PlayerStore>(mId, mFirstName, mLastName, mAge, mRank, mClub, mWeight, mCountry));
        tournament.playerAdded(mId);
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to redo create player.");
    }
}

void CreatePlayerAction::undoImpl(TournamentStore & tournament) {
    try {
        tournament.erasePlayer(mId);
        tournament.playerDeleted(mId);
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to undo create player.");
    }
}

ErasePlayerAction::ErasePlayerAction(TournamentStore & tournament, Id player)
    : mId(player)
{}

void ErasePlayerAction::redoImpl(TournamentStore & tournament) {
    try {
        // Remove the player all their categories (and their matches)
        for (Id category : tournament.getPlayer(mId).getCategories()) {
            RemovePlayerFromCategoryAction action(tournament, category, mId);
            action.redo(tournament);
            mActions.push(std::move(action));
        }

        mPlayer = tournament.erasePlayer(mId);
        tournament.playerDeleted(mId);
    }
    catch (const std::exception &e){
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to redo ErasePlayerAction.");
    }
}

void ErasePlayerAction::undoImpl(TournamentStore & tournament) {
    try {
        tournament.addPlayer(std::move(mPlayer));

        // undo the RemovePlayerFromCategoryAction in reverse order
        while (!mActions.empty()) {
            mActions.top().undo(tournament);
            mActions.pop();
        }

        tournament.playerAdded(mId);
    }
    catch (const std::exception &e){
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to undo ErasePlayerAction.");
    }
}

