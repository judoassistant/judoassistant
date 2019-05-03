#include "core/draw_systems/draw_system.hpp"
#include "core/draw_systems/draw_systems.hpp"

// TODO: Have a better way of checking if two draw systems are the same. This
// is currently done by name which is not ideal

const std::vector<std::unique_ptr<DrawSystem>> & DrawSystem::getDrawSystems() {
    static std::vector<std::unique_ptr<DrawSystem>> systems;

    if (systems.empty()) {
        systems.push_back(std::make_unique<PoolDrawSystem>());
        systems.push_back(std::make_unique<DoublePoolDrawSystem>());
        systems.push_back(std::make_unique<BestOfThreeDrawSystem>());
        systems.push_back(std::make_unique<KnockoutDrawSystem>());
    }

    return systems;
}
