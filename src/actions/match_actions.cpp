#include "actions/match_actions.hpp"
#include "exception.hpp"

CreateMatchAction::CreateMatchAction(TournamentStore & tournament, CategoryStore & category, std::optional<Id> whitePlayer, std::optional<Id> bluePlayer)
    : mId(tournament.generateNextMatchId())
    , mCategory(category.getId())
    , mWhitePlayer(whitePlayer)
    , mBluePlayer(bluePlayer)
{

}

void CreateMatchAction::redoImpl(TournamentStore & tournament) {
    try {
        CategoryStore & category = tournament.getCategory(mCategory);
        category.addMatch(std::make_unique<MatchStore>(mId, mWhitePlayer, mBluePlayer));
        tournament.matchAdded(mId);
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to redo create match.");
    }
}

void CreateMatchAction::undoImpl(TournamentStore & tournament) {
    try {
        CategoryStore & category = tournament.getCategory(mCategory);
        category.eraseMatch(mId);
        tournament.matchDeleted(mId);
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to undo create match.");
    }
}

Id CreateMatchAction::getId() const {
    return mId;
};
