#include "actions/match_event_actions.hpp"

// CreateMatchEventAction::CreateMatchEventAction(CategoryStore & category, MatchStore & match, std::unique_ptr<MatchEvent> && event)
//     : mCategoryId(category.getId())
//     , mMatchId(match.getId())
//     , mEvent(std::move(event))
// {}

// CreateMatchEventAction::CreateMatchEventAction(Id category, Id match, std::unique_ptr<MatchEvent> && event)
//     : mCategoryId(category)
//     , mMatchId(match)
//     , mEvent(std::move(event))
// {}

// bool CreateMatchEventAction::operator()(TournamentStore & tournament) const {
//     CategoryStore & category = tournament.getCategory(mCategoryId);
//     Ruleset & ruleset = category.getRuleset();
//     MatchStore & match = category.getMatch(mMatchId);
//     std::unique_ptr<MatchEvent> event = mEvent->clone();

//     (*event)(match, ruleset);
//     match.pushEvent(std::move(event));
//     tournament.matchChanged(match.getId());
//     return true;
// }

// std::unique_ptr<Action> CreateMatchEventAction::getInverse() const {
//     return std::make_unique<CreateMatchEventAction>(mCategoryId, mMatchId, mEvent->getInverse());
// }
