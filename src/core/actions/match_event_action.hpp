#pragma once

#include "core/actions/action.hpp"
#include "core/stores/match_store.hpp"

// Abstract class with common methods for match events
class MatchEventAction : public Action {
public:
    // TODO: Try to split this into multiple classes to reduce overhead
    MatchEventAction() = default;
    MatchEventAction(CategoryId categoryId, MatchId matchId);

    void save(const MatchStore &match, unsigned int eventsToSave = 0);
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
    bool mPrevBye;
    std::optional<std::pair<MatchStore::PlayerIndex, std::chrono::milliseconds>> mPrevOsaekomi;
    bool mPrevHasAwardedOsaekomiWazari;
    std::vector<MatchEvent> mSavedEvents;

    std::stack<std::unique_ptr<Action>> mDrawActions;
};

CEREAL_REGISTER_TYPE(MatchEventAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, MatchEventAction)

