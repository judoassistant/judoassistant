#pragma once

#include <stack>

#include "core/actions/action.hpp"
#include "core/stores/match_store.hpp"

// Abstract class with common methods for match events
class MatchEventAction : public Action {
public:
    // TODO: Try to split this into multiple classes to reduce overhead
    MatchEventAction() = default;
    MatchEventAction(const CombinedId &combinedId);

    void save(const MatchStore &match, unsigned int eventsToSave = 0);
    void recover(TournamentStore &tournament);
    bool shouldRecover();
    void notify(TournamentStore &tournament, const MatchStore &match);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCombinedId);
    }
protected:
    // fields common to all match event actions
    CombinedId mCombinedId;

    // undo fields
    bool mDidSave;
    MatchStore::State mPrevState;
    std::size_t mPrevEventSize;
    std::stack<std::unique_ptr<Action>> mDrawActions;
};

CEREAL_REGISTER_TYPE(MatchEventAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, MatchEventAction)

