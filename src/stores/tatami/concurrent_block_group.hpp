#pragma once

#include "core.hpp"
#include "id.hpp"
#include "position_manager.hpp"
#include "serialize.hpp"
#include "stores/tatami/sequential_block_group.hpp"

enum class MatchType;
class TournamentStore;
class SequentialGroupLocation;

class ConcurrentBlockGroup {
public:
    typedef std::vector<std::pair<CategoryId, MatchId>> MatchList; // Iteration and swapping of two elements given indexes must be supported

    enum class Status {
        NOT_STARTED, STARTED, FINISHED
    };

    ConcurrentBlockGroup();

    // TODO: Add interface to delay matches
    // TODO: Update status in action when neccesary
    const MatchList & getMatches() const;

    void eraseGroup(PositionHandle handle);
    PositionHandle getHandle(size_t index) const;
    size_t groupCount() const;

    void setStatus(Status status);
    Status getStatus() const;

    SequentialGroupLocation generateLocation(TournamentStore & tournament, size_t index);

    SequentialBlockGroup & getGroup(PositionHandle handle);
    SequentialBlockGroup & getGroup(PositionHandle handle) const;
    SequentialBlockGroup & getGroup(size_t index) const;

    void recompute(const TournamentStore &tournament);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mGroups, mMatches, mStatus, mMatchMap);
    }
private:
    PositionManager<SequentialBlockGroup> mGroups;
    MatchList mMatches;
    Status mStatus;
    std::unordered_map<std::pair<CategoryId, MatchId>,size_t> mMatchMap;
};

