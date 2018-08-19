#include "actions/create_match_event_action.hpp"

CreateMatchEventAction::CreateMatchEventAction(std::unique_ptr<CategoryStore> & category, std::unique_ptr<MatchStore> & match, std::unique_ptr<MatchEvent> && event)
    : mCategoryId(category->getId())
    , mMatchId(match->getId())
    , mEvent(std::move(event))
{}

CreateMatchEventAction::CreateMatchEventAction(Id category, Id match, std::unique_ptr<MatchEvent> && event)
    : mCategoryId(category)
    , mMatchId(match)
    , mEvent(std::move(event))
{}

bool CreateMatchEventAction::operator()(std::unique_ptr<TournamentStore> & tournament) const {
    std::unique_ptr<CategoryStore> & category = tournament->getCategory(mCategoryId);
    std::unique_ptr<Ruleset> & ruleset = category->getRuleset();
    std::unique_ptr<MatchStore> & match = category->getMatch(mMatchId);
    std::unique_ptr<MatchEvent> event = mEvent->clone();

    (*event)(match, ruleset);
    match->pushEvent(std::move(event));
    return true;
}

std::unique_ptr<Action> CreateMatchEventAction::getInverse() const {
    return std::make_unique<CreateMatchEventAction>(mCategoryId, mMatchId, mEvent->getInverse());
}
