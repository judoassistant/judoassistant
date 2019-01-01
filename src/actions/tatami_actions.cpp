#include "actions/tatami_actions.hpp"
#include "draw_systems/draw_system.hpp"
#include "rulesets/ruleset.hpp"
#include "stores/category_store.hpp"
#include "stores/tournament_store.hpp"
#include "stores/tatami/tatami_list.hpp"

SetTatamiLocationAction::SetTatamiLocationAction(std::pair<CategoryId, MatchType> block, std::optional<BlockLocation> location)
    : mBlock(block)
    , mLocation(location)
{}

void SetTatamiLocationAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mBlock.first)) return;

    auto & tatamis = tournament.getTatamis();
    // This action should not be used to create new tatamis
    if (mLocation && !tatamis.containsTatami(mLocation->getTatamiHandle()))
        return;

    CategoryStore &category = tournament.getCategory(mBlock.first);
    if (mBlock.second == MatchType::FINAL && !category.getDrawSystem().hasFinalBlock())
        return;

    mOldLocation = category.getLocation(mBlock.second);

    tatamis.moveBlock(tournament, mBlock, mOldLocation, mLocation);
    category.setLocation(mBlock.second, mLocation);

    {
        std::vector<BlockLocation> locations;
        if (mOldLocation) locations.push_back(*mOldLocation);
        if (mLocation) locations.push_back(*mLocation);
        std::vector<std::pair<CategoryId, MatchType>> blocks = {mBlock};

        tournament.changeTatamis(std::move(locations), std::move(blocks));
        tournament.changeCategories({mBlock.first});
    }
}

void SetTatamiLocationAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mBlock.first)) return;

    auto & tatamis = tournament.getTatamis();
    if (mLocation && !tatamis.containsTatami(mLocation->getTatamiHandle()))
        return;

    CategoryStore &category = tournament.getCategory(mBlock.first);
    if (mBlock.second == MatchType::FINAL && !category.getDrawSystem().hasFinalBlock())
        return;

    tatamis.moveBlock(tournament, mBlock, mLocation, mOldLocation);
    category.setLocation(mBlock.second, mOldLocation);

    {
        std::vector<BlockLocation> locations;
        if (mOldLocation) locations.push_back(*mOldLocation);
        if (mLocation) locations.push_back(*mLocation);
        std::vector<std::pair<CategoryId, MatchType>> blocks = {mBlock};

        tournament.changeTatamis(std::move(locations), std::move(blocks));
        tournament.changeCategories({mBlock.first});
    }
}

SetTatamiCountAction::SetTatamiCountAction(size_t count)
    : mCount(count)
{}

void SetTatamiCountAction::redoImpl(TournamentStore & tournament) {
    // TODO: Implement
    // auto & tatamis = tournament.getTatamis();

    // mOldCount = tatamis.tatamiCount();

    // if (tatamis.tatamiCount() < mCount) {
    //     std::vector<size_t> ids;
    //     for (size_t i = tatamis.tatamiCount(); i < mCount; ++i)
    //         ids.push_back(i);

    //     tournament.beginAddTatamis(std::move(ids));
    //     while (tatamis.tatamiCount() < mCount)
    //         tatamis.pushTatami();
    //     tournament.endAddTatamis();
    // }

    // if (tatamis.tatamiCount() > mCount) {
    //     std::vector<size_t> ids;
    //     for (size_t i = mCount; i < tatamis.tatamiCount(); ++i)
    //         ids.push_back(i);

    //     tournament.beginEraseTatamis(std::move(ids));
    //     while (tatamis.tatamiCount() > mCount)
    //         mOldContents.push(tatamis.popTatami());
    //     tournament.endEraseTatamis();
    // }
}

void SetTatamiCountAction::undoImpl(TournamentStore & tournament) {
    // TODO: Implement
    // auto & tatamis = tournament.getTatamis();

    // if (tatamis.tatamiCount() > mOldCount) {
    //     std::vector<size_t> ids;
    //     for (size_t i = mOldCount; i < tatamis.tatamiCount(); ++i)
    //         ids.push_back(i);

    //     tournament.beginEraseTatamis(std::move(ids));
    //     while (tatamis.tatamiCount() > mOldCount)
    //         tatamis.popTatami();
    //     tournament.endEraseTatamis();
    // }

    // if (tatamis.tatamiCount() < mOldCount) {
    //     std::vector<size_t> ids;
    //     for (size_t i = tatamis.tatamiCount(); i < mOldCount; ++i)
    //         ids.push_back(i);

    //     tournament.beginAddTatamis(std::move(ids));
    //     while (tatamis.tatamiCount() < mOldCount) {
    //         tatamis.recoverTatami(mOldContents.top());
    //         mOldContents.pop();
    //     }
    //     tournament.endAddTatamis();
    // }
}

std::unique_ptr<Action> SetTatamiCountAction::freshClone() const {
    return std::make_unique<SetTatamiCountAction>(mCount);
}

std::unique_ptr<Action> SetTatamiLocationAction::freshClone() const {
    return std::make_unique<SetTatamiLocationAction>(mBlock, mLocation);
}

std::string SetTatamiCountAction::getDescription() const {
    return "Set tatami count";
}

std::string SetTatamiLocationAction::getDescription() const {
    return "Set tatami location";
}

