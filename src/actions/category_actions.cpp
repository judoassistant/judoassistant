#include "actions/category_actions.hpp"
#include "draw_systems/draw_systems.hpp"
#include "rulesets/rulesets.hpp"

#include "exception.hpp"

AddCategoryAction::AddCategoryAction(TournamentStore & tournament, const std::string &name, uint8_t ruleset, uint8_t drawSystem)
    : mId(tournament.generateNextCategoryId())
    , mName(name)
    , mRuleset(ruleset)
    , mDrawSystem(drawSystem)
{}

void AddCategoryAction::redoImpl(TournamentStore & tournament) {
    if (tournament.containsCategory(mId))
        throw ActionExecutionException("Failed to redo AddCategoryAction. Category already exists.");

    const auto &rulesets = Rulesets::getRulesets();
    if (mRuleset > rulesets.size())
        throw ActionExecutionException("Failed to redo AddCategoryAction. Invalid ruleset specified.");
    auto ruleset = rulesets[mRuleset]->clone();

    const auto &drawSystems = DrawSystems::getDrawSystems();
    if (mDrawSystem > drawSystems.size())
        throw ActionExecutionException("Failed to redo AddCategoryAction. Invalid drawSystem specified.");
    auto drawSystem = drawSystems[mDrawSystem]->clone();

    tournament.beginAddCategories({mId});
    tournament.addCategory(std::make_unique<CategoryStore>(mId, mName, std::move(ruleset), std::move(drawSystem)));
    tournament.endAddCategories();
}

void AddCategoryAction::undoImpl(TournamentStore & tournament) {
    tournament.beginEraseCategories({mId});
    tournament.eraseCategory(mId);
    tournament.endEraseCategories();
}

AddPlayersToCategoryAction::AddPlayersToCategoryAction(TournamentStore & tournament, CategoryId categoryId, std::vector<PlayerId> playerIds)
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
        mDrawAction = std::make_unique<DrawCategoryAction>(tournament, mCategoryId);
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

ErasePlayersFromCategoryAction::ErasePlayersFromCategoryAction(TournamentStore & tournament, CategoryId categoryId, std::vector<PlayerId> playerIds)
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
        mDrawAction = std::make_unique<DrawCategoryAction>(tournament, mCategoryId);
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

EraseCategoriesAction::EraseCategoriesAction(TournamentStore & tournament, std::vector<CategoryId> categoryIds)
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

        for (const std::unique_ptr<MatchStore> &match : category.getMatches()) {
            auto whitePlayerId = match->getPlayer(MatchStore::PlayerIndex::WHITE);
            if (whitePlayerId)
                tournament.getPlayer(*whitePlayerId).eraseMatch(categoryId, match->getId());

            auto bluePlayerId = match->getPlayer(MatchStore::PlayerIndex::BLUE);
            if (bluePlayerId)
                tournament.getPlayer(*bluePlayerId).eraseMatch(categoryId, match->getId());
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

        for (const std::unique_ptr<MatchStore> &match : category->getMatches()) {
            auto whitePlayerId = match->getPlayer(MatchStore::PlayerIndex::WHITE);
            if (whitePlayerId)
                tournament.getPlayer(*whitePlayerId).addMatch(category->getId(), match->getId());

            auto bluePlayerId = match->getPlayer(MatchStore::PlayerIndex::BLUE);
            if (bluePlayerId)
                tournament.getPlayer(*bluePlayerId).addMatch(category->getId(), match->getId());
        }

        tournament.addCategory(std::move(category));
        mCategories.pop();
    }

    tournament.endAddCategories();
}

DrawCategoryAction::DrawCategoryAction(TournamentStore & tournament, CategoryId categoryId)
    : mCategoryId(categoryId)
{}

void DrawCategoryAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;

    // Delete all existing matches
    tournament.beginResetMatches(mCategoryId);
    CategoryStore & category = tournament.getCategory(mCategoryId);

    for (const std::unique_ptr<MatchStore> &match : category.getMatches()) {
        std::optional<PlayerId> whitePlayer = match->getPlayer(MatchStore::PlayerIndex::WHITE);
        if (whitePlayer && tournament.containsPlayer(*whitePlayer))
            tournament.getPlayer(*whitePlayer).eraseMatch(mCategoryId, match->getId());

        std::optional<PlayerId> bluePlayer = match->getPlayer(MatchStore::PlayerIndex::BLUE);
        if (bluePlayer && tournament.containsPlayer(*bluePlayer))
            tournament.getPlayer(*bluePlayer).eraseMatch(mCategoryId, match->getId());
    }

    mOldMatches = std::move(category.clearMatches());
    mOldDrawSystem = category.getDrawSystem().clone();

    std::vector<PlayerId> playerIds(category.getPlayers().begin(), category.getPlayers().end());
    std::vector<std::unique_ptr<Action>> actions = category.getDrawSystem().initCategory(playerIds, tournament, category);

    for (size_t i = 0; i < actions.size(); ++i) {
        actions[i]->redo(tournament);
        mActions.push(std::move(actions[i]));
    }

    tournament.endResetMatches(mCategoryId);

    for (MatchType type : {MatchType::FINAL, MatchType::NORMAL}) {
        std::optional<TatamiLocation> location = category.getTatamiLocation(type);
        if (location)
            tournament.getTatamis().recomputeBlock(tournament, *location);
    }
}

void DrawCategoryAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;

    tournament.beginResetMatches(mCategoryId);
    CategoryStore & category = tournament.getCategory(mCategoryId);

    while (!mActions.empty()) {
        mActions.top()->undo(tournament);
        mActions.pop();
    }

    category.setDrawSystem(std::move(mOldDrawSystem));

    for (std::unique_ptr<MatchStore> & match : mOldMatches) {
        std::optional<PlayerId> whitePlayer = match->getPlayer(MatchStore::PlayerIndex::WHITE);
        if (whitePlayer && tournament.containsPlayer(*whitePlayer))
            tournament.getPlayer(*whitePlayer).eraseMatch(mCategoryId, match->getId());

        std::optional<PlayerId> bluePlayer = match->getPlayer(MatchStore::PlayerIndex::BLUE);
        if (bluePlayer && tournament.containsPlayer(*bluePlayer))
            tournament.getPlayer(*bluePlayer).eraseMatch(mCategoryId, match->getId());

        category.pushMatch(std::move(match));
    }

    mOldMatches.clear();

    tournament.endResetMatches(mCategoryId);

    for (MatchType type : {MatchType::FINAL, MatchType::NORMAL}) {
        std::optional<TatamiLocation> location = category.getTatamiLocation(type);
        if (location)
            tournament.getTatamis().recomputeBlock(tournament, *location);
    }
}

ErasePlayersFromAllCategoriesAction::ErasePlayersFromAllCategoriesAction(TournamentStore & tournament, std::vector<PlayerId> playerIds)
    : mPlayerIds(playerIds)
{}

void ErasePlayersFromAllCategoriesAction::redoImpl(TournamentStore & tournament) {
    std::vector<PlayerId> playerIds;
    std::unordered_set<CategoryId> categoryIds;

    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId)) continue;

        playerIds.push_back(playerId);
        const PlayerStore &player = tournament.getPlayer(playerId);
        const auto &playerCategories = player.getCategories();
        categoryIds.insert(playerCategories.begin(), playerCategories.end());
    }

    for (auto categoryId : categoryIds) {
        auto action = std::make_unique<ErasePlayersFromCategoryAction>(tournament, categoryId, playerIds);
        action->redo(tournament);
        mActions.push(std::move(action));
    }
}

void ErasePlayersFromAllCategoriesAction::undoImpl(TournamentStore & tournament) {
    while (!mActions.empty()) {
        mActions.top()->undo(tournament);
        mActions.pop();
    }
}

