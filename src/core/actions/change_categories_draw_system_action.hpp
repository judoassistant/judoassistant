#pragma once

#include "core/actions/action.hpp"
#include "core/actions/confirmable_action.hpp"
#include "core/draw_systems/draw_system_identifier.hpp"

class CategoryId;
class DrawSystem;
class TournamentStore;

class ChangeCategoriesDrawSystemAction : public Action, public ConfirmableAction {
public:
    ChangeCategoriesDrawSystemAction() = default;
    ChangeCategoriesDrawSystemAction(std::vector<CategoryId> categoryIds, DrawSystemIdentifier drawSystem);
    ChangeCategoriesDrawSystemAction(std::vector<CategoryId> categoryIds, DrawSystemIdentifier drawSystem, unsigned int seed);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryIds, mDrawSystem, mSeed);
    }

    bool doesRequireConfirmation(const TournamentStore &tournament) const override;

private:
    std::vector<CategoryId> getCategoriesThatChange(const TournamentStore &tournament) const;

    std::vector<CategoryId> mCategoryIds;
    DrawSystemIdentifier mDrawSystem;
    unsigned int mSeed;

    // undo members
    std::vector<CategoryId> mChangedCategories;
    std::vector<std::unique_ptr<DrawSystem>> mOldDrawSystems;
    std::unique_ptr<Action> mDrawAction;
    std::vector<std::unique_ptr<Action>> mLocationActions;
};

CEREAL_REGISTER_TYPE(ChangeCategoriesDrawSystemAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeCategoriesDrawSystemAction)

