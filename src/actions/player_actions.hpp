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
    AddPlayerAction(TournamentStore & tournament, const std::string & firstName, const std::string & lastName, std::optional<PlayerAge> age, std::optional<PlayerRank> rank, const std::string &club, std::optional<PlayerWeight> weight, std::optional<PlayerCountry> country, std::optional<PlayerSex> sex);
    AddPlayerAction(PlayerId id, const std::string & firstName, const std::string & lastName, std::optional<PlayerAge> age, std::optional<PlayerRank> rank, const std::string &club, std::optional<PlayerWeight> weight, std::optional<PlayerCountry> country, std::optional<PlayerSex> sex);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

private:
    PlayerId mId;
    std::string mFirstName;
    std::string mLastName;
    std::optional<PlayerAge> mAge;
    std::optional<PlayerRank> mRank;
    std::string mClub;
    std::optional<PlayerWeight> mWeight;
    std::optional<PlayerCountry> mCountry;
    std::optional<PlayerSex> mSex;
};

class ErasePlayersFromCategoryAction;

class ErasePlayersAction : public Action {
public:
    ErasePlayersAction(const std::vector<PlayerId> &playerIds);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

private:
    std::vector<PlayerId> mPlayerIds;

    // undo members
    std::vector<PlayerId> mErasedPlayerIds;
    std::stack<std::unique_ptr<PlayerStore>> mPlayers;
    std::stack<std::unique_ptr<ErasePlayersFromCategoryAction>> mActions;
};

class ChangePlayerFirstNameAction : public Action {
public:
    ChangePlayerFirstNameAction(PlayerId playerId, const std::string &value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

private:
    PlayerId mPlayerId;
    std::string mValue;

    // undo members
    std::string mOldValue;
};

class ChangePlayerLastNameAction : public Action {
public:
    ChangePlayerLastNameAction(PlayerId playerId, const std::string &value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

private:
    PlayerId mPlayerId;
    std::string mValue;

    // undo members
    std::string mOldValue;
};

class ChangePlayerAgeAction : public Action {
public:
    ChangePlayerAgeAction(PlayerId playerId, std::optional<PlayerAge> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

private:
    PlayerId mPlayerId;
    std::optional<PlayerAge> mValue;

    // undo members
    std::optional<PlayerAge> mOldValue;
};

class ChangePlayerRankAction : public Action {
public:
    ChangePlayerRankAction(PlayerId playerId, std::optional<PlayerRank> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

private:
    PlayerId mPlayerId;
    std::optional<PlayerRank> mValue;

    // undo members
    std::optional<PlayerRank> mOldValue;
};

class ChangePlayerClubAction : public Action {
public:
    ChangePlayerClubAction(PlayerId playerId, const std::string &value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

private:
    PlayerId mPlayerId;
    std::string mValue;

    // undo members
    std::string mOldValue;
};

class ChangePlayerWeightAction : public Action {
public:
    ChangePlayerWeightAction(PlayerId playerId, std::optional<PlayerWeight> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

private:
    PlayerId mPlayerId;
    std::optional<PlayerWeight> mValue;

    // undo members
    std::optional<PlayerWeight> mOldValue;
};

class ChangePlayerCountryAction : public Action {
public:
    ChangePlayerCountryAction(PlayerId playerId, std::optional<PlayerCountry> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

private:
    PlayerId mPlayerId;
    std::optional<PlayerCountry> mValue;

    // undo members
    std::optional<PlayerCountry> mOldValue;
};

class ChangePlayerSexAction : public Action {
public:
    ChangePlayerSexAction(PlayerId playerId, std::optional<PlayerSex> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

private:
    PlayerId mPlayerId;
    std::optional<PlayerSex> mValue;

    // undo members
    std::optional<PlayerSex> mOldValue;
};
