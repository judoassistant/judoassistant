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
    mPreferredDrawSystems.emplace_back(7, DrawSystemIdentifier::DOUBLE_POOL);
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
    assert(size > 0);
    auto it = std::upper_bound(mPreferredDrawSystems.begin(), mPreferredDrawSystems.end(), size, Comparator());
    return std::prev(it)->drawSystem;
}

