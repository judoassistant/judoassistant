#include "core/actions/change_draw_system_preference_identifier_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/preferences_store.hpp"

ChangeDrawSystemPreferenceIdentifierAction::ChangeDrawSystemPreferenceIdentifierAction(std::size_t row, DrawSystemIdentifier identifier)
    : mRow(row)
    , mIdentifier(identifier)
{}

void ChangeDrawSystemPreferenceIdentifierAction::redoImpl(TournamentStore & tournament) {
    auto &systems = tournament.getPreferences().getPreferredDrawSystems();

    if (mRow >= systems.size())
        return;
    mPrevIdentifier = systems[mRow].drawSystem;
    systems[mRow].drawSystem = mIdentifier;

    tournament.changePreferences();
}

void ChangeDrawSystemPreferenceIdentifierAction::undoImpl(TournamentStore & tournament) {
    auto &systems = tournament.getPreferences().getPreferredDrawSystems();

    if (mRow >= systems.size())
        return;
    systems[mRow].drawSystem = mIdentifier;

    tournament.changePreferences();
}

std::unique_ptr<Action> ChangeDrawSystemPreferenceIdentifierAction::freshClone() const {
    return std::make_unique<ChangeDrawSystemPreferenceIdentifierAction>(mRow, mIdentifier);
}

std::string ChangeDrawSystemPreferenceIdentifierAction::getDescription() const {
    return "Change draw system preference";
}

