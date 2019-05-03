#pragma once

#include "core/actions/action.hpp"

class TournamentStore;
class CategoryId;
class DrawCategoriesAction;
class DrawSystem;

class ChangeCategoriesDrawSystemAction : public Action {
public:
    ChangeCategoriesDrawSystemAction() = default;
    ChangeCategoriesDrawSystemAction(std::vector<CategoryId> categoryIds, size_t drawSystem);
    ChangeCategoriesDrawSystemAction(std::vector<CategoryId> categoryIds, size_t drawSystem, unsigned int seed);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryIds, mDrawSystem, mSeed);
    }

private:
    std::vector<CategoryId> mCategoryIds;
    size_t mDrawSystem;
    unsigned int mSeed;

    // undo members
    std::vector<std::unique_ptr<DrawSystem>> mOldDrawSystems;
    std::unique_ptr<DrawCategoriesAction> mDrawAction;
};

CEREAL_REGISTER_TYPE(ChangeCategoriesDrawSystemAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeCategoriesDrawSystemAction)

