#include "core/draw_systems/draw_system.hpp"
#include "core/log.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "web/web_tatami_model.hpp"
#include "web/web_tournament_store.hpp"

WebTatamiModel::WebTatamiModel(const TournamentStore &tournament, TatamiLocation tatami)
    : mTournament(tournament)
    , mTatami(tatami)
    , mResetting(true)
    , mMatchesChanged(true)
    , mChanged(true)
{
    flush();
}

const std::list<CombinedId>& WebTatamiModel::getMatches() const {
    assert(!mResetting);
    return mMatches;
}

const std::list<CombinedId>& WebTatamiModel::getInsertedMatches() const {
    assert(!mResetting);
    return mInsertedMatches;
}

void WebTatamiModel::changeMatches(const TournamentStore &tournament, CategoryId categoryId, const std::vector<MatchId> &matchIds) {
    if (mResetting)
        return;

    const auto &category = mTournament.getCategory(categoryId);

    for (auto matchId: matchIds) {
        CombinedId combinedId(categoryId, matchId);
        auto it = mLoadedMatches.find(combinedId);
        if (it == mLoadedMatches.end())
            continue;

        const auto loadingTime = it->second;
        const auto &match = category.getMatch(matchId);

        bool wasFinished = (mUnfinishedLoadedMatchesSet.find(combinedId) == mUnfinishedLoadedMatchesSet.end());
        bool isFinished = (match.getStatus() == MatchStatus::FINISHED);

        if (isFinished && !wasFinished) {
            // Remove from unfinished matches
            for (auto i = mUnfinishedLoadedMatches.begin(); i != mUnfinishedLoadedMatches.end(); ++i) {
                const CombinedId &curId = i->first;
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
                auto curLoadingTime = i->second;
                if (curLoadingTime > loadingTime)
                    break;
            }

            mUnfinishedLoadedMatches.insert(i, std::pair(combinedId, loadingTime));
            mUnfinishedLoadedMatchesSet.insert(combinedId);
        }
    }
}

void WebTatamiModel::changeTatamis(const TournamentStore &tournament, const std::vector<BlockLocation> &locations, const std::vector<std::pair<CategoryId, MatchType>> &blocks) {
    if (mResetting)
        return;

    const auto &tatami = mTournament.getTatamis().at(mTatami);

    for (const auto &location : locations) {
        if (mTatami != location.sequentialGroup.concurrentGroup.tatami) continue;

        mChanged = true;

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

    mInsertedMatches.clear();
    mMatchesChanged = false;
    mChanged = false;
}

void WebTatamiModel::reset() {
    assert(mResetting);

    mLoadedMatches.clear();
    mLoadedGroups.clear();

    mUnfinishedLoadedMatches.clear();
    mUnfinishedLoadedMatchesSet.clear();

    mResetting = false;

    mMatchesChanged = true;
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

        for (const auto &combinedId : group.getMatches()) {
            auto &category = mTournament.getCategory(combinedId.getCategoryId());
            auto &match = category.getMatch(combinedId.getMatchId());
            auto loadingTime = mLoadedMatches.size();

            mLoadedMatches[combinedId] = loadingTime;

            if (match.getStatus() == MatchStatus::FINISHED)
                continue;

            mUnfinishedLoadedMatches.emplace_back(combinedId, loadingTime);
            mUnfinishedLoadedMatchesSet.insert(combinedId);
        }
    }
}

void WebTatamiModel::flush() {
    if (mResetting)
        reset();
    loadBlocks();

    // Update matches and inserted matches to be the front of mUnfinishedMatches
    size_t i = 0;

    auto it = mMatches.begin();
    auto jt = mUnfinishedLoadedMatches.begin();
    while (true) {
        if (it == mMatches.end() && jt == mUnfinishedLoadedMatches.end())
            break;

        if (it == mMatches.end() && i == DISPLAY_COUNT)
            break;

        // Copy matches from mUnfinishedMatches
        if (it == mMatches.end() && jt != mUnfinishedLoadedMatches.end()) {
            const CombinedId &combinedId = jt->first;
            mMatches.insert(it, combinedId);
            mInsertedMatches.push_back(combinedId);
            mMatchesChanged = true;
            ++i;
            ++jt;
            continue;
        }

        if (jt == mUnfinishedLoadedMatches.end() || i >= DISPLAY_COUNT) { // This match should not be loaded
            auto next = std::next(it);
            mMatches.erase(it);
            it = next;
            mMatchesChanged = true;
            continue;
        }

        // Both iterators != end
        const CombinedId &combinedId = jt->first;
        if (*it == combinedId) {
            // Matches are the same. Continue
            ++it;
            ++i;
            ++jt;
        }
        else {
            // Matches are different. Erase from matches and continue
            auto next = std::next(it);
            mMatches.erase(it);
            it = next;
            mMatchesChanged = true;
            continue;
        }
    }
}

bool WebTatamiModel::matchesChanged() const {
    assert(!mResetting);
    return mMatchesChanged;
}

bool WebTatamiModel::changed() const {
    assert(!mResetting);
    return mChanged;
}

