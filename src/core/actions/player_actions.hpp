#pragma once

#include <string>

#include "core/serialize.hpp"
#include "core/actions/action.hpp"
#include "core/stores/player_store.hpp"

class TournamentStore;

class AddPlayersAction : public Action {
public:
    AddPlayersAction() = default;
    AddPlayersAction(TournamentStore & tournament, const std::vector<PlayerFields> &fields);
    AddPlayersAction(const std::vector<PlayerId> &ids, const std::vector<PlayerFields> &fields);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mIds);
        ar(mFields);
    }

private:
    std::vector<PlayerId> mIds;
    std::vector<PlayerFields> mFields;
};

CEREAL_REGISTER_TYPE(AddPlayersAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, AddPlayersAction)

class ErasePlayersFromCategoryAction;

class ErasePlayersAction : public Action {
public:
    ErasePlayersAction() = default;
    ErasePlayersAction(const std::vector<PlayerId> &playerIds);
    ErasePlayersAction(const std::vector<PlayerId> &playerIds, unsigned int seed);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds, mSeed);
    }

private:
    std::vector<PlayerId> mPlayerIds;
    unsigned int mSeed;

    // undo members
    std::vector<PlayerId> mErasedPlayerIds;
    std::stack<std::unique_ptr<PlayerStore>> mPlayers;
    std::stack<std::unique_ptr<ErasePlayersFromCategoryAction>> mActions;
};

CEREAL_REGISTER_TYPE(ErasePlayersAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ErasePlayersAction)

class ChangePlayersFirstNameAction : public Action {
public:
    ChangePlayersFirstNameAction () = default;
    ChangePlayersFirstNameAction(std::vector<PlayerId> playerId, const std::string &value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds);
        ar(mValue);
    }

private:
    std::vector<PlayerId> mPlayerIds;
    std::string mValue;

    // undo members
    std::vector<std::string> mOldValues;
};

CEREAL_REGISTER_TYPE(ChangePlayersFirstNameAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayersFirstNameAction)

class ChangePlayersLastNameAction : public Action {
public:
    ChangePlayersLastNameAction() = default;
    ChangePlayersLastNameAction(std::vector<PlayerId> playerId, const std::string &value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds);
        ar(mValue);
    }

private:
    std::vector<PlayerId> mPlayerIds;
    std::string mValue;

    // undo members
    std::vector<std::string> mOldValues;
};

CEREAL_REGISTER_TYPE(ChangePlayersLastNameAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayersLastNameAction)

class ChangePlayersAgeAction : public Action {
public:
    ChangePlayersAgeAction() = default;
    ChangePlayersAgeAction(std::vector<PlayerId> playerIds, std::optional<PlayerAge> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds);
        ar(mValue);
    }

private:
    std::vector<PlayerId> mPlayerIds;
    std::optional<PlayerAge> mValue;

    // undo members
    std::vector<std::optional<PlayerAge>> mOldValues;
};

CEREAL_REGISTER_TYPE(ChangePlayersAgeAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayersAgeAction)

class ChangePlayersRankAction : public Action {
public:
    ChangePlayersRankAction() = default;
    ChangePlayersRankAction(std::vector<PlayerId> playerIds, std::optional<PlayerRank> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds);
        ar(mValue);
    }

private:
    std::vector<PlayerId> mPlayerIds;
    std::optional<PlayerRank> mValue;

    // undo members
    std::vector<std::optional<PlayerRank>> mOldValues;
};

CEREAL_REGISTER_TYPE(ChangePlayersRankAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayersRankAction)

class ChangePlayersClubAction : public Action {
public:
    ChangePlayersClubAction() = default;
    ChangePlayersClubAction(std::vector<PlayerId> playerIds, const std::string &value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds);
        ar(mValue);
    }

private:
    std::vector<PlayerId> mPlayerIds;
    std::string mValue;

    // undo members
    std::vector<std::string> mOldValues;
};

CEREAL_REGISTER_TYPE(ChangePlayersClubAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayersClubAction)

class ChangePlayersWeightAction : public Action {
public:
    ChangePlayersWeightAction() = default;
    ChangePlayersWeightAction(std::vector<PlayerId> playerIds, std::optional<PlayerWeight> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds);
        ar(mValue);
    }

private:
    std::vector<PlayerId> mPlayerIds;
    std::optional<PlayerWeight> mValue;

    // undo members
    std::vector<std::optional<PlayerWeight>> mOldValues;
};

CEREAL_REGISTER_TYPE(ChangePlayersWeightAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayersWeightAction)

class ChangePlayersCountryAction : public Action {
public:
    ChangePlayersCountryAction() = default;
    ChangePlayersCountryAction(std::vector<PlayerId> playerIds, std::optional<PlayerCountry> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds);
        ar(mValue);
    }

private:
    std::vector<PlayerId> mPlayerIds;
    std::optional<PlayerCountry> mValue;

    // undo members
    std::vector<std::optional<PlayerCountry>> mOldValues;
};

CEREAL_REGISTER_TYPE(ChangePlayersCountryAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayersCountryAction)

class ChangePlayersSexAction : public Action {
public:
    ChangePlayersSexAction() = default;
    ChangePlayersSexAction(std::vector<PlayerId> playerId, std::optional<PlayerSex> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds);
        ar(mValue);
    }

private:
    std::vector<PlayerId> mPlayerIds;
    std::optional<PlayerSex> mValue;

    // undo members
    std::vector<std::optional<PlayerSex>> mOldValues;
};

CEREAL_REGISTER_TYPE(ChangePlayersSexAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayersSexAction)

