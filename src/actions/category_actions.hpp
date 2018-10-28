#pragma once

#include "core.hpp"
#include "serialize.hpp"
#include "actions/action.hpp"
#include "actions/match_actions.hpp"
#include "stores/tournament_store.hpp"
#include "stores/category_store.hpp"
#include "stores/player_store.hpp"

class AddCategoryAction : public Action {
public:
    AddCategoryAction(TournamentStore & tournament, const std::string &name, uint8_t ruleset, uint8_t drawSystem);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
    CategoryId getId() const;
private:
    CategoryId mId;
    std::string mName;
    uint8_t mRuleset;
    uint8_t mDrawSystem;
};

class DrawCategoryAction : public Action {
public:
    DrawCategoryAction(TournamentStore & tournament, CategoryId categoryId);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    CategoryId mCategoryId;

    // undo members
    std::vector<std::unique_ptr<MatchStore>> mOldMatches;
    std::stack<std::unique_ptr<Action>> mActions;
    std::unique_ptr<DrawSystem> mOldDrawSystem;
};

class AddPlayersToCategoryAction : public Action {
public:
    AddPlayersToCategoryAction(TournamentStore & tournament, CategoryId categoryId, std::vector<PlayerId> playerIds);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    CategoryId mCategoryId;
    std::vector<PlayerId> mPlayerIds;

    // undo members
    std::vector<PlayerId> mAddedPlayerIds;
    std::unique_ptr<DrawCategoryAction> mDrawAction;
};

class ErasePlayersFromCategoryAction : public Action {
public:
    ErasePlayersFromCategoryAction(TournamentStore & tournament, CategoryId categoryId, std::vector<PlayerId> playerIds);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    CategoryId mCategoryId;
    std::vector<PlayerId> mPlayerIds;

    // undo members
    std::vector<PlayerId> mErasedPlayerIds;
    std::unique_ptr<DrawCategoryAction> mDrawAction;
};

class EraseCategoriesAction : public Action {
public:
    EraseCategoriesAction(TournamentStore & tournament, std::vector<CategoryId> categoryIds);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    std::vector<CategoryId> mCategoryIds;

    // undo members
    std::vector<CategoryId> mErasedCategoryIds;
    std::stack<std::unique_ptr<CategoryStore>> mCategories;
};

class ErasePlayersFromAllCategoriesAction : public Action {
public:
    ErasePlayersFromAllCategoriesAction(TournamentStore & tournament, std::vector<PlayerId> playerIds);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    std::vector<PlayerId> mPlayerIds;

    // undo members
    std::stack<std::unique_ptr<ErasePlayersFromCategoryAction>> mActions;
};

class AutoAddCategoriesAction : public Action {
public:
    AutoAddCategoriesAction(TournamentStore &tournament, std::vector<PlayerId> playerIds, std::string baseName, float maxDifference, size_t maxSize);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

private:
    std::vector<std::vector<PlayerId>> mPlayerIds;
    std::vector<CategoryId> mCategoryIds;
    std::string mBaseName;
};
