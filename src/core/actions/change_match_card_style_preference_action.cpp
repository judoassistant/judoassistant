#include "core/actions/change_match_card_style_preference_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/preferences_store.hpp"

ChangeMatchCardStylePreferenceAction::ChangeMatchCardStylePreferenceAction(MatchCardStylePreference value)
    : mValue(value)
{}

void ChangeMatchCardStylePreferenceAction::redoImpl(TournamentStore & tournament) {
    auto &preferences = tournament.getPreferences();

    mOldValue = preferences.getMatchCardStyle();
    if (mOldValue == mValue)
        return;

    preferences.setMatchCardStyle(mValue);

    tournament.changePreferences();
}

void ChangeMatchCardStylePreferenceAction::undoImpl(TournamentStore & tournament) {
    auto &preferences = tournament.getPreferences();

    auto value = preferences.getMatchCardStyle();
    if (value == mOldValue)
        return;

    preferences.setMatchCardStyle(mOldValue);

    tournament.changePreferences();
}

std::unique_ptr<Action> ChangeMatchCardStylePreferenceAction::freshClone() const {
    return std::make_unique<ChangeMatchCardStylePreferenceAction>(mValue);
}

std::string ChangeMatchCardStylePreferenceAction::getDescription() const {
    return "Change match card style preference";
}

