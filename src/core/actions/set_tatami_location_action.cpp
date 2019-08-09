#include "core/actions/set_tatami_location_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/tatami/tatami_list.hpp"
#include "core/stores/tournament_store.hpp"

SetTatamiLocationAction::SetTatamiLocationAction(std::pair<CategoryId, MatchType> block, std::optional<BlockLocation> location)
    : mBlock(block)
    , mLocation(location)
{}

void SetTatamiLocationAction::redoImpl(TournamentStore & tournament) {
    mPositionSet = false;

    if (!tournament.containsCategory(mBlock.first)) // does the category exist?
        return;

    auto &category = tournament.getCategory(mBlock.first);
    auto &tatamis = tournament.getTatamis();

    if (mLocation.has_value()) { // Perform sanity checks
        if (!tatamis.containsTatami(mLocation->getTatamiHandle())) // Does the tatami exist?
            return;

        if (mBlock.second == MatchType::FINAL && !category.getDrawSystem().hasFinalBlock()) // Are we moving a non-existing final block onto a tatami?
            return;

        const auto &tatami = tatamis.at(mLocation->getTatamiHandle());
        if (tatami.containsGroup(mLocation->getConcurrentGroupHandle())) { // Are we below the MAX_GROUP_COUNT for the concurrent group destination
            const auto &concurrentGroup = tatami.at(mLocation->getConcurrentGroupHandle());
            if (!concurrentGroup.containsGroup(mLocation->getSequentialGroupHandle()) && concurrentGroup.groupCount() == ConcurrentBlockGroup::MAX_GROUP_COUNT)
                return;
        }
    }

    mOldLocation = category.getLocation(mBlock.second);
    mPositionSet = true;

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
    if (!mPositionSet)
        return;

    auto &tatamis = tournament.getTatamis();
    auto &category = tournament.getCategory(mBlock.first);

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

std::unique_ptr<Action> SetTatamiLocationAction::freshClone() const {
    return std::make_unique<SetTatamiLocationAction>(mBlock, mLocation);
}

std::string SetTatamiLocationAction::getDescription() const {
    return "Set tatami location";
}

