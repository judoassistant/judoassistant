#include "actions/category_actions.hpp"

#include "exception.hpp"

AddCategoryAction::AddCategoryAction(TournamentStore & tournament, const std::string &name, std::unique_ptr<Ruleset> ruleset, std::unique_ptr<DrawStrategy> drawStrategy)
    : mId(tournament.generateNextCategoryId())
    , mName(name)
    , mRuleset(std::move(ruleset))
    , mDrawStrategy(std::move(drawStrategy))
{}

void AddCategoryAction::redoImpl(TournamentStore & tournament) {
    if (tournament.containsCategory(mId))
        throw ActionExecutionException("Failed to redo AddCategoryAction. Category already exists.");
    tournament.beginAddCategories({mId});
    tournament.addCategory(std::make_unique<CategoryStore>(mId, mName, mRuleset->clone(), mDrawStrategy->clone()));
    tournament.endAddCategories();
}

void AddCategoryAction::undoImpl(TournamentStore & tournament) {
    tournament.beginEraseCategories({mId});
    tournament.eraseCategory(mId);
    tournament.endEraseCategories();
}

AddPlayersToCategoryAction::AddPlayersToCategoryAction(TournamentStore & tournament, Id categoryId, std::vector<Id> playerIds)
    : mCategoryId(categoryId)
    , mPlayerIds(playerIds)
{}

void AddPlayersToCategoryAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;
    CategoryStore & category = tournament.getCategory(mCategoryId);

    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId)) continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        if (player.containsCategory(mCategoryId))
            continue;

        mAddedPlayerIds.push_back(playerId);
        player.addCategory(mCategoryId);
        category.addPlayer(playerId);
    }

    tournament.addPlayersToCategory(mCategoryId, mAddedPlayerIds);

    if (!mAddedPlayerIds.empty()) {
        auto mDrawAction = std::make_unique<DrawCategoryAction>(tournament, mCategoryId);
        mDrawAction->redo(tournament);
    }
}

void AddPlayersToCategoryAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;

    tournament.erasePlayersFromCategory(mCategoryId, mAddedPlayerIds);

    CategoryStore & category = tournament.getCategory(mCategoryId);
    for (auto playerId : mAddedPlayerIds) {
        PlayerStore & player = tournament.getPlayer(playerId);
        player.eraseCategory(mCategoryId);
        category.erasePlayer(playerId);
    }

    if (mDrawAction != nullptr) {
        mDrawAction->undo(tournament);
        mDrawAction.reset();
    }

    mAddedPlayerIds.clear();
}

ErasePlayersFromCategoryAction::ErasePlayersFromCategoryAction(TournamentStore & tournament, Id categoryId, std::vector<Id> playerIds)
    : mCategoryId(categoryId)
    , mPlayerIds(playerIds)
{}

void ErasePlayersFromCategoryAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;

    CategoryStore & category = tournament.getCategory(mCategoryId);

    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId)) continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        if (!player.containsCategory(mCategoryId))
            continue;

        mErasedPlayerIds.push_back(playerId);
        player.eraseCategory(mCategoryId);
        category.erasePlayer(playerId);
    }

    tournament.erasePlayersFromCategory(mCategoryId, mErasedPlayerIds);

    if (!mErasedPlayerIds.empty()) {
        auto mDrawAction = std::make_unique<DrawCategoryAction>(tournament, mCategoryId);
        mDrawAction->redo(tournament);
    }
}

void ErasePlayersFromCategoryAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;

    CategoryStore & category = tournament.getCategory(mCategoryId);

    tournament.addPlayersToCategory(mCategoryId, mErasedPlayerIds);

    for (auto playerId : mErasedPlayerIds) {
        PlayerStore & player = tournament.getPlayer(playerId);
        player.addCategory(mCategoryId);
        category.addPlayer(playerId);
    }

    if (mDrawAction != nullptr) {
        mDrawAction->undo(tournament);
        mDrawAction.reset();
    }

    mErasedPlayerIds.clear();
}

EraseCategoriesAction::EraseCategoriesAction(TournamentStore & tournament, std::vector<Id> categoryIds)
    : mCategoryIds(categoryIds)
{}

void EraseCategoriesAction::redoImpl(TournamentStore & tournament) {
    for (auto categoryId : mCategoryIds) {
        if (!tournament.containsCategory(categoryId))
            continue;

        mErasedCategoryIds.push_back(categoryId);
    }

    tournament.beginEraseCategories(mErasedCategoryIds);

    for (auto categoryId : mErasedCategoryIds) {
        CategoryStore & category = tournament.getCategory(categoryId);
        for (auto playerId : category.getPlayers()) {
            PlayerStore & player = tournament.getPlayer(playerId);
            player.eraseCategory(categoryId);
        }

        for (auto & it : category.getMatches()) {
            const std::unique_ptr<MatchStore> & match = it.second;
            auto whitePlayerId = match->getPlayer(MatchStore::PlayerIndex::WHITE);
            if (whitePlayerId)
                tournament.getPlayer(*whitePlayerId).eraseMatch(match->getId());

            auto bluePlayerId = match->getPlayer(MatchStore::PlayerIndex::BLUE);
            if (bluePlayerId)
                tournament.getPlayer(*bluePlayerId).eraseMatch(match->getId());
        }

        mCategories.push(tournament.eraseCategory(categoryId));
    }

    tournament.endEraseCategories();
}

void EraseCategoriesAction::undoImpl(TournamentStore & tournament) {
    tournament.beginAddCategories(mErasedCategoryIds);

    while (!mCategories.empty()) {
        std::unique_ptr<CategoryStore> category = std::move(mCategories.top());

        for (auto playerId : category->getPlayers()) {
            PlayerStore & player = tournament.getPlayer(playerId);
            player.addCategory(category->getId());
        }

        for (auto & it : category->getMatches()) {
            const std::unique_ptr<MatchStore> & match = it.second;

            auto whitePlayerId = match->getPlayer(MatchStore::PlayerIndex::WHITE);
            if (whitePlayerId)
                tournament.getPlayer(*whitePlayerId).addMatch(match->getId());

            auto bluePlayerId = match->getPlayer(MatchStore::PlayerIndex::BLUE);
            if (bluePlayerId)
                tournament.getPlayer(*bluePlayerId).addMatch(match->getId());
        }

        tournament.addCategory(std::move(category));
        mCategories.pop();
    }

    tournament.endAddCategories();
}

DrawCategoryAction::DrawCategoryAction(TournamentStore & tournament, Id category) {
    // TODO: Implement
    // Notes: category may contain invalid matches. Remember to erase matches from players and the category
}

void DrawCategoryAction::redoImpl(TournamentStore & tournament) {
    // TODO: Implement. Remember to check for empty category.
}

void DrawCategoryAction::undoImpl(TournamentStore & tournament) {
    // TODO: Implement. Remember to check for empty category
}
