#pragma once

#include "core/actions/action.hpp"
#include "core/id.hpp"
#include "core/rulesets/ruleset_identifier.hpp"
#include "core/draw_systems/draw_system_identifier.hpp"

class TournamentStore;
class AddCategoryAction;
class AddPlayersToCategoryAction;

class AddCategoryWithPlayersAction : public Action {
public:
    AddCategoryWithPlayersAction() = default;
    AddCategoryWithPlayersAction(TournamentStore & tournament, const std::string &name, RulesetIdentifier ruleset, DrawSystemIdentifier drawSystem, const std::vector<PlayerId> &playerIds);
    AddCategoryWithPlayersAction(CategoryId id, const std::string &name, RulesetIdentifier ruleset, DrawSystemIdentifier drawSystem, const std::vector<PlayerId> &playerIds, unsigned int seed);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mId, mName, mRuleset, mDrawSystem, mPlayerIds, mSeed);
    }

private:
    CategoryId mId;
    std::string mName;
    RulesetIdentifier mRuleset;
    DrawSystemIdentifier mDrawSystem;
    std::vector<PlayerId> mPlayerIds;
    unsigned int mSeed;

    // undo members
    std::unique_ptr<AddCategoryAction> mCategoryAction;
    std::unique_ptr<AddPlayersToCategoryAction> mPlayersAction;
};

CEREAL_REGISTER_TYPE(AddCategoryWithPlayersAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, AddCategoryWithPlayersAction)

