// #include "draw_systems/draw_system.hpp"
// #include "rulesets/ruleset.hpp"
// #include "stores/category_store.hpp"
// #include "stores/match_store.hpp"
// #include "stores/tatami_store.hpp"
// #include "stores/tournament_store.hpp"

// #include <queue>

// SequentialBlockGroup::SequentialBlockGroup()
//     : mMatchCount(0)
// {}

// SequentialBlockGroup::ConstMatchIterator SequentialBlockGroup::matchesBegin(const TournamentStore &tournament) const {
//     return ConstMatchIterator(tournament, *this, 0, 0);

// }

// SequentialBlockGroup::ConstMatchIterator SequentialBlockGroup::matchesEnd(const TournamentStore &tournament) const {
//     return ConstMatchIterator(tournament, *this, mBlocks.size(), 0);
// }

// size_t SequentialBlockGroup::getMatchCount() const {
//     return mMatchCount;
// }

// size_t SequentialBlockGroup::blockCount() const {
//     return mBlocks.size();
// }

// std::pair<CategoryId, MatchType> SequentialBlockGroup::getBlock(size_t index) {
//     assert(blockCount() > index);
//     return getBlock(getHandle(index));
// }

// std::pair<CategoryId, MatchType> & SequentialBlockGroup::getBlock(PositionHandle handle) {
//     return mBlocks.get(handle);
// }

// void SequentialBlockGroup::recompute(const TournamentStore &tournament) {
//     mMatchCount = 0;
//     for (size_t i = 0; i < blockCount(); ++i) {
//         auto block = getBlock(i);
//         mMatchCount += tournament.getCategory(block.first).getMatchCount(block.second);
//     }
// }

// SequentialBlockGroup::ConstMatchIterator::ConstMatchIterator(const TournamentStore &tournament, const SequentialBlockGroup &group, size_t currentBlock, size_t currentMatch)
//     : mTournament(tournament)
//     , mGroup(group)
//     , mCurrentCategory(nullptr)
//     , mCurrentBlock(currentBlock)
//     , mCurrentMatch(currentMatch)
// {
//     loadMatch();
// }

// void SequentialBlockGroup::ConstMatchIterator::loadMatch() {
//     while (true) {
//         if (mCurrentBlock == mGroup.blockCount())
//             break;

//         if (mCurrentCategory == nullptr) {
//             auto block = mGroup.getBlock(mCurrentBlock);
//             mCurrentCategory = &mTournament.getCategory(block.first);
//             mCurrentType = block.second;
//         }

//         if (mCurrentMatch == mCurrentCategory->getMatches().size()) {
//             mCurrentCategory = nullptr;
//             mCurrentMatch = 0;
//             ++mCurrentBlock;
//             continue;
//         }

//         const auto &match = *(mCurrentCategory->getMatches()[mCurrentMatch]);
//         if (match.isBye()) {
//             ++mCurrentMatch;
//             continue;
//         }

//         if (mCurrentType != match.getType()) {
//             ++mCurrentMatch;
//             continue;
//         }

//         break;
//     }
// }

// SequentialBlockGroup::ConstMatchIterator & SequentialBlockGroup::ConstMatchIterator::operator++() {
//     ++mCurrentMatch;
//     loadMatch();

//     return *this;
// }

// std::pair<CategoryId, MatchId> SequentialBlockGroup::ConstMatchIterator::operator*() {
//     assert(mCurrentCategory != nullptr);
//     auto matchId = mCurrentCategory->getMatches()[mCurrentMatch]->getId();
//     return std::make_pair(mCurrentCategory->getId(), matchId);
// }

// bool SequentialBlockGroup::ConstMatchIterator::operator!=(const SequentialBlockGroup::ConstMatchIterator &other) const {
//     return !(*this == other);
// }

// bool SequentialBlockGroup::ConstMatchIterator::operator==(const SequentialBlockGroup::ConstMatchIterator &other) const {
//     return mCurrentBlock == other.mCurrentBlock && mCurrentMatch == other.mCurrentMatch;
// }

// ConcurrentBlockGroup::ConcurrentBlockGroup()
//     : mStatus(Status::NOT_STARTED)
// {
// }

