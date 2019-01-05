#include "stores/tatami/tatami_list.hpp"
#include "stores/tatami/tatami_location.hpp"

void TatamiList::eraseTatami(PositionHandle handle) {
    assert(mTatamis.contains(handle));
    mTatamis.erase(handle);
}

bool TatamiList::containsTatami(PositionHandle handle) const {
    return mTatamis.contains(handle);
}

bool TatamiList::containsTatami(TatamiLocation location) const {
    return containsTatami(location.handle);
}

PositionHandle TatamiList::getHandle(size_t index) const {
    return mTatamis.getHandle(index);
}

size_t TatamiList::getIndex(PositionHandle handle) const {
    return mTatamis.getIndex(handle);
}

size_t TatamiList::getIndex(TatamiLocation location) const {
    return mTatamis.getIndex(location.handle);
}

size_t TatamiList::tatamiCount() const {
    return mTatamis.size();
}

TatamiLocation TatamiList::generateLocation(size_t index) {
    PositionHandle handle;
    handle.id = PositionId::generate(mTatamis);
    handle.index = index;
    return {std::move(handle)};
}

BlockLocation TatamiList::refreshLocation(BlockLocation location, std::pair<CategoryId, MatchType> block) const {
    PositionHandle &tatamiHandle = location.sequentialGroup.concurrentGroup.tatami.handle;
    tatamiHandle.index = getIndex(tatamiHandle);

    const TatamiStore &tatami = at(tatamiHandle);

    PositionHandle &concurrentHandle = location.sequentialGroup.concurrentGroup.handle;
    concurrentHandle.index = tatami.getIndex(concurrentHandle);

    const ConcurrentBlockGroup &concurrentGroup = tatami.at(concurrentHandle);

    PositionHandle &sequentialHandle = location.sequentialGroup.handle;
    sequentialHandle.index = concurrentGroup.getIndex(sequentialHandle);

    const SequentialBlockGroup &sequentialGroup = concurrentGroup.at(sequentialHandle);

    location.pos = sequentialGroup.getIndex(block);
    return location;
}

TatamiStore & TatamiList::operator[](PositionHandle handle) {
    return mTatamis[handle];
}

void TatamiList::insert(PositionHandle handle) {
    mTatamis.insert(handle);
}

const TatamiStore & TatamiList::at(PositionHandle handle) const {
    return mTatamis.at(handle);
}

TatamiStore & TatamiList::at(PositionHandle handle) {
    return mTatamis.at(handle);
}

const TatamiStore & TatamiList::at(size_t index) const {
    return mTatamis.at(index);
}

TatamiStore & TatamiList::at(size_t index) {
    return mTatamis.at(index);
}

TatamiStore & TatamiList::at(TatamiLocation location) {
    return mTatamis.at(location.handle);
}

const TatamiStore & TatamiList::at(TatamiLocation location) const {
    return mTatamis.at(location.handle);
}

ConcurrentBlockGroup & TatamiList::at(ConcurrentGroupLocation location) {
    return at(location.tatami).at(location);
}

const ConcurrentBlockGroup & TatamiList::at(ConcurrentGroupLocation location) const {
    return at(location.tatami).at(location);
}

SequentialBlockGroup & TatamiList::at(SequentialGroupLocation location) {
    return at(location.concurrentGroup).at(location);
}

const SequentialBlockGroup & TatamiList::at(SequentialGroupLocation location) const {
    return at(location.concurrentGroup).at(location);
}

void TatamiList::moveBlock(const TournamentStore &tournament, std::pair<CategoryId, MatchType> block, std::optional<BlockLocation> from, std::optional<BlockLocation> to) {
    TatamiStore *fromTatami, *toTatami;
    ConcurrentBlockGroup *fromConcurrentGroup, *toConcurrentGroup;
    SequentialBlockGroup *fromSequentialGroup, *toSequentialGroup;

    if (from) {
        fromTatami = &at(from->getTatamiHandle());
        fromConcurrentGroup = &fromTatami->at(from->getConcurrentGroupHandle());
        fromSequentialGroup = &fromConcurrentGroup->at(from->getSequentialGroupHandle());
        fromSequentialGroup->eraseBlock(block);
    }

    if (to) {
        toTatami = &(*this)[to->getTatamiHandle()];
        toConcurrentGroup = &(*toTatami)[to->getConcurrentGroupHandle()];
        toSequentialGroup = &(*toConcurrentGroup)[to->getSequentialGroupHandle()];
        toSequentialGroup->addBlock(std::min(to->pos, toSequentialGroup->blockCount()), block);
    }

    // Handle cleanup and recomputation of sequential groups
    if (from) {
        if (fromSequentialGroup->blockCount() == 0)
            fromConcurrentGroup->eraseGroup(from->getSequentialGroupHandle());
        else
            fromSequentialGroup->recompute(tournament);
    }

    if (to) {
        // Recompute if `from` sequentialGroup is not equiv to `to` sequentialGroup
        if (!(from && from->sequentialGroup.equiv(to->sequentialGroup)))
            toSequentialGroup->recompute(tournament);
    }

    // Handle cleanup and recomputation of concurrent groups
    if (from) {
        if (fromConcurrentGroup->groupCount() == 0)
            fromTatami->eraseGroup(from->getConcurrentGroupHandle());
        else
            fromConcurrentGroup->recompute(tournament);
    }

    if (to) {
        // Recompute if `from` concurrentGroup is not equiv to `to` concurrentGroup
        if (!(from && from->sequentialGroup.concurrentGroup.equiv(to->sequentialGroup.concurrentGroup)))
            toConcurrentGroup->recompute(tournament);
    }
}

void TatamiList::recomputeBlock(const TournamentStore &tournament, BlockLocation location) {
    TatamiStore &tatami = at(location.sequentialGroup.concurrentGroup.tatami.handle);
    ConcurrentBlockGroup &concurrentGroup = tatami.at(location.sequentialGroup.concurrentGroup.handle);
    SequentialBlockGroup &sequentialGroup = concurrentGroup.at(location.sequentialGroup.handle);

    sequentialGroup.recompute(tournament);
    concurrentGroup.recompute(tournament);
}
