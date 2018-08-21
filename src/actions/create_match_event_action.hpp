#pragma once

#include "core.hpp"
#include "serialize.hpp"
#include "actions/action.hpp"
#include "stores/tournament_store.hpp"
#include "stores/category_store.hpp"
#include "stores/match_store.hpp"
#include "stores/match_event.hpp"

class CreateMatchEventAction : public Action {
public:
    CreateMatchEventAction(std::unique_ptr<CategoryStore> & category, std::unique_ptr<MatchStore> & match, std::unique_ptr<MatchEvent> && event);
    CreateMatchEventAction(Id category, Id match, std::unique_ptr<MatchEvent> && event);

    virtual bool operator()(std::unique_ptr<TournamentStore> & tournament) const;
    virtual std::unique_ptr<Action> getInverse() const;
private:
    Id mCategoryId;
    Id mMatchId;
    std::unique_ptr<MatchEvent> mEvent;
};
