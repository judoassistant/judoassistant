#include "actions/category_actions.hpp"
#include "actions/match_actions.hpp"

#include "exception.hpp"

CreateCategoryAction::CreateCategoryAction(TournamentStore & tournament, const std::string &name, std::unique_ptr<Ruleset> ruleset, std::unique_ptr<DrawStrategy> drawStrategy)
    : mId(tournament.generateNextCategoryId())
    , mName(name)
    , mRuleset(std::move(ruleset))
    , mDrawStrategy(std::move(drawStrategy))
{}

void CreateCategoryAction::redoImpl(TournamentStore & tournament) {
    try {
        tournament.addCategory(std::make_unique<CategoryStore>(mId, mName, mRuleset->clone(), mDrawStrategy->clone()));
        tournament.categoryAdded(mId);
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to redo create category.");
    }
}

void CreateCategoryAction::undoImpl(TournamentStore & tournament) {
    try {
        tournament.eraseCategory(mId);
        tournament.categoryDeleted(mId);
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to undo create category.");
    }
}

AddPlayerToCategoryAction::AddPlayerToCategoryAction(TournamentStore & tournament, Id category, Id player)
    : mCategory(category)
    , mPlayer(player)
{}

void AddPlayerToCategoryAction::redoImpl(TournamentStore & tournament) {
    try {
        tournament.getPlayer(mPlayer).addCategory(mCategory);

        CategoryStore & category = tournament.getCategory(mCategory);
        category.addPlayer(mPlayer);
        mIsDrawn = category.isDrawn();
        category.setIsDrawn(false);
    }
    catch (const std::exception &e){
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to redo AddPlayerToCategoryAction.");
    }
}

void AddPlayerToCategoryAction::undoImpl(TournamentStore & tournament) {
    try {
        tournament.getPlayer(mPlayer).eraseCategory(mCategory);

        CategoryStore & category = tournament.getCategory(mCategory);
        category.erasePlayer(mPlayer);
        category.setIsDrawn(mIsDrawn);
    }
    catch (const std::exception &e){
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to undo AddPlayerToCategoryAction.");
    }
}

RemovePlayerFromCategoryAction::RemovePlayerFromCategoryAction(TournamentStore & tournament, Id category, Id player)
    : mCategory(category)
    , mPlayer(player)
{}

void RemovePlayerFromCategoryAction::redoImpl(TournamentStore & tournament) {
    try {
        // TODO: consider a way to block excessive calls to TournamentStore::categoryChanged etc.
        CategoryStore & category = tournament.getCategory(mCategory);
        PlayerStore & player = tournament.getPlayer(mPlayer);

        // Remove all the player's matches from the category
        auto &playerMatches = player.getMatches();
        for (const auto & it : category.getMatches()) {
            Id matchId = it.second->getId();
            if (playerMatches.find(matchId) == playerMatches.end())
                continue;

            EraseMatchAction action(tournament, mCategory, matchId);
            action.redo(tournament);
            mActions.push(std::move(action));
        }

        category.erasePlayer(mPlayer);
        player.eraseCategory(mCategory);
        tournament.categoryChanged(mCategory);
        tournament.playerChanged(mPlayer);
    }
    catch (const std::exception &e){
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to redo RemovePlayerFromCategoryAction.");
    }
}

void RemovePlayerFromCategoryAction::undoImpl(TournamentStore & tournament) {
    try {
        CategoryStore & category = tournament.getCategory(mCategory);
        PlayerStore & player = tournament.getPlayer(mPlayer);

        category.addPlayer(mPlayer);
        player.eraseCategory(mCategory);

        // undo the EraseMatchActions in reverse order
        while (!mActions.empty()) {
            mActions.top().undo(tournament);
            mActions.pop();
        }

        tournament.categoryChanged(mCategory);
        tournament.playerChanged(mPlayer);
    }
    catch (const std::exception &e){
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to undo RemovePlayerFromCategoryAction.");
    }
}

EraseCategoryAction::EraseCategoryAction(TournamentStore & tournament, Id category)
    : mId(category)
{}

void EraseCategoryAction::redoImpl(TournamentStore & tournament) {
    try {
        // Remove all the players from the category (and their matches)
        for (Id player : tournament.getCategory(mId).getPlayers()) {
            RemovePlayerFromCategoryAction action(tournament, mId, player);
            action.redo(tournament);
            mActions.push(std::move(action));
        }

        mCategory = tournament.eraseCategory(mId);
        tournament.categoryDeleted(mId);
    }
    catch (const std::exception &e){
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to redo EraseCategoryAction.");
    }
}

void EraseCategoryAction::undoImpl(TournamentStore & tournament) {
    try {
        tournament.addCategory(std::move(mCategory));

        // undo the RemovePlayerFromCategoryAction in reverse order
        while (!mActions.empty()) {
            mActions.top().undo(tournament);
            mActions.pop();
        }

        tournament.categoryAdded(mId);
    }
    catch (const std::exception &e){
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to undo EraseCategoryAction.");
    }
}
