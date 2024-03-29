#pragma once

#include "core/core.hpp"
#include "core/id.hpp"
#include "core/position_manager.hpp"
#include "core/serialize.hpp"
#include "core/stores/tatami/sequential_block_group.hpp"

enum class MatchType;
class TournamentStore;
struct ConcurrentGroupLocation;
struct SequentialGroupLocation;
class MatchStore;

class ConcurrentBlockGroup {
public:
    static constexpr int MAX_GROUP_COUNT = 3;

    enum class Status {
        NOT_STARTED, STARTED, FINISHED
    };

    typedef std::vector<CombinedId> MatchList; // Iteration and swapping of two elements given indexes must be supported

    ConcurrentBlockGroup();

    const MatchList & getMatches() const;

    void eraseGroup(PositionHandle handle);
    bool containsGroup(PositionHandle handle) const;
    PositionHandle getHandle(size_t index) const;
    size_t getIndex(PositionHandle handle) const;
    size_t groupCount() const;

    void setStatus(Status status);
    Status getStatus() const;

    SequentialBlockGroup & operator[](PositionHandle handle);

    const SequentialBlockGroup & at(PositionHandle handle) const;
    SequentialBlockGroup & at(PositionHandle handle);

    const SequentialBlockGroup & at(size_t index) const;
    SequentialBlockGroup & at(size_t index);

    const SequentialBlockGroup & at(SequentialGroupLocation location) const;
    SequentialBlockGroup & at(SequentialGroupLocation location);

    void recompute(const TournamentStore &tournament);
    void updateStatus(const MatchStore &match);

    SequentialGroupLocation generateLocation(TournamentStore & tournament, ConcurrentGroupLocation location, size_t index);
    SequentialGroupLocation generateLocation(ConcurrentGroupLocation location, size_t index);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mGroups, mMatches, mStatus, mMatchMap, mStartedMatches, mFinishedMatches, mExpectedDuration);
    }

    std::chrono::milliseconds getExpectedDuration() const;
private:
    void recomputeStatus();

    PositionManager<SequentialBlockGroup> mGroups;
    MatchList mMatches;
    Status mStatus;
    std::unordered_map<CombinedId,size_t> mMatchMap;
    std::chrono::milliseconds mExpectedDuration;

    std::unordered_set<CombinedId> mStartedMatches;
    std::unordered_set<CombinedId> mFinishedMatches;
};

