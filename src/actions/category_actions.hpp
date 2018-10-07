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
    AddCategoryAction(TournamentStore & tournament, const std::string &name, std::unique_ptr<Ruleset> ruleset, std::unique_ptr<DrawStrategy> drawStrategy);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    Id mId;
    std::string mName;
    std::unique_ptr<Ruleset> mRuleset;
    std::unique_ptr<DrawStrategy> mDrawStrategy;
};

class DrawCategoryAction : public Action {
public:
    DrawCategoryAction(TournamentStore & tournament, Id category);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    Id mCategoryId;

    // undo members
    std::unique_ptr<MatchStore> mOldMatches;
};

class AddPlayersToCategoryAction : public Action {
public:
    AddPlayersToCategoryAction(TournamentStore & tournament, Id categoryId, std::vector<Id> playerIds);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    Id mCategoryId;
    std::vector<Id> mPlayerIds;

    // undo members
    std::vector<Id> mAddedPlayerIds;
    std::unique_ptr<DrawCategoryAction> mDrawAction;
};

class ErasePlayersFromCategoryAction : public Action {
public:
    ErasePlayersFromCategoryAction(TournamentStore & tournament, Id categoryId, std::vector<Id> playerIds);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    Id mCategoryId;
    std::vector<Id> mPlayerIds;

    // undo members
    std::vector<Id> mErasedPlayerIds;
    std::unique_ptr<DrawCategoryAction> mDrawAction;
};

class EraseCategoriesAction : public Action {
public:
    EraseCategoriesAction(TournamentStore & tournament, std::vector<Id> categoryIds);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    std::vector<Id> mCategoryIds;

    // undo members
    std::vector<Id> mErasedCategoryIds;
    std::stack<std::unique_ptr<CategoryStore>> mCategories;
};
