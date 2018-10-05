#include "actions/player_actions.hpp"
#include "stores/tournament_store.hpp"
#include "exception.hpp"

CreatePlayerAction::CreatePlayerAction(TournamentStore & tournament, const std::string &firstName, const std::string &lastName, uint8_t age)
    : mId(tournament.generateNextPlayerId())
    , mFirstName(firstName)
    , mLastName(lastName)
    , mAge(age)
{}

void CreatePlayerAction::redoImpl(TournamentStore & tournament) {
    try {
        tournament.addPlayer(std::make_unique<PlayerStore>(mId, mFirstName, mLastName, mAge));
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
