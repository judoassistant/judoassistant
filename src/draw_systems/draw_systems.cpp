#include "draw_systems/draw_systems.hpp"

const std::vector<std::unique_ptr<DrawSystem>> & DrawSystems::getDrawSystems() {
    static std::vector<std::unique_ptr<DrawSystem>> systems;

    if (systems.empty()) {
        systems.push_back(std::make_unique<PoolDrawSystem>());
        systems.push_back(std::make_unique<KnockoutDrawSystem>());
    }

    return systems;
}
