#include "core/actions/add_draw_system_preference_row.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/preferences_store.hpp"

AddDrawSystemPreferenceRow::AddDrawSystemPreferenceRow(std::size_t row)
    : mRow(row)
{}

void AddDrawSystemPreferenceRow::redoImpl(TournamentStore & tournament) {
    auto &systems = tournament.getPreferences().getPreferredDrawSystems();

    if (mRow > systems.size())
        return;

    auto it = systems.begin() + mRow;
    const DrawSystemPreference &preference = *(it == systems.end() ? std::prev(it) : it); // copy row below or above
    systems.insert(it, preference);

    tournament.changePreferences();
}

void AddDrawSystemPreferenceRow::undoImpl(TournamentStore & tournament) {
    auto &systems = tournament.getPreferences().getPreferredDrawSystems();

    if (mRow > systems.size())
        return;

    auto it = systems.begin() + mRow;
    systems.erase(it);

    tournament.changePreferences();
}

std::unique_ptr<Action> AddDrawSystemPreferenceRow::freshClone() const {
    return std::make_unique<AddDrawSystemPreferenceRow>(mRow);
}

std::string AddDrawSystemPreferenceRow::getDescription() const {
    return "Add draw system preference row";
}

