#pragma once

#include "core/actions/action.hpp"
#include "core/draw_systems/draw_system_identifier.hpp"

class TournamentStore;

class ChangeDrawSystemPreferenceIdentifierAction : public Action {
public:
    ChangeDrawSystemPreferenceIdentifierAction() = default;
    ChangeDrawSystemPreferenceIdentifierAction(std::size_t row, DrawSystemIdentifier identifier);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mRow, mIdentifier);
    }

private:
    std::size_t mRow;
    DrawSystemIdentifier mIdentifier;

    // undo members
    DrawSystemIdentifier mPrevIdentifier;
};

CEREAL_REGISTER_TYPE(ChangeDrawSystemPreferenceIdentifierAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeDrawSystemPreferenceIdentifierAction)

