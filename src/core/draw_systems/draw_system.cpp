#include "core/draw_systems/draw_system.hpp"
#include "core/draw_systems/draw_systems.hpp"

const std::vector<std::unique_ptr<const DrawSystem>> & DrawSystem::getDrawSystems() {
    static std::vector<std::unique_ptr<const DrawSystem>> systems;

    if (systems.empty()) {
        systems.push_back(std::make_unique<const PoolDrawSystem>());
        systems.push_back(std::make_unique<const DoublePoolDrawSystem>());
        systems.push_back(std::make_unique<const BestOfThreeDrawSystem>());
        systems.push_back(std::make_unique<const KnockoutDrawSystem>());
    }

    return systems;
}

std::unique_ptr<DrawSystem> DrawSystem::getDrawSystem(DrawSystemIdentifier identifier) {
    return getDrawSystems()[DrawSystem::getDrawSystemIndex(identifier)]->clone();
}

std::size_t DrawSystem::getDrawSystemIndex(DrawSystemIdentifier identifier) {
    static std::unordered_map<DrawSystemIdentifier, std::size_t> systems;

    if (systems.empty()) {
        std::size_t i = 0;
        for (const auto &system : DrawSystem::getDrawSystems())
            systems[system->getIdentifier()] = i++;
    }

    return systems.at(identifier);
}

