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
    static constexpr unsigned int MATCH_COUNT = 5;

    WebTatamiModel(const TournamentStore &tournament, size_t index);
    const std::vector<std::pair<CategoryId, MatchId>>& getMatches() const;
    const std::vector<std::pair<CategoryId, MatchId>>& getInsertedMatches() const;

    void changeMatches(const TournamentStore &tournament, CategoryId categoryId, const std::vector<MatchId> &matchIds);
    void changeTatamis(const TournamentStore &tournament, const std::vector<BlockLocation> &locations, const std::vector<std::pair<CategoryId, MatchType>> &blocks);

    void flush();
    void clearChanges();

protected:
    void reset();
    void loadBlocks();
private:
    const TournamentStore &mTournament;
    size_t mIndex;

    bool mResetting;
    std::vector<std::pair<CategoryId, MatchId>> mMatches;
    std::vector<std::pair<CategoryId, MatchId>> mInsertedMatches;

    std::unordered_map<std::pair<CategoryId, MatchId>, size_t> mLoadedMatches; // Matches loaded and their loading time
    std::unordered_set<PositionId> mLoadedGroups; // Blocks loaded

    std::deque<std::tuple<CategoryId, MatchId, size_t>> mUnfinishedMatches; // Unfinished (and loaded) matches and loading time
    std::unordered_set<std::pair<CategoryId, MatchId>> mUnfinishedMatchesSet;
};

