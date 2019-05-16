#include "core/actions/erase_draw_system_preference_row.hpp"
#include "core/stores/tournament_store.hpp"

EraseDrawSystemPreferenceRow::EraseDrawSystemPreferenceRow(std::size_t row)
    : mRow(row)
{}

void EraseDrawSystemPreferenceRow::redoImpl(TournamentStore & tournament) {
    auto &systems = tournament.getPreferences().getPreferredDrawSystems();

    if (mRow >= systems.size())
        return;

    auto it = systems.begin() + mRow;
    mPrevValue = *it;
    systems.erase(it);

    tournament.changePreferences();
}

void EraseDrawSystemPreferenceRow::undoImpl(TournamentStore & tournament) {
    auto &systems = tournament.getPreferences().getPreferredDrawSystems();

    if (mRow > systems.size())
        return;

    auto it = systems.begin() + mRow;
    systems.insert(it, mPrevValue);

    tournament.changePreferences();
}

std::unique_ptr<Action> EraseDrawSystemPreferenceRow::freshClone() const {
    return std::make_unique<EraseDrawSystemPreferenceRow>(mRow);
}

std::string EraseDrawSystemPreferenceRow::getDescription() const {
    return "Erase draw system preference row";
}

