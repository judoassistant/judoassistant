#pragma once

#include <ostream>

#include "core.hpp"
#include "hash.hpp"
#include "position_manager.hpp"
#include "serialize.hpp"

struct NewTatamiLocation { // TODO: Rename to TatamiLocation. Temporary name used for ambiguity purposes
    PositionHandle handle;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(handle);
    }

    bool operator==(const NewTatamiLocation &other) const {
        return handle == other.handle;
    }

    bool equiv(const NewTatamiLocation &other) const {
        return handle.equiv(other.handle);
    }
};

struct ConcurrentGroupLocation {
    NewTatamiLocation tatami;
    PositionHandle handle;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(tatami, handle);
    }

    bool operator==(const ConcurrentGroupLocation &other) const {
        return handle == other.handle && tatami == other.tatami;
    }

    bool equiv(const ConcurrentGroupLocation &other) const {
        return tatami.equiv(other.tatami) && handle.equiv(other.handle);
    }
};

struct SequentialGroupLocation {
    ConcurrentGroupLocation concurrentGroup;
    PositionHandle handle;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(concurrentGroup, handle);
    }

    bool operator==(const SequentialGroupLocation &other) const {
        return handle == other.handle && concurrentGroup == other.concurrentGroup;
    }

    bool equiv(const SequentialGroupLocation &other) const {
        return concurrentGroup.equiv(other.concurrentGroup) && handle.equiv(other.handle);
    }
};

struct BlockLocation {
    SequentialGroupLocation sequentialGroup;
    size_t pos;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(sequentialGroup, pos);
    }

    bool operator==(const BlockLocation &other) const {
        return pos == other.pos && sequentialGroup == other.sequentialGroup;
    }

    PositionHandle getTatamiHandle() const {
        return sequentialGroup.concurrentGroup.tatami.handle;
    }

    PositionHandle getConcurrentGroupHandle() const {
        return sequentialGroup.concurrentGroup.handle;
    }

    PositionHandle getSequentialGroupHandle() const {
        return sequentialGroup.handle;
    }
};

std::ostream &operator<<(std::ostream &out, const NewTatamiLocation &location);
std::ostream &operator<<(std::ostream &out, const ConcurrentGroupLocation &location);
std::ostream &operator<<(std::ostream &out, const SequentialGroupLocation &location);
std::ostream &operator<<(std::ostream &out, const BlockLocation &location);

namespace std {
    template <>
    struct hash<NewTatamiLocation> {
        size_t operator()(const NewTatamiLocation &v) const {
            std::hash<PositionHandle> hasher;
            return hasher(v.handle);
        }
    };

    template <>
    struct hash<ConcurrentGroupLocation> {
        size_t operator()(const ConcurrentGroupLocation &v) const {
            size_t seed = 0;
            hash_combine(seed, v.handle);
            hash_combine(seed, v.tatami);
            return seed;
        }
    };

    template <>
    struct hash<SequentialGroupLocation> {
        size_t operator()(const SequentialGroupLocation &v) const {
            size_t seed = 0;
            hash_combine(seed, v.handle);
            hash_combine(seed, v.concurrentGroup);
            return seed;
        }
    };

    template <>
    struct hash<BlockLocation> {
        size_t operator()(const BlockLocation &v) const {
            size_t seed = 0;
            hash_combine(seed, v.pos);
            hash_combine(seed, v.sequentialGroup);
            return seed;
        }
    };
};

