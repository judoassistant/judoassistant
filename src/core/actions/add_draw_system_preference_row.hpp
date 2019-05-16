#pragma once

#include "core/actions/action.hpp"
#include "core/draw_systems/draw_system_identifier.hpp"

class TournamentStore;

// Insert a new row before the given row
class AddDrawSystemPreferenceRow : public Action {
public:
    AddDrawSystemPreferenceRow() = default;
    AddDrawSystemPreferenceRow(std::size_t row);
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
};

CEREAL_REGISTER_TYPE(AddDrawSystemPreferenceRow)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, AddDrawSystemPreferenceRow)

