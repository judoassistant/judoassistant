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
    WebTatamiModel(const TournamentStore &tournament, size_t index);
    const std::vector<std::pair<CategoryId, MatchId>>& getMatches() const;
    const std::vector<std::pair<CategoryId, MatchId>>& getInsertedMatches() const;

    void changeMatches(const TournamentStore &tournament, CategoryId categoryId, const std::vector<MatchId> &matchIds);
    void changeTatamis(const TournamentStore &tournament, const std::vector<BlockLocation> &locations, const std::vector<std::pair<CategoryId, MatchType>> &blocks);

    void clearChanges();

private:
    bool mResetting;
    std::vector<std::pair<CategoryId, MatchId>> mMatches;
    std::vector<std::pair<CategoryId, MatchId>> mInsertedMatches;
};

