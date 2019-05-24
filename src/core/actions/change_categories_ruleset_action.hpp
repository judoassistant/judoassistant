#pragma once

#include "core/actions/action.hpp"
#include "core/rulesets/ruleset_identifier.hpp"

class TournamentStore;
class Ruleset;
class ResetMatchesAction;

class ChangeCategoriesRulesetAction : public Action {
public:
    ChangeCategoriesRulesetAction() = default;
    ChangeCategoriesRulesetAction(const std::vector<CategoryId> &categoryIds, RulesetIdentifier ruleset);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryIds, mRuleset);
    }

private:
    std::vector<CategoryId> mCategoryIds;
    RulesetIdentifier mRuleset;

    // undo members
    std::vector<CategoryId> mChangedCategories;
    std::vector<std::unique_ptr<Ruleset>> mOldRulesets;
    std::unique_ptr<ResetMatchesAction> mResetAction;
};

CEREAL_REGISTER_TYPE(ChangeCategoriesRulesetAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeCategoriesRulesetAction)
