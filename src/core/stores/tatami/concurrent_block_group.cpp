#include <queue>

#include "core/draw_systems/draw_system.hpp"
#include "core/misc/merge_queue_element.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "core/stores/tatami/concurrent_block_group.hpp"
#include "core/stores/tournament_store.hpp"

ConcurrentBlockGroup::ConcurrentBlockGroup()
    : mStatus(Status::NOT_STARTED)
    , mExpectedDuration(0)
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

void ConcurrentBlockGroup::recompute(const TournamentStore &tournament) {
    mMatches.clear();
    mMatchMap.clear();
    mStartedMatches.clear();
    mFinishedMatches.clear();
    mExpectedDuration = std::chrono::seconds(0);

    // Merging algorithm: Keep fetching matches from the group with smallest progress(#(matches fetched) / #(matches total))
    std::priority_queue<MergeQueueElement> progressQueue;
    std::vector<SequentialBlockGroup::ConstMatchIterator> iterators;

    for (size_t i = 0; i < groupCount(); ++i) {
        const SequentialBlockGroup & group = at(i);
        iterators.push_back(group.matchesBegin(tournament));
        mExpectedDuration += group.getExpectedDuration();

        if (group.getMatchCount() > 0)
            progressQueue.push(MergeQueueElement(i, 0, group.getMatchCount()));
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

        const auto &category = tournament.getCategory(combinedId.first);
        const auto &match = category.getMatch(combinedId.second);

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

std::chrono::milliseconds ConcurrentBlockGroup::getExpectedDuration() const {
    return mExpectedDuration;
}

bool ConcurrentBlockGroup::containsGroup(PositionHandle handle) const {
    return mGroups.contains(handle);
}

