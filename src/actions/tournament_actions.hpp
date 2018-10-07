#pragma once

#include <string>

#include "serialize.hpp"
#include "actions/action.hpp"
#include "stores/tournament_store.hpp"
#include "stores/player_store.hpp"

class ChangeTournamentNameAction : public Action {
public:
    ChangeTournamentNameAction(TournamentStore &tournament, const std::string &name);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    std::string mName;

    // undo members
    std::string mOldName;
};

