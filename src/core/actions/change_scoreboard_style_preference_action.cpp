#include "core/actions/change_scoreboard_style_preference_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/preferences_store.hpp"

ChangeScoreboardStylePreferenceAction::ChangeScoreboardStylePreferenceAction(ScoreboardStylePreference value)
    : mValue(value)
{}

void ChangeScoreboardStylePreferenceAction::redoImpl(TournamentStore & tournament) {
    auto &preferences = tournament.getPreferences();

    mOldValue = preferences.getScoreboardStyle();
    if (mOldValue == mValue)
        return;

    preferences.setScoreboardStyle(mValue);

    tournament.changePreferences();
}

void ChangeScoreboardStylePreferenceAction::undoImpl(TournamentStore & tournament) {
    auto &preferences = tournament.getPreferences();

    auto value = preferences.getScoreboardStyle();
    if (value == mOldValue)
        return;

    preferences.setScoreboardStyle(mOldValue);

    tournament.changePreferences();
}

std::unique_ptr<Action> ChangeScoreboardStylePreferenceAction::freshClone() const {
    return std::make_unique<ChangeScoreboardStylePreferenceAction>(mValue);
}

std::string ChangeScoreboardStylePreferenceAction::getDescription() const {
    return "Change scoreboard style preference";
}

