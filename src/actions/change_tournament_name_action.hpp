#pragma once

#include <string>

#include "serialize.hpp"
#include "actions/action.hpp"
#include "stores/tournament_store.hpp"
#include "stores/player_store.hpp"

class ChangeTournamentNameAction : public Action {
public:
    ChangeTournamentNameAction(TournamentStore &tournament, const std::string &name);
    ChangeTournamentNameAction(const std::string &oldName, const std::string &newName);
    bool operator()(TournamentStore &tournament) const override;
    std::unique_ptr<Action> getInverse() const override;

private:
    std::string mNewName;
    std::string mOldName;
    Id mId;
};
