#pragma once

#include <unordered_map>
#include <unordered_set>

#include "core/id.hpp"
#include "core/actions/action.hpp"
#include "core/stores/player_store.hpp"

class TournamentStore;
class PlayerFields;
class AddPlayersAction;
class AddCategoryAction;
class AddPlayersToCategoryAction;

class AddPlayersWithCategoriesAction : public Action {
public:
    AddPlayersWithCategoriesAction() = default;
    AddPlayersWithCategoriesAction(const TournamentStore &tournament, const std::vector<PlayerFields> &playerFields, const std::vector<std::optional<std::string>> &playerCategories);
    AddPlayersWithCategoriesAction(const std::vector<PlayerId> &playerIds, const std::vector<PlayerFields> &playerFields, const std::vector<std::optional<CategoryId>> &playerCategories, const std::vector<std::pair<CategoryId, std::string>> &newCategories, unsigned int seed);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds, mPlayerFields, mPlayerCategories, mNewCategories, mSeed);
    }

private:
    std::vector<PlayerId> mPlayerIds;
    std::vector<PlayerFields> mPlayerFields;
    std::vector<std::optional<CategoryId>> mPlayerCategories;
    std::vector<std::pair<CategoryId, std::string>> mNewCategories;
    unsigned int mSeed;

    // undo members
    std::unique_ptr<AddPlayersAction> mAddPlayerAction;
    std::stack<std::unique_ptr<AddCategoryAction>> mAddCategoryActions;
    std::stack<std::unique_ptr<AddPlayersToCategoryAction>> mAddPlayersToCategoryActions;

    // helper methods
    std::unordered_map<std::string, CategoryId> createOrGetCategoryIds(const std::unordered_set<std::string> &categoryNames, const TournamentStore &tournament);
    std::unordered_set<std::string> getUniqueCategoryNames(const std::vector<std::optional<std::string>> &categoryNames) const;
};

CEREAL_REGISTER_TYPE(AddPlayersWithCategoriesAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, AddPlayersWithCategoriesAction)

