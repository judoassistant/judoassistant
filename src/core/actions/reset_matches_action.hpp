#pragma once

#include "core/actions/action.hpp"
#include "core/id.hpp"
#include "core/stores/match_store.hpp"

class ResetMatchesAction : public Action {
public:
    ResetMatchesAction() = default;
    ResetMatchesAction(CategoryId categoryId, const std::vector<MatchId> &matchIds);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;
    bool shouldDisplay(CategoryId categoryId, MatchId matchId) const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId, mMatchIds);
    }

private:
    CategoryId mCategoryId;
    std::vector<MatchId> mMatchIds;

    // undo fields
    std::stack<MatchStore::State> mPrevStates;
    std::stack<std::vector<MatchEvent>> mPrevEvents;
    std::stack<std::unique_ptr<Action>> mDrawActions;
};

CEREAL_REGISTER_TYPE(ResetMatchesAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ResetMatchesAction)
