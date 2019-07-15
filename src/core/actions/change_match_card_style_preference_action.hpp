#pragma once

#include "core/actions/action.hpp"
#include "core/stores/preferences_store.hpp"

class TournamentStore;

class ChangeMatchCardStylePreferenceAction : public Action {
public:
    ChangeMatchCardStylePreferenceAction() = default;
    ChangeMatchCardStylePreferenceAction(MatchCardStylePreference value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mValue);
    }

private:
    MatchCardStylePreference mValue;

    // undo members
    MatchCardStylePreference mOldValue;
};

CEREAL_REGISTER_TYPE(ChangeMatchCardStylePreferenceAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeMatchCardStylePreferenceAction)

