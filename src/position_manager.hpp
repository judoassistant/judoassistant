#pragma once

#include <algorithm>
#include <ostream>
#include <vector>
#include <unordered_map>

#include "core.hpp"
#include "id.hpp"
#include "serialize.hpp"
#include "hash.hpp"

struct PositionHandle {
    PositionId id;
    size_t index;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::make_nvp("id", id));
        ar(cereal::make_nvp("index", index));
    }

    bool operator==(const PositionHandle &other) const {
        return id == other.id && index == other.index;
    }

    bool equiv(const PositionHandle &other) const {
        return id == other.id;
    }
};

namespace std {
    template <>
    struct hash<PositionHandle> {
        size_t operator()(const PositionHandle &v) const {
            size_t seed = 0;
            hash_combine(seed, v.id);
            hash_combine(seed, v.index);
            return seed;
        }
    };
};

std::ostream &operator<<(std::ostream &out, const PositionHandle &handle);

template <typename T>
class PositionManager {
public:

    T & operator[](PositionHandle handle) {
        auto it = mElements.find(handle.id);
        if (it != mElements.end())
            return it->second;

        mIds.insert(mIds.begin() + std::min(mIds.size(), handle.index), handle.id);
        return mElements[handle.id];
    }

    T & at(PositionHandle handle) {
        auto it = mElements.find(handle.id);
        assert(it != mElements.end());
        return it->second;
    }

    T & at(size_t index) {
        assert(index < mElements.size());
        return mElements.at(mIds[index]);
    }

    const T & at(PositionHandle handle) const {
        auto it = mElements.find(handle.id);
        assert(it != mElements.end());
        return it->second;
    }

    const T & at(size_t index) const {
        assert(index < mElements.size());
        return mElements.at(mIds[index]);
    }

    void erase(PositionHandle handle) {
        auto it = std::find(mIds.begin(), mIds.end(), handle.id);
        if (it == mIds.end())
            return;

        mIds.erase(it);
        mElements.erase(handle.id);
    }

    PositionHandle getHandle(size_t index) const {
        assert(index < mIds.size());

        PositionHandle handle;
        handle.index = index;
        handle.id = mIds[index];
        return handle;
    }

    size_t getIndex(PositionHandle handle) const {
        auto it = std::find(mIds.begin(), mIds.end(), handle.id);
        assert (it != mIds.end());
        return std::distance(mIds.begin(), it);
    }

    size_t size() const {
        return mIds.size();
    }

    bool contains(PositionId id) const {
        return mElements.find(id) != mElements.end();
    }

    bool contains(PositionHandle handle) const {
        return contains(handle.id);
    }

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::make_nvp("ids", mIds));
        ar(cereal::make_nvp("elements", mElements));
    }

private:
    std::vector<PositionId> mIds;
    std::unordered_map<PositionId, T> mElements;
};
