#pragma once

#include "core.hpp"
#include "serialize.hpp"
#include "actions/action.hpp"
#include "stores/tournament_store.hpp"
#include "stores/category_store.hpp"
#include "stores/player_store.hpp"

class AddMatchAction : public Action {
public:
    AddMatchAction(TournamentStore & tournament, Id categoryId, std::optional<Id> whitePlayerId, std::optional<Id> bluePlayerId);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    Id mId;
    Id mCategoryId;
    std::optional<Id> mWhitePlayerId;
    std::optional<Id> mBluePlayerId;
};

