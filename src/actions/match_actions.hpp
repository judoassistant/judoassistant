#pragma once

#include "core.hpp"
#include "id.hpp"
#include "serialize.hpp"
#include "actions/action.hpp"
#include "stores/match_store.hpp"
#include "stores/player_store.hpp"

class TournamentStore;

// Only used when drawing matches and beginResetCategory has already been called
class AddMatchAction : public Action {
public:
    AddMatchAction() = default;
    AddMatchAction(const TournamentStore & tournament, CategoryId categoryId, MatchType type, const std::string &title, bool bye, std::optional<PlayerId> whitePlayerId, std::optional<PlayerId> bluePlayerId);
    AddMatchAction(MatchId id, CategoryId categoryId, MatchType type, const std::string &title, bool bye, std::optional<PlayerId> whitePlayerId, std::optional<PlayerId> bluePlayerId);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
    MatchId getMatchId();

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mId);
        ar(mCategoryId);
        ar(mType);
        ar(mTitle);
        ar(mBye);
        ar(mWhitePlayerId);
        ar(mBluePlayerId);
    }

private:
    MatchId mId;
    CategoryId mCategoryId;
    MatchType mType;
    std::string mTitle;
    bool mBye;
    std::optional<PlayerId> mWhitePlayerId;
    std::optional<PlayerId> mBluePlayerId;
};

CEREAL_REGISTER_TYPE(AddMatchAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, AddMatchAction)

// Abstract class with common methods for match events
class MatchEventAction : public Action {
public:
    MatchEventAction() = default;
    MatchEventAction(CategoryId categoryId, MatchId matchId);

    void save(const MatchStore &match);
    void recover(TournamentStore &tournament);
    bool shouldRecover();
    void notify(TournamentStore &tournament, const MatchStore &match);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId, mMatchId);
    }
protected:
    // fields common to all match event actions
    CategoryId mCategoryId;
    MatchId mMatchId;

    // undo fields
    bool mDidSave;
    size_t mPrevEventSize;
    MatchStatus mPrevStatus;
    MatchStore::Score mPrevWhiteScore;
    MatchStore::Score mPrevBlueScore;
    bool mPrevGoldenScore;
    std::chrono::milliseconds mPrevResumeTime; // the time when the clock was last resumed
    std::chrono::milliseconds mPrevDuration; // the match duration when the clock was last paused

    std::stack<std::unique_ptr<Action>> mDrawActions;
};

CEREAL_REGISTER_TYPE(MatchEventAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, MatchEventAction)

class ResumeMatchAction : public MatchEventAction {
public:
    ResumeMatchAction() = default;
    ResumeMatchAction(CategoryId categoryId, MatchId matchId, std::chrono::milliseconds masterTime);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::base_class<MatchEventAction>(this));
        ar(mMasterTime);
    }

private:
    std::chrono::milliseconds mMasterTime;
};

CEREAL_REGISTER_TYPE(ResumeMatchAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(MatchEventAction, ResumeMatchAction)

class PauseMatchAction : public MatchEventAction {
public:
    PauseMatchAction() = default;
    PauseMatchAction(CategoryId categoryId, MatchId matchId, std::chrono::milliseconds masterTime);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::base_class<MatchEventAction>(this));
        ar(mMasterTime);
    }

private:
    std::chrono::milliseconds mMasterTime;
};

CEREAL_REGISTER_TYPE(PauseMatchAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(MatchEventAction, PauseMatchAction)

class AwardIpponAction : public MatchEventAction {
public:
    AwardIpponAction() = default;
    AwardIpponAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;
    bool shouldDisplay(CategoryId categoryId, MatchId matchId) const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::base_class<MatchEventAction>(this));
        ar(mPlayerIndex, mMasterTime);
    }

private:
    MatchStore::PlayerIndex mPlayerIndex;
    std::chrono::milliseconds mMasterTime;
};

CEREAL_REGISTER_TYPE(AwardIpponAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(MatchEventAction, AwardIpponAction)

class AwardWazariAction : public MatchEventAction {
public:
    AwardWazariAction() = default;
    AwardWazariAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
    bool shouldDisplay(CategoryId categoryId, MatchId matchId) const override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::base_class<MatchEventAction>(this));
        ar(mPlayerIndex, mMasterTime);
    }

private:
    MatchStore::PlayerIndex mPlayerIndex;
    std::chrono::milliseconds mMasterTime;
};

CEREAL_REGISTER_TYPE(AwardWazariAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(MatchEventAction, AwardWazariAction)

class AwardShidoAction : public MatchEventAction {
public:
    AwardShidoAction() = default;
    AwardShidoAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;
    bool shouldDisplay(CategoryId categoryId, MatchId matchId) const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::base_class<MatchEventAction>(this));
        ar(mPlayerIndex, mMasterTime);
    }

private:
    MatchStore::PlayerIndex mPlayerIndex;
    std::chrono::milliseconds mMasterTime;
};

CEREAL_REGISTER_TYPE(AwardShidoAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(MatchEventAction, AwardShidoAction)

class AwardHansokuMakeAction : public MatchEventAction {
public:
    AwardHansokuMakeAction() = default;
    AwardHansokuMakeAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;
    bool shouldDisplay(CategoryId categoryId, MatchId matchId) const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::base_class<MatchEventAction>(this));
        ar(mPlayerIndex, mMasterTime);
    }

private:
    MatchStore::PlayerIndex mPlayerIndex;
    std::chrono::milliseconds mMasterTime;
};

CEREAL_REGISTER_TYPE(AwardHansokuMakeAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(MatchEventAction, AwardHansokuMakeAction)

