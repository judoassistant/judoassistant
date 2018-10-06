#include "actions/player_actions.hpp"
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

// DeletePlayerAction::DeletePlayerAction(TournamentStore & tournament, const Id & player)
//     : mId(id)
//     , mPlayer(nullptr)
// {}

// void DeletePlayerAction::redoImpl(TournamentStore & tournament) {
//     try {
//         mPlayer = tournament.erasePlayer(mId);

//         for (Id category : mPlayer->getCategories()) {
//             tournament.getCategory(category
//         }
//     }
//     catch (std::exception e) {
//         std::cout << e << std::endl;
//         throw ActionExecutionException("Failed to delete player");
//     }
// }

// void DeletePlayerAction::undoImpl(TournamentStore & tournament) {
//     tournament.addPlayer(std::move(mPlayer));
// }
