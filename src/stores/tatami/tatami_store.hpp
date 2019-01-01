#pragma once

#include "core.hpp"
#include "id.hpp"
#include "position_manager.hpp"
#include "serialize.hpp"
#include "stores/tatami/concurrent_block_group.hpp"

enum class MatchType;
class TournamentStore;

class TatamiStore {
public:
    void eraseGroup(PositionHandle handle);
    bool containsGroup(PositionHandle handle) const;
    PositionHandle getHandle(size_t index) const;
    size_t getIndex(PositionHandle handle) const;
    size_t groupCount() const;
    SequentialGroupLocation generateLocation(TournamentStore & tournament, size_t index);

    ConcurrentBlockGroup & operator[](PositionHandle handle);

    ConcurrentBlockGroup & at(PositionHandle handle) const;
    ConcurrentBlockGroup & at(size_t index) const;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mGroups);
    }
private:
    PositionManager<ConcurrentBlockGroup> mGroups;
};


