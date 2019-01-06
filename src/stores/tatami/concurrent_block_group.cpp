#include <queue>

#include "stores/tatami/concurrent_block_group.hpp"
#include "stores/category_store.hpp"
#include "draw_systems/draw_system.hpp"
#include "rulesets/ruleset.hpp"
#include "stores/match_store.hpp"
#include "stores/tournament_store.hpp"

ConcurrentBlockGroup::ConcurrentBlockGroup()
    : mStatus(Status::NOT_STARTED)
{}

const ConcurrentBlockGroup::MatchList & ConcurrentBlockGroup::getMatches() const {
    return mMatches;
}

void ConcurrentBlockGroup::eraseGroup(PositionHandle handle) {
    mGroups.erase(handle);
}

PositionHandle ConcurrentBlockGroup::getHandle(size_t index) const {
    assert(index < mGroups.size());
    return mGroups.getHandle(index);
}

size_t ConcurrentBlockGroup::getIndex(PositionHandle handle) const {
    return mGroups.getIndex(handle);
}

size_t ConcurrentBlockGroup::groupCount() const {
    return mGroups.size();
}

void ConcurrentBlockGroup::setStatus(ConcurrentBlockGroup::Status status) {
    mStatus = status;
}

ConcurrentBlockGroup::Status ConcurrentBlockGroup::getStatus() const {
    return mStatus;
}

SequentialGroupLocation  ConcurrentBlockGroup::generateLocation(ConcurrentGroupLocation location, size_t index) {
    PositionHandle handle;
    handle.id = PositionId::generate(mGroups);
    handle.index = index;

    return {location, handle};
}

SequentialBlockGroup & ConcurrentBlockGroup::operator[](PositionHandle handle) {
    return mGroups[handle];
}

SequentialBlockGroup & ConcurrentBlockGroup::at(PositionHandle handle) {
    return mGroups.at(handle);
}

const SequentialBlockGroup & ConcurrentBlockGroup::at(PositionHandle handle) const {
    return mGroups.at(handle);
}

const SequentialBlockGroup & ConcurrentBlockGroup::at(SequentialGroupLocation location) const {
    return at(location.handle);
}

SequentialBlockGroup & ConcurrentBlockGroup::at(SequentialGroupLocation location) {
    return at(location.handle);
}

const SequentialBlockGroup & ConcurrentBlockGroup::at(size_t index) const {
    return mGroups.at(index);
}

SequentialBlockGroup & ConcurrentBlockGroup::at(size_t index) {
    return mGroups.at(index);
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
    mStartedMatches.clear();
    mFinishedMatches.clear();

    // Merging algorithm: Keep fetching matches from the group with smallest progress(#(matches fetched) / #(matches total))
    std::priority_queue<QueueElement> progressQueue;
    std::vector<SequentialBlockGroup::ConstMatchIterator> iterators;

    for (size_t i = 0; i < groupCount(); ++i) {
        const SequentialBlockGroup & group = at(i);
        iterators.push_back(group.matchesBegin(tournament));
        if (group.getMatchCount() == 0) continue;
        progressQueue.push(QueueElement(i, 0, group.getMatchCount()));
    }

    while (!progressQueue.empty()) {
        auto element = progressQueue.top();
        progressQueue.pop();

        auto & iterator = iterators[element.index];
        auto combinedId = *iterator;
        ++iterator;
        mMatchMap[combinedId] = mMatches.size();
        mMatches.push_back(combinedId);

        ++(element.matchCount);
        if (element.matchCount == element.totalMatchCount) continue;
        progressQueue.push(element);

        const auto &match = tournament.getCategory(combinedId.first).getMatch(combinedId.second);

        if (match.getStatus() == MatchStatus::FINISHED)
            mFinishedMatches.insert(combinedId);
        else if (match.getStatus() != MatchStatus::NOT_STARTED)
            mStartedMatches.insert(combinedId);
    }

    recomputeStatus();
}

void ConcurrentBlockGroup::recomputeStatus() {
    if (mMatches.size() == mFinishedMatches.size())
        mStatus = Status::FINISHED;
    else if (!mStartedMatches.empty() || !mFinishedMatches.empty())
        mStatus = Status::STARTED;
    else
        mStatus = Status::NOT_STARTED;
}

void ConcurrentBlockGroup::updateStatus(const MatchStore &match) {
    auto combinedId = match.getCombinedId();
    if (match.getStatus() == MatchStatus::FINISHED) {
        // insert into finished matches and delete from other maps if the match
        // was not already finished.
        if (mFinishedMatches.insert(combinedId).second)
            mStartedMatches.erase(combinedId);
    }
    else if (match.getStatus() != MatchStatus::NOT_STARTED) {
        if (mStartedMatches.insert(combinedId).second)
            mFinishedMatches.erase(combinedId);
    }
    else {
        mStartedMatches.erase(combinedId);
        mFinishedMatches.erase(combinedId);
    }

    recomputeStatus();
}

