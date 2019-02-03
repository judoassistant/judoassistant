#include "core/position_manager.hpp"

std::ostream &operator<<(std::ostream &out, const PositionHandle &handle) {
    return out << "(" << handle.id << "; " << handle.index << ")";
}
