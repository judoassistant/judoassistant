#include "actions/tatami_actions.hpp"
#include "draw_systems/draw_system.hpp"
#include "rulesets/ruleset.hpp"
#include "stores/tatami/tatami_list.hpp"
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

SetTatamiCountAction::SetTatamiCountAction(const std::vector<TatamiLocation> &locations)
    : mLocations(locations)
{}

SetTatamiCountAction::SetTatamiCountAction(TournamentStore &tournament, size_t count) {
    auto &tatamis = tournament.getTatamis();
    // generate `count` locations even though fewer might be enough
    for (size_t i = 0; i < count; ++i)
        mLocations.push_back(tatamis.generateLocation(i));
}

void SetTatamiCountAction::redoImpl(TournamentStore & tournament) {
    auto & tatamis = tournament.getTatamis();
    mOldCount = tatamis.tatamiCount();

    if (tatamis.tatamiCount() < mLocations.size()) {
        std::vector<TatamiLocation> locations;
        for (size_t i = tatamis.tatamiCount(); i < mLocations.size(); ++i) {
            locations.push_back(mLocations[i]);
        }

        tournament.beginAddTatamis(locations);
        for (TatamiLocation location : locations)
            tatamis.insert(location.handle);
        tournament.endAddTatamis();
    }
    else if (tatamis.tatamiCount() > mLocations.size()) {
        std::vector<TatamiLocation> locations;
        std::unordered_set<CategoryId> categories;
        for (size_t i = mLocations.size(); i < tatamis.tatamiCount(); ++i)
            locations.push_back({tatamis.getHandle(i)});

        tournament.beginEraseTatamis(locations);
        for (TatamiLocation location : locations) {
            // erase the tatami
            TatamiStore tatami = std::move(tatamis.at(location));
            tatamis.eraseTatami(location.handle);

            // update all categories on the tatami
            for (size_t i = 0; i < tatami.groupCount(); ++i) {
                const auto &concurrentGroup = tatami.at(i);
                for (size_t j = 0; j < concurrentGroup.groupCount(); ++j) {
                    const auto &sequentialGroup = concurrentGroup.at(j);

                    for (size_t k = 0; k < sequentialGroup.blockCount(); ++k) {
                        auto block = sequentialGroup.at(k);
                        categories.insert(block.first);

                        tournament.getCategory(block.first).setLocation(block.second, std::nullopt);
                    }
                }
            }

            mErasedTatamis.push_back(std::make_pair(location, std::move(tatami)));
        }
        tournament.endEraseTatamis();
        tournament.changeCategories(std::vector(categories.begin(), categories.end()));
    }
}

void SetTatamiCountAction::undoImpl(TournamentStore & tournament) {
    auto & tatamis = tournament.getTatamis();

    if (tatamis.tatamiCount() > mOldCount) {
        std::vector<TatamiLocation> locations;
        for (size_t i = mOldCount; i < tatamis.tatamiCount(); ++i)
            locations.push_back({tatamis.getHandle(i)});

        tournament.beginEraseTatamis(locations);
        for (TatamiLocation location : locations)
            tatamis.eraseTatami(location.handle);
        tournament.endEraseTatamis();
    }
    else if (tatamis.tatamiCount() < mOldCount) {
        std::vector<TatamiLocation> locations;
        std::unordered_set<CategoryId> categories;
        for (const auto & pair : mErasedTatamis) {
            auto tatamiLocation = pair.first;
            const TatamiStore &tatami = pair.second;

            locations.push_back(tatamiLocation);

            // update all categories on the tatami
            for (size_t i = 0; i < tatami.groupCount(); ++i) {
                const auto &concurrentGroup = tatami.at(i);
                ConcurrentGroupLocation concurrentLocation = {tatamiLocation, i};
                for (size_t j = 0; j < concurrentGroup.groupCount(); ++j) {
                    const auto &sequentialGroup = concurrentGroup.at(j);
                    SequentialGroupLocation sequentialLocation = {concurrentLocation, j};

                    for (size_t k = 0; k < sequentialGroup.blockCount(); ++k) {
                        auto block = sequentialGroup.at(k);
                        BlockLocation blockLocation = {sequentialLocation, k};
                        categories.insert(block.first);

                        tournament.getCategory(block.first).setLocation(block.second, blockLocation);
                    }
                }
            }

        }

        tournament.beginAddTatamis(locations);
        for (auto & pair : mErasedTatamis)
            tatamis[pair.first.handle] = std::move(pair.second);
        mErasedTatamis.clear();
        tournament.endAddTatamis();
        tournament.changeCategories(std::vector(categories.begin(), categories.end()));
    }
}

std::unique_ptr<Action> SetTatamiCountAction::freshClone() const {
    return std::make_unique<SetTatamiCountAction>(mLocations);
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

