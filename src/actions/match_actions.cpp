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

        if (mWhitePlayer.has_value())
            tournament.getPlayer(mWhitePlayer.value()).addMatch(mId);
        if (mBluePlayer.has_value())
            tournament.getPlayer(mBluePlayer.value()).addMatch(mId);

        category.addMatch(std::make_unique<MatchStore>(mId, mCategory, mWhitePlayer, mBluePlayer));

        mIsDrawn = category.isDrawn();
        category.setIsDrawn(false);

        tournament.matchAdded(mId);
        tournament.categoryChanged(mId);
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to redo CreateMatchAction.");
    }
}

void CreateMatchAction::undoImpl(TournamentStore & tournament) {
    try {
        CategoryStore & category = tournament.getCategory(mCategory);

        if (mWhitePlayer.has_value())
            tournament.getPlayer(mWhitePlayer.value()).eraseMatch(mId);
        if (mBluePlayer.has_value())
            tournament.getPlayer(mBluePlayer.value()).eraseMatch(mId);

        category.eraseMatch(mId);
        tournament.matchDeleted(mId);

        category.setIsDrawn(mIsDrawn);

        tournament.matchDeleted(mId);
        tournament.categoryChanged(mId);
        // TODO: should playerChanged be called in cases like this?
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to undo CreateMatchAction.");
    }
}

Id CreateMatchAction::getId() const {
    return mId;
};

EraseMatchAction::EraseMatchAction(TournamentStore & tournament, Id category, Id id)
    : mCategory(category)
    , mId(id)
{}

void EraseMatchAction::redoImpl(TournamentStore & tournament) {
    try {
        CategoryStore & category = tournament.getCategory(mCategory);
        mMatch = category.eraseMatch(mId);

        std::optional<Id> whitePlayer = mMatch->getPlayer(MatchStore::PlayerIndex::WHITE);
        if (whitePlayer.has_value())
            tournament.getPlayer(whitePlayer.value()).eraseMatch(mId);
        std::optional<Id> bluePlayer = mMatch->getPlayer(MatchStore::PlayerIndex::BLUE);
        if (bluePlayer.has_value())
            tournament.getPlayer(bluePlayer.value()).eraseMatch(mId);

        mIsDrawn = category.isDrawn();
        category.setIsDrawn(false);

        tournament.matchDeleted(mId);
        tournament.categoryChanged(mId);
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to redo EraseMatchAction.");
    }
}

void EraseMatchAction::undoImpl(TournamentStore & tournament) {
    try {
        CategoryStore & category = tournament.getCategory(mCategory);

        std::optional<Id> whitePlayer = mMatch->getPlayer(MatchStore::PlayerIndex::WHITE);
        if (whitePlayer.has_value())
            tournament.getPlayer(whitePlayer.value()).addMatch(mId);
        std::optional<Id> bluePlayer = mMatch->getPlayer(MatchStore::PlayerIndex::BLUE);
        if (bluePlayer.has_value())
            tournament.getPlayer(bluePlayer.value()).addMatch(mId);

        category.addMatch(std::move(mMatch));

        category.setIsDrawn(mIsDrawn);

        tournament.matchAdded(mId);
        tournament.categoryChanged(mId);
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to undo EraseMatchAction.");
    }
}

