#pragma once

#include "core.hpp"
#include "serialize.hpp"
#include "actions/action.hpp"
#include "stores/tournament_store.hpp"
#include "stores/category_store.hpp"
#include "stores/player_store.hpp"

// TODO: Consider renaming Create(*)Action to Add(*)Action
class CreateMatchAction : public Action {
public:
    CreateMatchAction(TournamentStore & tournament, CategoryStore & category, std::optional<Id> whitePlayer, std::optional<Id> bluePlayer);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
    Id getId() const;
private:
    Id mId;
    Id mCategory;
    std::optional<Id> mWhitePlayer;
    std::optional<Id> mBluePlayer;

    bool mIsDrawn;
};

class EraseMatchAction : public Action {
public:
    // TODO: Make actions use Ids or optional consistly
    EraseMatchAction(TournamentStore & tournament, Id category, Id match);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    Id mCategory;
    Id mId;

    bool mIsDrawn;
    std::unique_ptr<MatchStore> mMatch;
};
