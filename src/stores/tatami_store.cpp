#include "stores/tatami_store.hpp"
#include "stores/tournament_store.hpp"
#include "stores/category_store.hpp"

#include <queue>

SequentialBlockGroup::SequentialBlockGroup()
    : mMatchCount(0)
{}

SequentialBlockGroup::ConstMatchIterator SequentialBlockGroup::matchesBegin(const TournamentStore &tournament) const {
    return ConstMatchIterator(tournament, *this, 0, 0);

}

SequentialBlockGroup::ConstMatchIterator SequentialBlockGroup::matchesEnd(const TournamentStore &tournament) const {
    return ConstMatchIterator(tournament, *this, mBlocks.size(), 0);
}

size_t SequentialBlockGroup::getMatchCount() const {
    return mMatchCount;
}

std::pair<CategoryId, MatchType> SequentialBlockGroup::getBlock(size_t pos) const {
    return mBlocks[pos];
}

void SequentialBlockGroup::addBlock(size_t pos, CategoryId categoryId, MatchType type) {
    mBlocks.insert(mBlocks.begin() + pos, std::make_pair(categoryId, type));
}

void SequentialBlockGroup::eraseBlock(CategoryId categoryId, MatchType type) {
    auto it = std::find(mBlocks.begin(), mBlocks.end(), std::make_pair(categoryId, type));
    if (it != mBlocks.end())
        mBlocks.erase(it);
}

size_t SequentialBlockGroup::findBlock(CategoryId categoryId, MatchType type) {
    auto it = std::find(mBlocks.begin(), mBlocks.end(), std::make_pair(categoryId, type));
    return std::distance(mBlocks.begin(), it);
}

void SequentialBlockGroup::recompute(const TournamentStore &tournament) {
    mMatchCount = 0;
    for (size_t i = 0; i < blockCount(); ++i) {
        auto block = getBlock(i);
        mMatchCount += tournament.getCategory(block.first).getMatchCount(block.second);
    }
}

size_t SequentialBlockGroup::blockCount() const {
    return mBlocks.size();
}

SequentialBlockGroup::ConstMatchIterator::ConstMatchIterator(const TournamentStore &tournament, const SequentialBlockGroup &group, size_t currentBlock, size_t currentMatch)
    : mTournament(tournament)
    , mGroup(group)
    , mCurrentCategory(nullptr)
    , mCurrentBlock(currentBlock)
    , mCurrentMatch(currentMatch)
{
    loadMatch();
}

void SequentialBlockGroup::ConstMatchIterator::loadMatch() {
    while (true) {
        if (mCurrentBlock == mGroup.blockCount())
            break;

        if (mCurrentCategory == nullptr) {
            auto block = mGroup.getBlock(mCurrentBlock);
            mCurrentCategory = &mTournament.getCategory(block.first);
            mCurrentType = block.second;
        }

        if (mCurrentMatch == mCurrentCategory->getMatches().size()) {
            mCurrentCategory = nullptr;
            mCurrentMatch = 0;
            ++mCurrentBlock;
            continue;
        }

        if (mCurrentType != mCurrentCategory->getMatches()[mCurrentMatch]->getType()) {
            ++mCurrentMatch;
            continue;
        }

        break;
    }
}

SequentialBlockGroup::ConstMatchIterator & SequentialBlockGroup::ConstMatchIterator::operator++() {
    ++mCurrentMatch;
    loadMatch();

    return *this;
}

std::pair<CategoryId, MatchId> SequentialBlockGroup::ConstMatchIterator::operator*() {
    assert(mCurrentCategory != nullptr);
    auto matchId = mCurrentCategory->getMatches()[mCurrentMatch]->getId();
    return std::make_pair(mCurrentCategory->getId(), matchId);
}

bool SequentialBlockGroup::ConstMatchIterator::operator!=(const SequentialBlockGroup::ConstMatchIterator &other) const {
    return !(*this == other);
}

bool SequentialBlockGroup::ConstMatchIterator::operator==(const SequentialBlockGroup::ConstMatchIterator &other) const {
    return mCurrentBlock == other.mCurrentBlock && mCurrentMatch == other.mCurrentMatch;
}

ConcurrentBlockGroup::ConcurrentBlockGroup() {
}

const ConcurrentBlockGroup::MatchList & ConcurrentBlockGroup::getMatches() const {
    return mMatches;
}


void ConcurrentBlockGroup::eraseGroup(PositionHandle handle) {
    mGroups.erase(handle);
}

PositionHandle ConcurrentBlockGroup::addGroup(TournamentStore & tournament, size_t index) {
    PositionHandle handle;
    handle.id = tournament.generateNextPositionId(mGroups);
    handle.index = index;
    return handle;
}

PositionHandle ConcurrentBlockGroup::getHandle(size_t index) const {
    assert(index < groupCount());
    return mGroups.getHandle(index);
}

size_t ConcurrentBlockGroup::groupCount() const {
    return mGroups.size();
}

SequentialBlockGroup & ConcurrentBlockGroup::getGroup(PositionHandle handle) {
    return mGroups.get(handle);
}

struct QueueElement { // Using custom class for queue entries to avoid floating point division potentially being inconsistent
    QueueElement(size_t index, size_t matchCount, size_t totalMatchCount)
        : index(index)
        , matchCount(matchCount)
        , totalMatchCount(totalMatchCount)
    {}

