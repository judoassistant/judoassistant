#pragma once

#include "core.hpp"
#include "id.hpp"
#include "position_manager.hpp"
#include "serialize.hpp"
#include "stores/tatami/sequential_block_group.hpp"

enum class MatchType;
class TournamentStore;
class ConcurrentGroupLocation;
class SequentialGroupLocation;
class MatchStore;

// TODO: Add interface to delay matches

class ConcurrentBlockGroup {
public:
    enum class Status {
        NOT_STARTED, STARTED, FINISHED
    };

    typedef std::vector<std::pair<CategoryId, MatchId>> MatchList; // Iteration and swapping of two elements given indexes must be supported

    ConcurrentBlockGroup();

    const MatchList & getMatches() const;

    void eraseGroup(PositionHandle handle);
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
        ar(mGroups, mMatches, mStatus, mMatchMap);
    }
private:
    void recomputeStatus();

    PositionManager<SequentialBlockGroup> mGroups;
    MatchList mMatches;
    Status mStatus;
    std::unordered_map<std::pair<CategoryId, MatchId>,size_t> mMatchMap;

    std::unordered_set<std::pair<CategoryId, MatchId>> mStartedMatches;
    std::unordered_set<std::pair<CategoryId, MatchId>> mFinishedMatches;
};

