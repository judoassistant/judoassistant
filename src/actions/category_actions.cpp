#include "actions/category_actions.hpp"
#include "exception.hpp"

CreateCategoryAction::CreateCategoryAction(TournamentStore & tournament, Id id, const std::string &name, std::unique_ptr<Ruleset> ruleset, std::unique_ptr<DrawStrategy> drawStrategy)
    : mId(tournament.generateNextCategoryId())
    , mName(name)
    , mRuleset(std::move(ruleset))
    , mDrawStrategy(std::move(drawStrategy))
{}

void CreateCategoryAction::redoImpl(TournamentStore & tournament) {
    try {
        // CategoryStore & category = tournament.getCategory(mCategory);
        // category.addCategory(std::make_unique<CategoryStore>(mId, mWhitePlayer, mBluePlayer));
        // tournament.matchAdded(mId);
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to redo create category.");
    }
}

void CreateCategoryAction::undoImpl(TournamentStore & tournament) {
    try {
        // CategoryStore & category = tournament.getCategory(mCategory);
        // category.eraseCategory(mId);
        // tournament.matchDeleted(mId);
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        throw ActionExecutionException("Failed to undo create category.");
    }
}

