#pragma once

#include "core/core.hpp"
#include "core/id.hpp"
#include "core/position_manager.hpp"
#include "core/serialize.hpp"
#include "core/stores/tatami/concurrent_block_group.hpp"

class TournamentStore;
struct TatamiLocation;

class TatamiStore {
public:
    void eraseGroup(PositionHandle handle);
    bool containsGroup(PositionHandle handle) const;
    PositionHandle getHandle(size_t index) const;
    size_t getIndex(PositionHandle handle) const;
    size_t groupCount() const;
    SequentialGroupLocation generateLocation(TatamiLocation location, size_t index);

    ConcurrentBlockGroup & operator[](PositionHandle handle);

    ConcurrentBlockGroup & at(PositionHandle handle);
    const ConcurrentBlockGroup & at(PositionHandle handle) const;

    const ConcurrentBlockGroup & at(ConcurrentGroupLocation location) const;
    ConcurrentBlockGroup & at(ConcurrentGroupLocation location);

    const ConcurrentBlockGroup & at(size_t index) const;
    ConcurrentBlockGroup & at(size_t index);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mGroups);
    }
private:
    PositionManager<ConcurrentBlockGroup> mGroups;
};


