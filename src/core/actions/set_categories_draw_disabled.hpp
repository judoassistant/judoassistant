#pragma once

#include "core/actions/action.hpp"
#include "core/actions/confirmable_action.hpp"

class TournamentStore;
class CategoryId;
class DrawCategoriesAction;
class DrawSystem;

class SetCategoriesDrawDisabled : public Action, public ConfirmableAction {
public:
    SetCategoriesDrawDisabled() = default;
    SetCategoriesDrawDisabled(std::vector<CategoryId> categoryIds, bool disabled);
    SetCategoriesDrawDisabled(std::vector<CategoryId> categoryIds, bool disabled, unsigned int seed);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryIds, mDisabled, mSeed);
    }

    bool doesRequireConfirmation(const TournamentStore &tournament) const override;

private:
    std::vector<CategoryId> mCategoryIds;
    bool mDisabled;
    unsigned int mSeed;

    // undo members
    std::vector<CategoryId> mChangedCategories;
    std::unique_ptr<DrawCategoriesAction> mDrawAction;
};

CEREAL_REGISTER_TYPE(SetCategoriesDrawDisabled)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, SetCategoriesDrawDisabled)

