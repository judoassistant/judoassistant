#pragma once

#include <string>

#include "serialize.hpp"
#include "actions/action.hpp"
#include "actions/match_actions.hpp"
#include "actions/category_actions.hpp"
#include "stores/tournament_store.hpp"
#include "stores/player_store.hpp"

class AddPlayerAction : public Action {
public:
    AddPlayerAction(TournamentStore & tournament, const std::string & firstName, const std::string & lastName, std::optional<uint8_t> age, std::optional<PlayerRank> rank, const std::string &club, std::optional<float> weight, std::optional<PlayerCountry> country);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

private:
    PlayerId mId;
    std::string mFirstName;
    std::string mLastName;
    std::optional<uint8_t> mAge;
    std::optional<PlayerRank> mRank;
    std::string mClub;
    std::optional<float> mWeight;
    std::optional<PlayerCountry> mCountry;
};

class ErasePlayersFromCategoryAction;

class ErasePlayersAction : public Action {
public:
    ErasePlayersAction(TournamentStore & tournament, std::vector<PlayerId> playerIds);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    std::vector<PlayerId> mPlayerIds;

    // undo members
    std::vector<PlayerId> mErasedPlayerIds;
    std::stack<std::unique_ptr<PlayerStore>> mPlayers;
    std::stack<std::unique_ptr<ErasePlayersFromCategoryAction>> mActions;
};
