#include "stores/tatami/location.hpp"

// TODO: Consider making compares and hashes ignore index

std::ostream &operator<<(std::ostream &out, const TatamiLocation &location) {
    return out << location.handle;
}

std::ostream &operator<<(std::ostream &out, const ConcurrentGroupLocation &location) {
    return out << "(" << location.handle << "; " << location.tatami << ")";
}

std::ostream &operator<<(std::ostream &out, const SequentialGroupLocation &location) {
    return out << "(" << location.handle << "; " << location.concurrentGroup.handle << "; " << location.concurrentGroup.tatami << ")";
}

std::ostream &operator<<(std::ostream &out, const BlockLocation &location) {
    return out << "(" << location.pos << "; " << location.sequentialGroup.handle << "; " << location.sequentialGroup.concurrentGroup.handle << "; " << location.sequentialGroup.concurrentGroup.tatami << ")";
}

