#pragma once

#include "core/actions/action.hpp"

class TournamentStore;
class CategoryId;

class ChangeCategoriesNameAction : public Action {
public:
    ChangeCategoriesNameAction() = default;
    ChangeCategoriesNameAction(std::vector<CategoryId> categoryIds, const std::string &value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryIds);
        ar(mValue);
    }

private:
    std::vector<CategoryId> mCategoryIds;
    std::string mValue;

    // undo members
    std::vector<CategoryId> mChangedCategories;
    std::vector<std::string> mOldValues;
};

CEREAL_REGISTER_TYPE(ChangeCategoriesNameAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeCategoriesNameAction)

