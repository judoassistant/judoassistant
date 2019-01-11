#pragma once

#include "draw_systems/draw_system.hpp"
#include "draw_systems/knockout_draw_system.hpp"
#include "draw_systems/pool_draw_system.hpp"

class DrawSystems {
public:
    static const std::vector<std::unique_ptr<DrawSystem>> & getDrawSystems();
private:
};
