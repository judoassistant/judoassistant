#pragma once

#include "core/actions/action.hpp"
#include "core/core.hpp"
#include "core/id.hpp"
#include "core/rulesets/ruleset_identifier.hpp"
#include "core/draw_systems/draw_system_identifier.hpp"

class TournamentStore;

class AddCategoryAction : public Action {
public:
    AddCategoryAction() = default;
    AddCategoryAction(TournamentStore & tournament, const std::string &name, RulesetIdentifier ruleset, DrawSystemIdentifier drawSystem);
    AddCategoryAction(CategoryId id, const std::string &name, RulesetIdentifier ruleset, DrawSystemIdentifier drawSystem);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
    CategoryId getId() const;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mId);
        ar(mName);
        ar(mRuleset);
        ar(mDrawSystem);
    }

private:
    CategoryId mId;
    std::string mName;
    RulesetIdentifier mRuleset;
    DrawSystemIdentifier mDrawSystem;
};

CEREAL_REGISTER_TYPE(AddCategoryAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, AddCategoryAction)

