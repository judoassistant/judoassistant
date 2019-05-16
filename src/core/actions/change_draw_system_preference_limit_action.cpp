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

    if (mRow == 0) // First limit should always be 1
        return;

    mPrevBelowLimits.push_back(systems[mRow].playerLowerLimit);
    systems[mRow].playerLowerLimit = mLimit;

    // Make sure limits stay sorted by capping other limits
    for (int i = static_cast<int>(mRow) - 1; i >= 0; --i) {
        if (systems[i].playerLowerLimit <= mLimit)
            break;
        mPrevAboveLimits.push_back(systems[i].playerLowerLimit);
        systems[i].playerLowerLimit = mLimit;
    }

    for (size_t i = mRow + 1; i < systems.size(); ++i) {
        if (systems[i].playerLowerLimit >= mLimit)
            break;
        mPrevBelowLimits.push_back(systems[i].playerLowerLimit);
        systems[i].playerLowerLimit = mLimit;
    }

    tournament.changePreferences();
}

void ChangeDrawSystemPreferenceLimitAction::undoImpl(TournamentStore & tournament) {
    auto &systems = tournament.getPreferences().getPreferredDrawSystems();

    if (mRow >= systems.size())
        return;

    if (mRow == 0) // First limit should always be 1
        return;

    for (int i = static_cast<int>(mRow)-1, j=0; j < static_cast<int>(mPrevAboveLimits.size()); --i, ++j)
        systems[i].playerLowerLimit = mPrevAboveLimits[j];

    for (size_t i = mRow, j = 0; j < mPrevBelowLimits.size(); ++i, ++j)
        systems[i].playerLowerLimit = mPrevBelowLimits[j];

    tournament.changePreferences();
}

std::unique_ptr<Action> ChangeDrawSystemPreferenceLimitAction::freshClone() const {
    return std::make_unique<ChangeDrawSystemPreferenceLimitAction>(mRow, mLimit);
}

std::string ChangeDrawSystemPreferenceLimitAction::getDescription() const {
    return "Change draw system preference lower player limit";
}

