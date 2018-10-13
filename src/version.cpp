#include "version.hpp"
#include <sstream>

std::string getApplicationVersion() {
    std::stringstream ss;
    ss << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH;
    return ss.str();
}

