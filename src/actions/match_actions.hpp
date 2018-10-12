#pragma once

#include "core.hpp"
#include "serialize.hpp"
#include "actions/action.hpp"
#include "stores/tournament_store.hpp"
#include "stores/category_store.hpp"
#include "stores/player_store.hpp"

class AddMatchAction : public Action {
public:
    AddMatchAction(TournamentStore & tournament, CategoryId categoryId, std::optional<PlayerId> whitePlayerId, std::optional<PlayerId> bluePlayerId);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    MatchId mId;
    CategoryId mCategoryId;
    std::optional<PlayerId> mWhitePlayerId;
    std::optional<PlayerId> mBluePlayerId;
};

