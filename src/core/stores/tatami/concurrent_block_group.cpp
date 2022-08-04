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

std::vector<CombinedId> mergeMatchIterators(std::vector<ConstMatchIterator> &matchIterators, std::vector<ConstMatchIterator> &matchEndIterators) {
    std::queue<size_t> mergeQueue;
    for (size_t i = 0; i != matchIterators.size(); ++i) {
        mergeQueue.push(i);
    }

    std::vector<CombinedId> matches;
    while (!mergeQueue.empty()) {
        const auto index = mergeQueue.front();
        mergeQueue.pop();

        auto &it = matchIterators[index];
        const auto end = matchEndIterators[index];

        if (it == end)
            continue;

        matches.push_back(*it);
        ++it;
        mergeQueue.push(index);
    }

    return matches;
}

void ConcurrentBlockGroup::recompute(const TournamentStore &tournament) {
    mMatches.clear();
    mMatchMap.clear();
    mStartedMatches.clear();
    mFinishedMatches.clear();
    mExpectedDuration = std::chrono::seconds(0);

    std::vector<ConstMatchIterator> matchBeginIterators;
    std::vector<ConstMatchIterator> matchEndIterators;

    for (size_t i = 0; i < groupCount(); ++i) {
        const SequentialBlockGroup & group = at(i);
        mExpectedDuration += group.getExpectedDuration();

        matchBeginIterators.push_back(group.matchesBegin(tournament));
        matchEndIterators.push_back(group.matchesEnd(tournament));
    }

    const auto matchIds = mergeMatchIterators(matchBeginIterators, matchEndIterators);
    for (const CombinedId combinedId : matchIds) {
        mMatchMap[combinedId] = mMatches.size();
        mMatches.push_back(combinedId);

        const auto &category = tournament.getCategory(combinedId.getCategoryId());
        const auto &match = category.getMatch(combinedId.getMatchId());

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

