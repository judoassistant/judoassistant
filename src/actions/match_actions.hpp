#pragma once

#include "core.hpp"
#include "id.hpp"
#include "serialize.hpp"
#include "actions/action.hpp"
#include "stores/match_store.hpp"
#include "stores/player_store.hpp"
#include "stores/tatami_store.hpp"

class TournamentStore;

// Only used when drawing matches and beginResetCategory has already been called
class AddMatchAction : public Action {
public:
    AddMatchAction() = default;
    AddMatchAction(TournamentStore & tournament, CategoryId categoryId, MatchType type, const std::string &title, bool bye, std::optional<PlayerId> whitePlayerId, std::optional<PlayerId> bluePlayerId);
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

class ResumeMatchAction : public Action {
public:
    ResumeMatchAction() = default;
    ResumeMatchAction(CategoryId categoryId, MatchId matchId, std::chrono::milliseconds masterTime);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId, mMatchId, mMasterTime);
    }

private:
    CategoryId mCategoryId;
    MatchId mMatchId;
    std::chrono::milliseconds mMasterTime;

    // undo members
    bool mDidResume;
    MatchStatus mPrevStatus;
    std::chrono::milliseconds mPrevResumeTime;
};

CEREAL_REGISTER_TYPE(ResumeMatchAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ResumeMatchAction)

class PauseMatchAction : public Action {
public:
    PauseMatchAction() = default;
    PauseMatchAction(CategoryId categoryId, MatchId matchId, std::chrono::milliseconds masterTime);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId, mMatchId, mMasterTime);
    }

private:
    CategoryId mCategoryId;
    MatchId mMatchId;
    std::chrono::milliseconds mMasterTime;

    // undo members
    bool mDidPause;
    std::chrono::milliseconds mPrevDuration;
};

CEREAL_REGISTER_TYPE(PauseMatchAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, PauseMatchAction)
