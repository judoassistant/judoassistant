#include "actions/category_actions.hpp"
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

// void RemovePlayerFromCategoryAction::redoImpl(TournamentStore & tournament) {
//     try {
//         tournament.getPlayer(mPlayer).addCategory(mCategory);

//         CategoryStore & category = tournament.getCategory(mCategory);
//         category.addPlayer(mPlayer);
//         mIsDrawn = category.isDrawn();
//         category.setIsDrawn(false);

//         // TODO: remove matches
//     }
//     catch (const std::exception &e){
//         std::cout << e.what() << std::endl;
//         throw ActionExecutionException("Failed to redo AddPlayerToCategoryAction.");
//     }
// }

// void RemovePlayerFromCategoryAction::undoImpl(TournamentStore & tournament) {
//     try {
//         tournament.getPlayer(mPlayer).eraseCategory(mCategory);

//         CategoryStore & category = tournament.getCategory(mCategory);
//         category.erasePlayer(mPlayer);
//         category.setIsDrawn(mIsDrawn);
//     }
//     catch (const std::exception &e){
//         std::cout << e.what() << std::endl;
//         throw ActionExecutionException("Failed to undo AddPlayerToCategoryAction.");
//     }
// }
