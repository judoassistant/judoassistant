#include "core/actions/change_draw_system_preference_limit_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/preferences_store.hpp"

ChangeDrawSystemPreferenceLimitAction::ChangeDrawSystemPreferenceLimitAction(std::size_t row, std::size_t limit)
    : mRow(row)
    , mLimit(limit)
{}

void ChangeDrawSystemPreferenceLimitAction::redoImpl(TournamentStore & tournament) {
    auto &systems = tournament.getPreferences().getPreferredDrawSystems();

    if (mRow >= systems.size())
        return;
    mPrevLimit = systems[mRow].playerLowerLimit;
    systems[mRow].playerLowerLimit = mLimit;

    tournament.changePreferences();
}

void ChangeDrawSystemPreferenceLimitAction::undoImpl(TournamentStore & tournament) {
    auto &systems = tournament.getPreferences().getPreferredDrawSystems();

    if (mRow >= systems.size())
        return;
    systems[mRow].playerLowerLimit = mPrevLimit;

    tournament.changePreferences();
}

std::unique_ptr<Action> ChangeDrawSystemPreferenceLimitAction::freshClone() const {
    return std::make_unique<ChangeDrawSystemPreferenceLimitAction>(mRow, mLimit);
}

std::string ChangeDrawSystemPreferenceLimitAction::getDescription() const {
    return "Change draw system preference lower player limit";
}

