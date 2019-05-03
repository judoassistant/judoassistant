#pragma once

#include "core/actions/action.hpp"
#include "core/id.hpp"

class TournamentStore;

class AutoAddCategoriesAction : public Action {
public:
    AutoAddCategoriesAction() = default;
    AutoAddCategoriesAction(TournamentStore &tournament, std::vector<PlayerId> playerIds, std::string baseName, float maxDifference, size_t maxSize);
    AutoAddCategoriesAction(const std::vector<std::vector<PlayerId>> &playerIds, std::vector<CategoryId> categoryIds, std::string baseName, unsigned int seed);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds, mCategoryIds, mBaseName, mSeed);
    }

private:
    std::vector<std::vector<PlayerId>> mPlayerIds;
    std::vector<CategoryId> mCategoryIds;
    std::string mBaseName;
    unsigned int mSeed;
};

CEREAL_REGISTER_TYPE(AutoAddCategoriesAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, AutoAddCategoriesAction)

