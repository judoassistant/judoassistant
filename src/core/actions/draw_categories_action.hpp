#pragma once

#include "core/actions/action.hpp"
#include "core/actions/confirmable_action.hpp"
#include "core/actions/confirmable_action.hpp"

class CategoryId;
class DrawSystem;
class MatchStore;
class TournamentStore;
struct CategoryStatus;

class DrawCategoriesAction : public Action, public ConfirmableAction {
public:
    DrawCategoriesAction() = default;
    DrawCategoriesAction(const std::vector<CategoryId> &categoryIds);
    DrawCategoriesAction(const std::vector<CategoryId> &categoryIds, unsigned int seed);
    // DrawCategoriesAction(CategoryId categoryId);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    bool doesRequireConfirmation(const TournamentStore &tournament) const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryIds, mSeed);
    }

private:
    std::vector<CategoryId> getCategoriesThatChange(const TournamentStore &tournament) const;

    std::vector<CategoryId> mCategoryIds;
    unsigned int mSeed;

    // undo members
    std::vector<std::vector<std::unique_ptr<MatchStore>>> mOldMatches;
    std::vector<std::vector<std::unique_ptr<Action>>> mActions;
    std::vector<std::unique_ptr<DrawSystem>> mOldDrawSystems;
    std::vector<std::array<CategoryStatus, 2>> mOldStati;
};

CEREAL_REGISTER_TYPE(DrawCategoriesAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, DrawCategoriesAction)

