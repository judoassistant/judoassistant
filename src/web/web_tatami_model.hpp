#pragma once

#include <vector>

#include "core/core.hpp"
#include "core/id.hpp"
#include "core/stores/tatami/location.hpp"

class TournamentStore;
enum class MatchType;

class WebTatamiModel {
public:
    static constexpr unsigned int DISPLAY_COUNT = 5; // The minimum number of matches to keep loaded

    WebTatamiModel(const TournamentStore &tournament, TatamiLocation tatami);
    const std::list<CombinedId>& getMatches() const;
    const std::list<CombinedId>& getInsertedMatches() const;
    bool matchesChanged() const;
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

    std::unordered_map<CombinedId, size_t> mLoadedMatches; // Matches loaded and their loading time
    std::unordered_set<PositionId> mLoadedGroups; // Blocks loaded

    std::list<std::pair<CombinedId, size_t>> mUnfinishedLoadedMatches; // Unfinished (and loaded) matches and loading time
    std::unordered_set<CombinedId> mUnfinishedLoadedMatchesSet;

    bool mMatchesChanged;
    bool mChanged;
    std::list<CombinedId> mMatches;
    std::list<CombinedId> mInsertedMatches;
};

