#pragma once

#include <algorithm>
#include <ostream>
#include <vector>
#include <unordered_map>

#include "core/core.hpp"
#include "core/id.hpp"
#include "core/serialize.hpp"
#include "core/hash.hpp"

struct PositionHandle {
    PositionId id;
    size_t index;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(id, index);
    }

    bool operator==(const PositionHandle &other) const {
        return id == other.id;
    }

    bool operator!=(const PositionHandle &other) const {
        return id != other.id;
    }
};

namespace std {
    template <>
    struct hash<PositionHandle> {
        size_t operator()(const PositionHandle &v) const {
            return std::hash<PositionId>()(v.id);
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

    void insert(PositionHandle handle) {
        auto it = mElements.find(handle.id);
        if (it != mElements.end())
            return;

        mIds.insert(mIds.begin() + std::min(mIds.size(), handle.index), handle.id);
        mElements.insert({handle.id, T()});
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
        ar(mIds, mElements);
    }

private:
    std::vector<PositionId> mIds;
    std::unordered_map<PositionId, T> mElements;
};
