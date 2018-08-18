#pragma once

#include <string>

#include "actions/action.hpp"
#include "stores/tournament_store.hpp"
#include "stores/player_store.hpp"

class CreatePlayerAction : public Action {
public:
    CreatePlayerAction(TournamentStore * store, const std::string & firstName, const std::string & lastName, uint8_t age);
    virtual bool operator()(TournamentStore * store) const;
    virtual std::unique_ptr<Action> getInverse() const;

private:
    std::string mFirstName;
    std::string mLastName;
    uint8_t mAge;
    Id mId;
};
