#pragma once

#include <stack>

#include "core/actions/action.hpp"
#include "core/stores/match_store.hpp"

// Abstract class with common methods for match events
class MatchEventAction : public Action {
public:
    // TODO: Try to split this into multiple classes to reduce overhead
    MatchEventAction() = default;
    MatchEventAction(CategoryId categoryId, MatchId matchId, std::chrono::milliseconds masterTime);

    void save(const MatchStore &match, unsigned int eventsToSave = 0);
    void recover(TournamentStore &tournament);
    bool shouldRecover();
    void notify(TournamentStore &tournament, MatchStore &match);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId, mMatchId, mMasterTime);
    }
protected:
    // fields common to all match event actions
    CategoryId mCategoryId;
    MatchId mMatchId;
    std::chrono::milliseconds mMasterTime;

    // undo fields
    bool mDidSave;
    std::optional<std::chrono::milliseconds> mPrevWhiteLastFinishTime;
    std::optional<std::chrono::milliseconds> mPrevBlueLastFinishTime;
    MatchStore::State mPrevState;
    std::size_t mPrevEventSize;
    std::stack<std::unique_ptr<Action>> mDrawActions;
};

CEREAL_REGISTER_TYPE(MatchEventAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, MatchEventAction)

