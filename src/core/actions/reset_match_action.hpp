#pragma once

#include "core/actions/action.hpp"
#include "core/id.hpp"
#include "core/stores/match_store.hpp"

class ResetMatchAction : public Action {
public:
    ResetMatchAction() = default;
    ResetMatchAction(CategoryId categoryId, MatchId matchId);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;
    bool shouldDisplay(CategoryId categoryId, MatchId matchId) const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId, mMatchId);
    }

private:
    CategoryId mCategoryId;
    MatchId mMatchId;

    // undo fields
    MatchStore::State mPrevState;
    std::vector<MatchEvent> mPrevEvents;
    std::stack<std::unique_ptr<Action>> mDrawActions;
};

CEREAL_REGISTER_TYPE(ResetMatchAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ResetMatchAction)
