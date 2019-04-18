#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "web/web_tatami_model.hpp"
#include "web/web_tournament_store.hpp"

WebTatamiModel::WebTatamiModel(const TournamentStore &tournament, size_t index) {

}

std::vector<std::pair<CategoryId, MatchId>> WebTatamiModel::getMatches() const {
    return {};
}

std::vector<std::pair<CategoryId, MatchId>> WebTatamiModel::getInsertedMatches() const {
    return {};
}

void WebTatamiModel::changeMatches(const TournamentStore &tournament, CategoryId categoryId, const std::vector<MatchId> &matchIds) {

}

void WebTatamiModel::changeTatamis(const TournamentStore &tournament, const std::vector<BlockLocation> &locations, const std::vector<std::pair<CategoryId, MatchType>> &blocks) {

}

