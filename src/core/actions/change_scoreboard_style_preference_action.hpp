#pragma once

#include "core/actions/action.hpp"
#include "core/stores/preferences_store.hpp"

class TournamentStore;

class ChangeScoreboardStylePreferenceAction : public Action {
public:
    ChangeScoreboardStylePreferenceAction() = default;
    ChangeScoreboardStylePreferenceAction(ScoreboardStylePreference value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mValue);
    }

private:
    ScoreboardStylePreference mValue;

    // undo members
    ScoreboardStylePreference mOldValue;
};

CEREAL_REGISTER_TYPE(ChangeScoreboardStylePreferenceAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeScoreboardStylePreferenceAction)

