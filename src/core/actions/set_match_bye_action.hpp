#pragma once

#include "core/actions/action.hpp"
#include "core/id.hpp"
#include "core/stores/match_store.hpp"
#include "core/actions/match_event_action.hpp"

// technically not a match event, but same code
class SetMatchByeAction : public MatchEventAction {
public:
    SetMatchByeAction() = default;
    SetMatchByeAction(CombinedId combinedId, bool bye);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::base_class<MatchEventAction>(this));
        ar(mBye);
    }

private:
    bool mBye;
};

CEREAL_REGISTER_TYPE(SetMatchByeAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(MatchEventAction, SetMatchByeAction)

