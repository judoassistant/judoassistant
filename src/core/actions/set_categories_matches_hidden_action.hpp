#pragma once

#include "core/actions/action.hpp"
#include "core/actions/confirmable_action.hpp"

class TournamentStore;
class CategoryId;
class DrawCategoriesAction;
class DrawSystem;
class BlockLocation;

class SetCategoriesMatchesHiddenAction : public Action, public ConfirmableAction {
public:
    SetCategoriesMatchesHiddenAction() = default;
    SetCategoriesMatchesHiddenAction(std::vector<CategoryId> categoryIds, bool hidden);
    SetCategoriesMatchesHiddenAction(std::vector<CategoryId> categoryIds, bool hidden, unsigned int seed);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryIds, mHidden, mSeed);
    }

    bool doesRequireConfirmation(const TournamentStore &tournament) const override;

private:
    void signalChanges(const std::vector<CategoryId> &changedCategories, TournamentStore &tournament);
    std::vector<CategoryId> getCategoriesThatChange(const TournamentStore & tournament) const;

    std::vector<CategoryId> mCategoryIds;
    bool mHidden;
    unsigned int mSeed;

    // undo members
    std::vector<CategoryId> mChangedCategories;
};

CEREAL_REGISTER_TYPE(SetCategoriesMatchesHiddenAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, SetCategoriesMatchesHiddenAction)

