#pragma once

#include <unordered_map>
#include <vector>
#include <algorithm>
#include "id.hpp"
#include "core.hpp"
#include "serialize.hpp"

struct PositionHandle {
    PositionId id;
    size_t index;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::make_nvp("id", id));
        ar(cereal::make_nvp("index", index));
    }
};

std::ostream &operator<<(std::ostream &out, const PositionHandle &handle);

template <typename T>
class PositionManager {
public:

    T & get(PositionHandle handle) {
        auto it = mElements.find(handle.id);
        if (it != mElements.end())
            return it->second;

        mIds.insert(mIds.begin() + std::min(mIds.size(), handle.index), handle.id);
        return mElements[handle.id];
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

    size_t size() const {
        return mIds.size();
    }

    bool containsId(PositionId id) const {
        return mElements.find(id) != mElements.end();
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
