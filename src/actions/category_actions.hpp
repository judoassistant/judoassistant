#pragma once

#include "core.hpp"
#include "serialize.hpp"
#include "actions/action.hpp"
#include "actions/match_actions.hpp"
#include "stores/tournament_store.hpp"
#include "stores/category_store.hpp"
#include "stores/player_store.hpp"

class CreateCategoryAction : public Action {
public:
    CreateCategoryAction(TournamentStore & tournament, const std::string &name, std::unique_ptr<Ruleset> ruleset, std::unique_ptr<DrawStrategy> drawStrategy);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    Id mId;
    std::string mName;
    std::unique_ptr<Ruleset> mRuleset;
    std::unique_ptr<DrawStrategy> mDrawStrategy;
};

class AddPlayerToCategoryAction : public Action {
public:
    AddPlayerToCategoryAction(TournamentStore & tournament, Id category, Id player);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    Id mCategory;
    Id mPlayer;
    bool mIsDrawn;
};

class EraseMatchAction;

class RemovePlayerFromCategoryAction : public Action {
public:
    RemovePlayerFromCategoryAction(TournamentStore & tournament, Id category, Id player);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    Id mCategory;
    Id mPlayer;
    std::stack<EraseMatchAction> mActions;
};

class EraseCategoryAction : public Action {
public:
    EraseCategoryAction(TournamentStore & tournament, Id category);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    Id mId;
    std::stack<RemovePlayerFromCategoryAction> mActions;
    std::unique_ptr<CategoryStore> mCategory;
};
