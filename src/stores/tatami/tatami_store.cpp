// #include "draw_systems/draw_system.hpp"
// #include "rulesets/ruleset.hpp"
// #include "stores/category_store.hpp"
// #include "stores/match_store.hpp"
// #include "stores/tatami_store.hpp"
// #include "stores/tournament_store.hpp"


// bool TatamiStore::containsGroup(PositionHandle handle) const {
//     return mGroups.containsId(handle.id);
// }

// void TatamiStore::eraseGroup(PositionHandle handle) {
//     mGroups.erase(handle);
// }

// SequentialGroupLocation TatamiStore::generateLocation(TournamentStore & tournament, size_t index) {
//     PositionHandle conHandle;
//     conHandle.id = PositionId::generate(mGroups);
//     conHandle.index = index;

//     PositionHandle seqHandle;
//     // the new group is empty. Just generate id based on this group
//     seqHandle.id = PositionId::generate(mGroups);
//     seqHandle.index = index;

//     return {conHandle, seqHandle};
// }

// PositionHandle TatamiStore::getHandle(size_t index) const {
//     return mGroups.getHandle(index);
// }

// size_t TatamiStore::getIndex(PositionHandle handle) const {
//     return mGroups.getIndex(handle);
// }

// size_t TatamiStore::groupCount() const {
//     return mGroups.size();
// }

// ConcurrentBlockGroup & TatamiStore::getGroup(PositionHandle handle) {
//     return mGroups.get(handle);
// }

// ConcurrentBlockGroup & TatamiStore::getGroup(size_t index) {
//     assert(groupCount() > index);
//     return getGroup(getHandle(index));
// }

// void TatamiList::moveBlock(const TournamentStore &tournament, std::pair<CategoryId, MatchType> block, std::optional<BlockLocation> from, std::optional<BlockLocation> to) {
//     TatamiStore *fromTatami, *toTatami;
//     ConcurrentBlockGroup *fromConcurrentGroup, *toConcurrentGroup;
//     SequentialBlockGroup *fromSequentialGroup, *toSequentialGroup;

//     if (from) {
//         assert(mTatamis.containsTatami(from->getTatamiHandle()));
//         fromTatami = &mTatamis.getTatami(from->getTatamiHandle());

//         assert(fromTatami->containsGroup(from->getConcurrentGroupHandle()));
//         fromConcurrentGroup = &fromTatami->getGroup(from->getConcurrentGroupHandle());

//         assert(fromConcurrentGroup.containsGroup(from->getSequentialGroupHandle()));
//         fromSequentialGroup = &fromConcurrentGroup->getGroup(from->getSequentialGroupHandle());

//         assert(fromSequentialGroup->blockCount() > from->getBlockPos());
//         fromSequentialGroup->eraseBlock(block);
//     }

//     if (to) {
//         toTatami = &mTatamis.getTatami(to->getTatamiHandle());
//         toConcurrentGroup = &toTatami->getGroup(to->getConcurrentGroupHandle());
//         toSequentialGroup = &toConcurrentGroup->getGroup(to->getSequentialGroupHandle());
//         toSequentialGroup->addBlock(std::min(seqIndex, toSequentialGroup->blockCount()), categoryId, type);
//     }

//     // Handle cleanup and recomputation of sequential groups
//     if (from) {
//         if (fromSequentialGroup->blockCount() == 0)
//             fromConcurrentGroup->eraseGroup(from->getSequentialGroupHandle());
//         else
//             fromSequentialGroup->recompute(tournament);
//     }

//     if (to) {
//         // Recompute if `from` sequentialGroup is not equiv to `to` sequentialGroup
//         if (!(from && from->sequentialGroup.equiv(to->sequentialGroup)))
//             toSequentialGroup->recompute(tournament);
//     }

//     // Handle cleanup and recomputation of concurrent groups
//     if (from) {
//         if (fromConcurrentGroup->groupCount() == 0)
//             fromTatami->eraseGroup(from->concurrentGroup);
//         else
//             fromConcurrentGroup->recompute(tournament);
//     }

//     if (to) {
//         // Recompute if `from` concurrentGroup is not equiv to `to` concurrentGroup
//         if (!(from && from->sequentialGroup.concurrentGroup.equiv(to->sequentialGroup.concurrentGroup)))
//             toConcurrentGroup->recompute(tournament);
//     }
// }

// void TatamiList::pushTatami() {
//     mTatamis.emplace_back();
// }

// size_t TatamiList::tatamiCount() const {
//     return mTatamis.size();
// }

// TatamiStore & TatamiList::operator[](size_t index) {
//     assert(index < tatamiCount());
//     return mTatamis[index];
// }

// const TatamiStore & TatamiList::operator[](size_t index) const {
//     assert(index < tatamiCount());
//     return mTatamis[index];
// }

