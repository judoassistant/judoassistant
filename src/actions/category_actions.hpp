#pragma once

#include "core.hpp"
#include "serialize.hpp"
#include "actions/action.hpp"
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

class RemovePlayerFromCategoryAction : public Action {
public:
    RemovePlayerFromCategoryAction(TournamentStore & tournament, Id category, Id player);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    Id mCategory;
    Id mPlayer;
    bool mIsDrawn;
    std::vector<std::unique_ptr<MatchStore>> mMatches;
};
