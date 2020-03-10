#pragma once

#include "core/core.hpp"
#include "core/actions/action.hpp"

class TournamentStore;

class ChangeTournamentLocationAction : public Action {
public:
    ChangeTournamentLocationAction() = default;
    ChangeTournamentLocationAction(const std::string &location);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mLocation);
    }

private:
    std::string mLocation;

    // undo members
    std::string mOldLocation;
};

CEREAL_REGISTER_TYPE(ChangeTournamentLocationAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeTournamentLocationAction)

