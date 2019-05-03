#pragma once

#include "core/actions/action.hpp"
#include "core/id.hpp"
#include "core/stores/match_store.hpp"
#include "core/actions/match_event_action.hpp"

class StopOsaekomiAction : public MatchEventAction {
public:
    StopOsaekomiAction() = default;
    StopOsaekomiAction(CategoryId categoryId, MatchId matchId, std::chrono::milliseconds masterTime);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::base_class<MatchEventAction>(this), mMasterTime);
    }

private:
    std::chrono::milliseconds mMasterTime;
};

CEREAL_REGISTER_TYPE(StopOsaekomiAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(MatchEventAction, StopOsaekomiAction)
