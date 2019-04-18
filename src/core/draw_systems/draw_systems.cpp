#include "core/draw_systems/draw_systems.hpp"

// TODO: Have a better way of checking if two draw systems are the same. This
// is currently done by name which is not ideal
// TODO: Consider having a results changed signal

const std::vector<std::unique_ptr<DrawSystem>> & DrawSystems::getDrawSystems() {
    static std::vector<std::unique_ptr<DrawSystem>> systems;

    if (systems.empty()) {
        systems.push_back(std::make_unique<PoolDrawSystem>());
        systems.push_back(std::make_unique<KnockoutDrawSystem>());
        systems.push_back(std::make_unique<BestOfThreeDrawSystem>());
    }

    return systems;
}
