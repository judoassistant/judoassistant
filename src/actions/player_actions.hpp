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

class ChangePlayerFirstNameAction : public Action {
public:
    ChangePlayerFirstNameAction(TournamentStore &tournament, PlayerId playerId, const std::string &value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    PlayerId mPlayerId;
    std::string mValue;

    // undo members
    std::string mOldValue;
};

class ChangePlayerLastNameAction : public Action {
public:
    ChangePlayerLastNameAction(TournamentStore &tournament, PlayerId playerId, const std::string &value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    PlayerId mPlayerId;
    std::string mValue;

    // undo members
    std::string mOldValue;
};

class ChangePlayerAgeAction : public Action {
public:
    ChangePlayerAgeAction(TournamentStore &tournament, PlayerId playerId, std::optional<uint8_t> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    PlayerId mPlayerId;
    std::optional<uint8_t> mValue;

    // undo members
    std::optional<uint8_t> mOldValue;
};

class ChangePlayerRankAction : public Action {
public:
    ChangePlayerRankAction(TournamentStore &tournament, PlayerId playerId, std::optional<PlayerRank> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    PlayerId mPlayerId;
    std::optional<PlayerRank> mValue;

    // undo members
    std::optional<PlayerRank> mOldValue;
};

class ChangePlayerClubAction : public Action {
public:
    ChangePlayerClubAction(TournamentStore &tournament, PlayerId playerId, const std::string &value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    PlayerId mPlayerId;
    std::string mValue;

    // undo members
    std::string mOldValue;
};

class ChangePlayerWeightAction : public Action {
public:
    ChangePlayerWeightAction(TournamentStore &tournament, PlayerId playerId, std::optional<float> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    PlayerId mPlayerId;
    std::optional<float> mValue;

    // undo members
    std::optional<float> mOldValue;
};

class ChangePlayerCountryAction : public Action {
public:
    ChangePlayerCountryAction(TournamentStore &tournament, PlayerId playerId, std::optional<PlayerCountry> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
private:
    PlayerId mPlayerId;
    std::optional<PlayerCountry> mValue;

    // undo members
    std::optional<PlayerCountry> mOldValue;
};
