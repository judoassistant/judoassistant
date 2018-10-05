#pragma once

#include <string>

#include "serialize.hpp"
#include "actions/action.hpp"
#include "stores/tournament_store.hpp"
#include "stores/player_store.hpp"

class CreatePlayerAction : public Action {
public:
    CreatePlayerAction(TournamentStore & tournament, const std::string &firstName, const std::string &lastName, uint8_t age);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

private:
    Id mId;
    std::string mFirstName;
    std::string mLastName;
    uint8_t mAge;
};

// class DeletePlayerAction : public Action {
// public:
//     DeletePlayerAction(TournamentStore & tournament, const Id & player);
//     void redoImpl(TournamentStore & tournament) override;
//     void undoImpl(TournamentStore & tournament) override;
// private:
//     Id mId;
//     std::unique_ptr<PlayerStore player> mPlayer;
// };

