#pragma once

#include "core/id.hpp"
#include "core/actions/action.hpp"

class TournamentStore;
class DrawCategoriesAction;

class AddPlayersToCategoryAction : public Action {
public:
    AddPlayersToCategoryAction() = default;
    AddPlayersToCategoryAction(CategoryId categoryId, const std::vector<PlayerId> &playerIds);
    AddPlayersToCategoryAction(CategoryId categoryId, const std::vector<PlayerId> &playerIds, unsigned int seed);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId, mPlayerIds, mSeed);
    }

private:
    CategoryId mCategoryId;
    std::vector<PlayerId> mPlayerIds;
    unsigned int mSeed;

    // undo members
    std::vector<PlayerId> mAddedPlayerIds;
    std::unique_ptr<DrawCategoriesAction> mDrawAction;
};

CEREAL_REGISTER_TYPE(AddPlayersToCategoryAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, AddPlayersToCategoryAction)

