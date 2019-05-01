#include "core/actions/set_tatami_count_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/tatami/tatami_list.hpp"
#include "core/stores/tournament_store.hpp"

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
        tournament.endAddTatamis(locations);
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
        tournament.endEraseTatamis(locations);
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
        tournament.endEraseTatamis(locations);
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
                const auto &concurrentHandle = tatami.getHandle(i);
                const auto &concurrentGroup = tatami.at(concurrentHandle);
                ConcurrentGroupLocation concurrentLocation = {tatamiLocation, concurrentHandle};
                for (size_t j = 0; j < concurrentGroup.groupCount(); ++j) {
                    const auto &sequentialHandle = concurrentGroup.getHandle(j);
                    const auto &sequentialGroup = concurrentGroup.at(sequentialHandle);
                    SequentialGroupLocation sequentialLocation = {concurrentLocation, sequentialHandle};

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
        tournament.endAddTatamis(locations);
        tournament.changeCategories(std::vector(categories.begin(), categories.end()));
    }
}

std::unique_ptr<Action> SetTatamiCountAction::freshClone() const {
    return std::make_unique<SetTatamiCountAction>(mLocations);
}

std::string SetTatamiCountAction::getDescription() const {
    return "Set tatami count";
}

