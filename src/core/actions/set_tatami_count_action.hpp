#pragma once

#include "core/id.hpp"
#include "core/actions/action.hpp"
#include "core/stores/tatami/location.hpp"
#include "core/stores/tatami/tatami_store.hpp"

class TournamentStore;

class SetTatamiCountAction : public Action {
public:
    SetTatamiCountAction() = default;
    SetTatamiCountAction(TournamentStore &tournament, size_t count);
    SetTatamiCountAction(const std::vector<TatamiLocation> &locations);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mLocations);
    }

private:
    std::vector<TatamiLocation> mLocations;

    // undo members
    size_t mOldCount;
    std::vector<std::pair<TatamiLocation, TatamiStore>> mErasedTatamis;
};

CEREAL_REGISTER_TYPE(SetTatamiCountAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, SetTatamiCountAction)

