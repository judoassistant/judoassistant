#include <chrono>

#include "core/random.hpp"

unsigned int getSeed() {
    return std::chrono::system_clock::now().time_since_epoch().count();
}
