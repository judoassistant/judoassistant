#include "core/stores/preferences_store.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/draw_systems/best_of_three_draw_system.hpp"
#include "core/draw_systems/best_of_three_draw_system.hpp"
#include "core/draw_systems/double_pool_draw_system.hpp"
#include "core/draw_systems/pool_draw_system.hpp"

DrawSystemPreference::DrawSystemPreference(std::size_t playerLowerLimit, DrawSystemIdentifier drawSystem)
    : playerLowerLimit(playerLowerLimit)
    , drawSystem(drawSystem)
{}

PreferencesStore::PreferencesStore() {
    mPreferredDrawSystems.emplace_back(1, DrawSystemIdentifier::BEST_OF_THREE);
    mPreferredDrawSystems.emplace_back(3, DrawSystemIdentifier::POOL);
    mPreferredDrawSystems.emplace_back(6, DrawSystemIdentifier::DOUBLE_POOL);
    mScoreboardStyle = ScoreboardStylePreference::NATIONAL;
    mMatchCardStyle = MatchCardStylePreference::NATIONAL;
}

const std::vector<DrawSystemPreference>& PreferencesStore::getPreferredDrawSystems() const {
    return mPreferredDrawSystems;
}

std::vector<DrawSystemPreference>& PreferencesStore::getPreferredDrawSystems() {
    return mPreferredDrawSystems;
}

struct Comparator {
    bool operator()(std::size_t value, const DrawSystemPreference &preference) {
        return value < preference.playerLowerLimit;
    }
};

DrawSystemIdentifier PreferencesStore::getPreferredDrawSystem(std::size_t size) const {
    size_t i = mPreferredDrawSystems.size() - 1;
    while (i > 0 && mPreferredDrawSystems[i].playerLowerLimit > size)
        --i;

    return mPreferredDrawSystems[i].drawSystem;
}

ScoreboardStylePreference PreferencesStore::getScoreboardStyle() const {
    return mScoreboardStyle;
}

void PreferencesStore::setScoreboardStyle(ScoreboardStylePreference style) {
    mScoreboardStyle = style;
}

MatchCardStylePreference PreferencesStore::getMatchCardStyle() const {
    return mMatchCardStyle;
}

void PreferencesStore::setMatchCardStyle(MatchCardStylePreference style) {
    mMatchCardStyle = style;
}

