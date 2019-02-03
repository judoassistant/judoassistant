#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "core/stores/tatami/tatami_store.hpp"
#include "core/stores/tatami/location.hpp"
#include "core/stores/tournament_store.hpp"

void TatamiStore::eraseGroup(PositionHandle handle) {
    mGroups.erase(handle);
}

bool TatamiStore::containsGroup(PositionHandle handle) const {
    return mGroups.contains(handle);
}

PositionHandle TatamiStore::getHandle(size_t index) const {
    return mGroups.getHandle(index);
}

size_t TatamiStore::getIndex(PositionHandle handle) const {
    return mGroups.getIndex(handle);
}

size_t TatamiStore::groupCount() const {
    return mGroups.size();
}

SequentialGroupLocation TatamiStore::generateLocation(TatamiLocation location, size_t index) {
    PositionHandle conHandle;
    conHandle.id = PositionId::generate(mGroups);
    conHandle.index = index;

    ConcurrentGroupLocation conLoc = {location, conHandle};

    // the new group is empty. Just generate id based on this group
    PositionHandle seqHandle;
    seqHandle.id = PositionId::generate(mGroups);
    seqHandle.index = index;

    return {conLoc, seqHandle};
}

ConcurrentBlockGroup & TatamiStore::operator[](PositionHandle handle) {
    return mGroups[handle];
}

ConcurrentBlockGroup & TatamiStore::at(PositionHandle handle) {
    return mGroups.at(handle);
}

const ConcurrentBlockGroup & TatamiStore::at(PositionHandle handle) const {
    return mGroups.at(handle);
}

const ConcurrentBlockGroup & TatamiStore::at(ConcurrentGroupLocation location) const {
    return at(location.handle);
}

ConcurrentBlockGroup & TatamiStore::at(ConcurrentGroupLocation location) {
    return at(location.handle);
}

const ConcurrentBlockGroup & TatamiStore::at(size_t index) const {
    return mGroups.at(index);
}

ConcurrentBlockGroup & TatamiStore::at(size_t index) {
    return mGroups.at(index);
}

