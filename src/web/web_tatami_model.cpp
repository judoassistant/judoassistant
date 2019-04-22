#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "web/web_tatami_model.hpp"
#include "web/web_tournament_store.hpp"

WebTatamiModel::WebTatamiModel(const TournamentStore &tournament, TatamiLocation tatami)
    : mTournament(tournament)
    , mTatami(tatami)
    , mResetting(true)
{
    flush();
}

const std::vector<std::pair<CategoryId, MatchId>>& WebTatamiModel::getMatches() const {
    assert(!mResetting);
    return {};
}

const std::vector<std::pair<CategoryId, MatchId>>& WebTatamiModel::getInsertedMatches() const {
    assert(!mResetting);
    return {};
}

void WebTatamiModel::changeMatches(const TournamentStore &tournament, CategoryId categoryId, const std::vector<MatchId> &matchIds) {
    if (mResetting)
        return;

    const auto &category = mTournament.getCategory(categoryId);

    for (auto matchId: matchIds) {
        auto combinedId = std::make_pair(categoryId, matchId);
        auto it = mLoadedMatches.find(combinedId);
        if (it == mLoadedMatches.end())
            continue;

        const auto loadingTime = it->second;
        const auto &match = category.getMatch(matchId);

        bool wasFinished = (mUnfinishedLoadedMatchesSet.find(combinedId) == mUnfinishedLoadedMatchesSet.end());
        bool isFinished = (match.getStatus() == MatchStatus::FINISHED);

        if (isFinished && !wasFinished) {
            // Remove from unfinished matches
            // Probably at the front of the list
            for (auto i = mUnfinishedLoadedMatches.begin(); i != mUnfinishedLoadedMatches.end(); ++i) {
                auto curId = std::make_pair(std::get<0>(*i), std::get<1>(*i));
                if (curId == combinedId) {
                    mUnfinishedLoadedMatches.erase(i);
                    break;
                }
            }

            mUnfinishedLoadedMatchesSet.erase(combinedId);
        }
        else if(!isFinished && wasFinished) {
            // Add to unfinished matches
            auto i = mUnfinishedLoadedMatches.begin();
            for (;i != mUnfinishedLoadedMatches.end(); ++i) {
                auto curLoadingTime = std::get<2>(*i);
                if (curLoadingTime > loadingTime)
                    break;
            }

            mUnfinishedLoadedMatches.insert(i, std::make_tuple(categoryId, matchId, loadingTime));
            mUnfinishedLoadedMatchesSet.insert(combinedId);
        }
    }

}

void WebTatamiModel::changeTatamis(const TournamentStore &tournament, const std::vector<BlockLocation> &locations, const std::vector<std::pair<CategoryId, MatchType>> &blocks) {
    if (mResetting)
        return;

    const auto &tatami = mTournament.getTatamis().at(mTatami);

    for (const auto &location : locations) {
        if (!mTatami.equiv(location.sequentialGroup.concurrentGroup.tatami)) continue;

        auto handle = location.sequentialGroup.concurrentGroup.handle;
        if (mLoadedGroups.find(handle.id) != mLoadedGroups.end()) {
            mResetting = true;
            return;
        }

        if (tatami.containsGroup(handle) && tatami.getIndex(handle) < mLoadedGroups.size()) {
            mResetting = true;
            return;
        }
    }
}

void WebTatamiModel::clearChanges() {
    assert(!mResetting);
}

void WebTatamiModel::reset() {
    assert(mResetting);

    mLoadedMatches.clear();
    mLoadedGroups.clear();

    mUnfinishedLoadedMatches.clear();
    mUnfinishedLoadedMatchesSet.clear();

    mResetting = false;
}

void WebTatamiModel::loadBlocks() {
    assert(!mResetting);

    const auto &tatami = mTournament.getTatamis().at(mTatami);

    while (mUnfinishedLoadedMatches.size() < DISPLAY_COUNT) {
        if (mLoadedGroups.size() == tatami.groupCount())
            break;

        auto handle = tatami.getHandle(mLoadedGroups.size());
        mLoadedGroups.insert(handle.id);
        const auto &group = tatami.at(handle);

        for (const auto &p : group.getMatches()) {
            auto &category = mTournament.getCategory(p.first);
            auto &match = category.getMatch(p.second);
            auto loadingTime = mLoadedMatches.size();
            auto combinedId = match.getCombinedId();

            mLoadedMatches[combinedId] = loadingTime;

            if (match.getStatus() == MatchStatus::FINISHED)
                continue;

            mUnfinishedLoadedMatches.emplace_back(match.getCategory(), match.getId(), loadingTime);
            mUnfinishedLoadedMatchesSet.insert(combinedId);
        }
    }
}

void WebTatamiModel::flush() {
    if (mResetting)
        reset();
    loadBlocks();
}

bool WebTatamiModel::didRemoveMatches() const {
    assert(!mResetting);
    return true;
}

