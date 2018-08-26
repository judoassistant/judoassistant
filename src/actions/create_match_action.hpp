#pragma once

#include "core.hpp"
#include "serialize.hpp"
#include "actions/action.hpp"
#include "stores/tournament_store.hpp"
#include "stores/category_store.hpp"
#include "stores/player_store.hpp"

class CreateMatchAction : public Action {
public:
    CreateMatchAction(TournamentStore & tournament, CategoryStore & category, std::optional<Id> whitePlayer, std::optional<Id> bluePlayer);
    bool operator()(TournamentStore & tournament) const override;
    std::unique_ptr<Action> getInverse() const override;

    Id getId() const;
private:
    Id mId;
    Id mCategory;
    std::optional<Id> mWhitePlayer;
    std::optional<Id> mBluePlayer;
};
