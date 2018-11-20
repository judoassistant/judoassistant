#include "version.hpp"
#include <sstream>

ApplicationVersion ApplicationVersion::current() {
    ApplicationVersion res;
    res.mVersionMajor = VERSION_MAJOR;
    res.mVersionMinor = VERSION_MINOR;
    res.mVersionPatch = VERSION_PATCH;

    return std::move(res);
}

bool ApplicationVersion::isBackwardsCompatible(const ApplicationVersion &other) const {
    return mVersionMajor == other.mVersionMajor;
}

std::string ApplicationVersion::toString() const {
    std::stringstream ss;
    ss << mVersionMajor << "." << mVersionMinor << "." << mVersionPatch;
    return ss.str();
}
