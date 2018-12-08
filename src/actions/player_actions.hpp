#pragma once

#include <string>

#include "serialize.hpp"
#include "actions/action.hpp"
#include "stores/player_store.hpp"

class TournamentStore;

class AddPlayerAction : public Action {
public:
    AddPlayerAction() = default;
    AddPlayerAction(TournamentStore & tournament, const PlayerFields &fields);
    AddPlayerAction(PlayerId id, const PlayerFields &fields);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mId);
        ar(mFields);
    }

private:
    PlayerId mId;
    PlayerFields mFields;
};

CEREAL_REGISTER_TYPE(AddPlayerAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, AddPlayerAction)

class ErasePlayersFromCategoryAction;

class ErasePlayersAction : public Action {
public:
    ErasePlayersAction() = default;
    ErasePlayersAction(const std::vector<PlayerId> &playerIds);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds);
    }

private:
    std::vector<PlayerId> mPlayerIds;

    // undo members
    std::vector<PlayerId> mErasedPlayerIds;
    std::stack<std::unique_ptr<PlayerStore>> mPlayers;
    std::stack<std::unique_ptr<ErasePlayersFromCategoryAction>> mActions;
};

CEREAL_REGISTER_TYPE(ErasePlayersAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ErasePlayersAction)

class ChangePlayerFirstNameAction : public Action {
public:
    ChangePlayerFirstNameAction() = default;
    ChangePlayerFirstNameAction(PlayerId playerId, const std::string &value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerId);
        ar(mValue);
    }

private:
    PlayerId mPlayerId;
    std::string mValue;

    // undo members
    std::string mOldValue;
};

CEREAL_REGISTER_TYPE(ChangePlayerFirstNameAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayerFirstNameAction)

class ChangePlayerLastNameAction : public Action {
public:
    ChangePlayerLastNameAction() = default;
    ChangePlayerLastNameAction(PlayerId playerId, const std::string &value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerId);
        ar(mValue);
    }

private:
    PlayerId mPlayerId;
    std::string mValue;

    // undo members
    std::string mOldValue;
};

CEREAL_REGISTER_TYPE(ChangePlayerLastNameAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayerLastNameAction)

class ChangePlayerAgeAction : public Action {
public:
    ChangePlayerAgeAction() = default;
    ChangePlayerAgeAction(PlayerId playerId, std::optional<PlayerAge> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerId);
        ar(mValue);
    }

private:
    PlayerId mPlayerId;
    std::optional<PlayerAge> mValue;

    // undo members
    std::optional<PlayerAge> mOldValue;
};

CEREAL_REGISTER_TYPE(ChangePlayerAgeAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayerAgeAction)

class ChangePlayerRankAction : public Action {
public:
    ChangePlayerRankAction() = default;
    ChangePlayerRankAction(PlayerId playerId, std::optional<PlayerRank> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerId);
        ar(mValue);
    }

private:
    PlayerId mPlayerId;
    std::optional<PlayerRank> mValue;

    // undo members
    std::optional<PlayerRank> mOldValue;
};

CEREAL_REGISTER_TYPE(ChangePlayerRankAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayerRankAction)

class ChangePlayerClubAction : public Action {
public:
    ChangePlayerClubAction() = default;
    ChangePlayerClubAction(PlayerId playerId, const std::string &value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerId);
        ar(mValue);
    }

private:
    PlayerId mPlayerId;
    std::string mValue;

    // undo members
    std::string mOldValue;
};

CEREAL_REGISTER_TYPE(ChangePlayerClubAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayerClubAction)

class ChangePlayerWeightAction : public Action {
public:
    ChangePlayerWeightAction() = default;
    ChangePlayerWeightAction(PlayerId playerId, std::optional<PlayerWeight> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerId);
        ar(mValue);
    }

private:
    PlayerId mPlayerId;
    std::optional<PlayerWeight> mValue;

    // undo members
    std::optional<PlayerWeight> mOldValue;
};

CEREAL_REGISTER_TYPE(ChangePlayerWeightAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayerWeightAction)

class ChangePlayerCountryAction : public Action {
public:
    ChangePlayerCountryAction() = default;
    ChangePlayerCountryAction(PlayerId playerId, std::optional<PlayerCountry> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerId);
        ar(mValue);
    }

private:
    PlayerId mPlayerId;
    std::optional<PlayerCountry> mValue;

    // undo members
    std::optional<PlayerCountry> mOldValue;
};

CEREAL_REGISTER_TYPE(ChangePlayerCountryAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayerCountryAction)

class ChangePlayerSexAction : public Action {
public:
    ChangePlayerSexAction() = default;
    ChangePlayerSexAction(PlayerId playerId, std::optional<PlayerSex> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerId);
        ar(mValue);
    }

private:
    PlayerId mPlayerId;
    std::optional<PlayerSex> mValue;

    // undo members
    std::optional<PlayerSex> mOldValue;
};

CEREAL_REGISTER_TYPE(ChangePlayerSexAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayerSexAction)

