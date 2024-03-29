#pragma once

#include <stack>

#include "core/actions/action.hpp"
#include "core/actions/confirmable_action.hpp"
#include "core/id.hpp"
#include "core/stores/match_store.hpp"

// Action used for either resetting multiple categories or one match
class ResetMatchesAction : public Action, public ConfirmableAction {
public:
    ResetMatchesAction() = default;
    ResetMatchesAction(const std::vector<CategoryId> &categoryIds);
    ResetMatchesAction(CombinedId combinedId);
    ResetMatchesAction(const std::vector<CategoryId> &categoryIds, std::optional<MatchId> matchId);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryIds, mMatchId);
    }

    bool doesRequireConfirmation(const TournamentStore &tournament) const override;

private:
    std::vector<CategoryId> mCategoryIds;
    std::optional<MatchId> mMatchId;

    // undo fields
    std::vector<CombinedId> mChangedMatches;
    std::stack<MatchStore::State> mPrevStates;
    std::stack<std::vector<MatchEvent>> mPrevEvents;
    std::stack<std::unique_ptr<Action>> mDrawActions;
};

CEREAL_REGISTER_TYPE(ResetMatchesAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ResetMatchesAction)

