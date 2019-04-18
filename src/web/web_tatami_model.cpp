#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "web/web_tatami_model.hpp"
#include "web/web_tournament_store.hpp"

WebTatamiModel::WebTatamiModel(const TournamentStore &tournament, size_t index)
    : mTournament(tournament)
    , mIndex(index)
    , mResetting(true)
{
    flush();
}

const std::vector<std::pair<CategoryId, MatchId>>& WebTatamiModel::getMatches() const {
    return mMatches;
}

const std::vector<std::pair<CategoryId, MatchId>>& WebTatamiModel::getInsertedMatches() const {
    return mInsertedMatches;
}

void WebTatamiModel::changeMatches(const TournamentStore &tournament, CategoryId categoryId, const std::vector<MatchId> &matchIds) {
    if (mResetting)
        return;

}

void WebTatamiModel::changeTatamis(const TournamentStore &tournament, const std::vector<BlockLocation> &locations, const std::vector<std::pair<CategoryId, MatchType>> &blocks) {
    if (mResetting)
        return;
}

void WebTatamiModel::clearChanges() {
    mInsertedMatches.clear();
}

void WebTatamiModel::reset() {
    assert(mResetting);

    mMatches.clear();
    mInsertedMatches.clear();

    mLoadedMatches.clear();
    mLoadedGroups.clear();

    mUnfinishedMatches.clear();
    mUnfinishedMatchesSet.clear();

    mResetting = false;
}

void WebTatamiModel::loadBlocks() {
    if (mMatches.size() >= MATCH_COUNT)
        return;
}

void WebTatamiModel::flush() {
    if (mResetting)
        reset();
    loadBlocks();
}
