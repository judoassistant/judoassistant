#pragma once

#include <vector>

#include "core/core.hpp"
#include "core/id.hpp"
#include "core/stores/tatami/location.hpp"

class TournamentStore;
enum class MatchType;

// TODO: Try to share code base with ui tatami matches model
class WebTatamiModel {
public:
    static constexpr unsigned int DISPLAY_COUNT = 5; // The minimum number of matches to keep loaded

    WebTatamiModel(const TournamentStore &tournament, TatamiLocation tatami);
    const std::list<std::pair<CategoryId, MatchId>>& getMatches() const;
    const std::list<std::pair<CategoryId, MatchId>>& getInsertedMatches() const;
    bool changed() const;

    void changeMatches(const TournamentStore &tournament, CategoryId categoryId, const std::vector<MatchId> &matchIds);
    void changeTatamis(const TournamentStore &tournament, const std::vector<BlockLocation> &locations, const std::vector<std::pair<CategoryId, MatchType>> &blocks);

    void flush();
    void clearChanges();

protected:
    void reset();
    void loadBlocks();
private:
    const TournamentStore &mTournament;
    TatamiLocation mTatami;

    bool mResetting;

    std::unordered_map<std::pair<CategoryId, MatchId>, size_t> mLoadedMatches; // Matches loaded and their loading time
    std::unordered_set<PositionId> mLoadedGroups; // Blocks loaded

    std::list<std::tuple<CategoryId, MatchId, size_t>> mUnfinishedLoadedMatches; // Unfinished (and loaded) matches and loading time
    std::unordered_set<std::pair<CategoryId, MatchId>> mUnfinishedLoadedMatchesSet;

    bool mDidRemoveMatches;
    std::list<std::pair<CategoryId, MatchId>> mMatches;
    std::list<std::pair<CategoryId, MatchId>> mInsertedMatches;
};

