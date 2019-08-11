#pragma once

#include <stack>

#include "core/actions/action.hpp"
#include "core/id.hpp"

class TournamentStore;
struct BlockLocation;
enum class MatchType;

class EraseCategoriesAction : public Action {
public:
    EraseCategoriesAction() = default;
    EraseCategoriesAction(const std::vector<CategoryId> &categoryIds);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryIds);
    }

private:
    std::vector<CategoryId> mCategoryIds;

    // undo members
    std::vector<CategoryId> mErasedCategoryIds;
    std::stack<std::unique_ptr<CategoryStore>> mCategories;
    std::vector<BlockLocation> mLocations;
    std::vector<std::pair<CategoryId, MatchType>> mBlocks;
};

CEREAL_REGISTER_TYPE(EraseCategoriesAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, EraseCategoriesAction)