    bool operator<(const QueueElement &other) const {
        // First order by fraction (desc), then total match count(asc) then index(asc)

        // mMatchCount/mTotalMatchCount > other.mMatchCount/other.mTotalMatchCount
        if (matchCount * other.totalMatchCount != other.matchCount * totalMatchCount)
            return (matchCount * other.totalMatchCount) > (other.matchCount * totalMatchCount);
        if (totalMatchCount < other.totalMatchCount)
            return totalMatchCount < other.totalMatchCount;
        return index < other.index;
    }

    size_t index;
    size_t matchCount;
    size_t totalMatchCount;
};

void ConcurrentBlockGroup::recompute(const TournamentStore &tournament) {
    mMatches.clear();
    mMatchMap.clear();

    // Merging algorithm: Keep fetching matches from the group with smallest progress(#(matches fetched) / #(matches total))
    std::priority_queue<QueueElement> progressQueue;
    std::vector<SequentialBlockGroup::ConstMatchIterator> iterators;

    for (size_t i = 0; i < groupCount(); ++i) {
        auto handle = getHandle(i);
        const SequentialBlockGroup & group = getGroup(handle);
        iterators.push_back(group.matchesBegin(tournament));
        if (group.getMatchCount() == 0) continue;
        progressQueue.push(QueueElement(i, 0, group.getMatchCount()));
    }

    while (!progressQueue.empty()) {
        auto element = progressQueue.top();
        progressQueue.pop();

        auto & iterator = iterators[element.index];
        auto matchId = *iterator;
        mMatchMap[matchId] = mMatches.size();
        mMatches.push_back(matchId);

        ++(element.matchCount);
        if (element.matchCount == element.totalMatchCount) continue;
        progressQueue.push(element);
    }
}

void TatamiStore::eraseGroup(PositionHandle handle) {
    mGroups.erase(handle);
}

PositionHandle TatamiStore::addGroup(TournamentStore & tournament, size_t index) {
    PositionHandle handle;
    handle.id = tournament.generateNextPositionId(mGroups);
    handle.index = index;
    return handle;
}

PositionHandle TatamiStore::getHandle(size_t index) const {
    return mGroups.getHandle(index);
}

size_t TatamiStore::groupCount() const {
    return mGroups.size();
}

ConcurrentBlockGroup & TatamiStore::getGroup(PositionHandle handle) {
    return mGroups.get(handle);
}

void TatamiList::recomputeBlock(const TournamentStore &tournament, TatamiLocation location) {
    TatamiStore & tatami = mTatamis[location.tatamiIndex];
    ConcurrentBlockGroup & concurrentGroup = tatami.getGroup(location.concurrentGroup);
    SequentialBlockGroup & sequentialGroup = concurrentGroup.getGroup(location.sequentialGroup);

    sequentialGroup.recompute(tournament);
    concurrentGroup.recompute(tournament);
}

size_t TatamiList::getSeqIndex(TatamiLocation location, CategoryId categoryId, MatchType type) {
    return mTatamis[location.tatamiIndex].getGroup(location.concurrentGroup).getGroup(location.sequentialGroup).findBlock(categoryId, type);
}

bool TatamiList::containsTatami(TatamiLocation location) {
    return location.tatamiIndex < mTatamis.size();
}

void TatamiList::moveBlock(const TournamentStore &tournament, CategoryId categoryId, MatchType type, std::optional<TatamiLocation> from, std::optional<TatamiLocation> to, size_t seqIndex) {
    TatamiStore *fromTatami, *toTatami;
    ConcurrentBlockGroup *fromConcurrentGroup, *toConcurrentGroup;
    SequentialBlockGroup *fromSequentialGroup, *toSequentialGroup;

    if (from) {
        fromTatami = &mTatamis[from->tatamiIndex];
        fromConcurrentGroup = &fromTatami->getGroup(from->concurrentGroup);
        fromSequentialGroup = &fromConcurrentGroup->getGroup(from->sequentialGroup);
        fromSequentialGroup->eraseBlock(categoryId, type);

    }

    if (to) {
        toTatami = &mTatamis[to->tatamiIndex];
        toConcurrentGroup = &toTatami->getGroup(to->concurrentGroup);
        toSequentialGroup = &toConcurrentGroup->getGroup(to->sequentialGroup);
        toSequentialGroup->addBlock(std::min(seqIndex, toSequentialGroup->blockCount()), categoryId, type);
    }

    // Cleanup and recomputation
    if (from) {
        if (fromSequentialGroup->blockCount() == 0)
            fromConcurrentGroup->eraseGroup(from->sequentialGroup);
        else
            fromSequentialGroup->recompute(tournament);

        if (fromConcurrentGroup->groupCount() == 0)
            fromTatami->eraseGroup(from->concurrentGroup);
        else
            fromConcurrentGroup->recompute(tournament);
    }

    // recompute to location (avoiding recomputing twice if within same group)
    if (from && to && from->tatamiIndex == to->tatamiIndex && from->concurrentGroup.id == to->concurrentGroup.id) {
        if (to->sequentialGroup.id != from->sequentialGroup.id)
            toConcurrentGroup->recompute(tournament);
    }
    else if (to && !from) {
        toSequentialGroup->recompute(tournament);
        toConcurrentGroup->recompute(tournament);
    }
}

void TatamiList::pushTatami() {
    mTatamis.emplace_back();
}

std::vector<std::tuple<CategoryId, MatchType, TatamiLocation>> TatamiList::popTatami() {
    mTatamis.pop_back();
    return {};
}

void TatamiList::recoverTatami(const std::vector<std::tuple<CategoryId, MatchType, TatamiLocation>> &contents) {
    mTatamis.emplace_back();
}

size_t TatamiList::tatamiCount() const {
    return mTatamis.size();
}

