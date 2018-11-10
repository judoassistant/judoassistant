#pragma once

#include <string>

#include "serialize.hpp"
#include "actions/action.hpp"
#include "stores/tournament_store.hpp"
#include "stores/player_store.hpp"

class ChangeTournamentNameAction : public Action {
public:
    ChangeTournamentNameAction() = default;
    ChangeTournamentNameAction(const std::string &name);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mName);
    }

private:
    std::string mName;

    // undo members
    std::string mOldName;
};

CEREAL_REGISTER_TYPE(ChangeTournamentNameAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeTournamentNameAction)

