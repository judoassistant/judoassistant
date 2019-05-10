#include "core/stores/preferences_store.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/draw_systems/best_of_three_draw_system.hpp"
#include "core/draw_systems/best_of_three_draw_system.hpp"
#include "core/draw_systems/double_pool_draw_system.hpp"
#include "core/draw_systems/pool_draw_system.hpp"

PreferencesStore::PreferencesStore() {
    mPreferredDrawSystems.emplace_back(1, std::make_unique<BestOfThreeDrawSystem>());
    mPreferredDrawSystems.emplace_back(3, std::make_unique<PoolDrawSystem>());
    mPreferredDrawSystems.emplace_back(7, std::make_unique<DoublePoolDrawSystem>());
}

PreferencesStore::PreferencesStore(const PreferencesStore &other) {
    for (const auto &p : other.mPreferredDrawSystems)
        mPreferredDrawSystems.emplace_back(p.first, p.second->clone());
}

const std::unique_ptr<const DrawSystem>& PreferencesStore::getPreferredDrawSystem(std::size_t size) const {
    auto it = mPreferredDrawSystems.begin();
    while (true) {
        auto next = std::next(it);
        if (next == mPreferredDrawSystems.end())
            break;
        if (next->playerLowerLimit > size)
            break;
        it = next;
    }
    return it->drawSystem;
}