// const ConcurrentBlockGroup::MatchList & ConcurrentBlockGroup::getMatches() const {
//     return mMatches;
// }


// void ConcurrentBlockGroup::eraseGroup(PositionHandle handle) {
//     mGroups.erase(handle);
// }

// ConcurrentGroupLocation ConcurrentBlockGroup::generateLocation(TournamentStore & tournament, size_t index) {
//     PositionHandle handle;
//     handle.id = PositionId::generate(mGroups);
//     handle.index = index;
//     return handle;
// }

// PositionHandle ConcurrentBlockGroup::getHandle(size_t index) const {
//     assert(index < groupCount());
//     return mGroups.getHandle(index);
// }

// bool TatamiStore::containsGroup(PositionHandle handle) const {
//     return mGroups.containsId(handle.id);
// }

// size_t ConcurrentBlockGroup::groupCount() const {
//     return mGroups.size();
// }

// SequentialBlockGroup & ConcurrentBlockGroup::getGroup(PositionHandle handle) {
//     return mGroups.get(handle);
// }

// SequentialBlockGroup & ConcurrentBlockGroup::getGroup(size_t index) {
//     assert(groupCount() > index); // TODO: Make sure the code is consistent with assert and throw usage
//     return getGroup(getHandle(index));
// }

// struct QueueElement { // Using custom class for queue entries to avoid floating point division potentially being inconsistent
//     QueueElement(size_t index, size_t matchCount, size_t totalMatchCount)
//         : index(index)
//         , matchCount(matchCount)
//         , totalMatchCount(totalMatchCount)
//     {}

//     bool operator<(const QueueElement &other) const {
//         // First order by fraction (desc), then total match count(asc) then index(asc)

//         // mMatchCount/mTotalMatchCount > other.mMatchCount/other.mTotalMatchCount
//         if (matchCount * other.totalMatchCount != other.matchCount * totalMatchCount)
//             return (matchCount * other.totalMatchCount) > (other.matchCount * totalMatchCount);
//         if (totalMatchCount < other.totalMatchCount)
//             return totalMatchCount < other.totalMatchCount;
//         return index < other.index;
//     }

//     size_t index;
//     size_t matchCount;
//     size_t totalMatchCount;
// };

// void ConcurrentBlockGroup::recompute(const TournamentStore &tournament) {
//     mMatches.clear();
//     mMatchMap.clear();
//     mStatus = Status::NOT_STARTED;

//     // Merging algorithm: Keep fetching matches from the group with smallest progress(#(matches fetched) / #(matches total))
//     std::priority_queue<QueueElement> progressQueue;
//     std::vector<SequentialBlockGroup::ConstMatchIterator> iterators;

//     for (size_t i = 0; i < groupCount(); ++i) {
//         auto handle = getHandle(i);
//         const SequentialBlockGroup & group = getGroup(handle);
//         iterators.push_back(group.matchesBegin(tournament));
//         if (group.getMatchCount() == 0) continue;
//         progressQueue.push(QueueElement(i, 0, group.getMatchCount()));
//     }

//     while (!progressQueue.empty()) {
//         auto element = progressQueue.top();
//         progressQueue.pop();

//         auto & iterator = iterators[element.index];
//         auto combinedId = *iterator;
//         ++iterator;
//         mMatchMap[combinedId] = mMatches.size();
//         mMatches.push_back(combinedId);

//         ++(element.matchCount);
//         if (element.matchCount == element.totalMatchCount) continue;
//         progressQueue.push(element);

//         const auto &match = tournament.getCategory(combinedId.first).getMatch(combinedId.second);

//         if (match.getStatus() == MatchStatus::FINISHED && mStatus == Status::NOT_STARTED)
//             mStatus = Status::FINISHED;
//         else if (match.getStatus() != MatchStatus::NOT_STARTED)
//             mStatus = Status::STARTED;
//     }

//     log_debug().field("status", (int) mStatus).msg("Done recomputing");
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

// void ConcurrentBlockGroup::setStatus(ConcurrentBlockGroup::Status status) {
//     mStatus = status;
// }

// ConcurrentBlockGroup::Status ConcurrentBlockGroup::getStatus() const {
//     return mStatus;
// }
