#pragma once

#include "core/actions/action.hpp"
#include "core/draw_systems/draw_system_identifier.hpp"
#include "core/stores/preferences_store.hpp"

class TournamentStore;

// Insert a new row before the given row
class EraseDrawSystemPreferenceRow : public Action {
public:
    EraseDrawSystemPreferenceRow() = default;
    EraseDrawSystemPreferenceRow(std::size_t row);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mRow);
    }

private:
    std::size_t mRow;

    // undo members
    DrawSystemPreference mPrevValue;
};

CEREAL_REGISTER_TYPE(EraseDrawSystemPreferenceRow)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, EraseDrawSystemPreferenceRow)

