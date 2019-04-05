#pragma once

#include <chrono>
#include <map>
#include <string>
#include <unordered_set>

#include "core/core.hpp"
#include "core/id.hpp"
#include "core/serialize.hpp"
#include "core/stores/tatami/location.hpp"

class DrawSystem;
class MatchStore;
class Ruleset;

enum class MatchType;

// TODO: Make 'magic' categories that update automatically

struct CategoryStatus {
    CategoryStatus()
        : notStartedMatches(0)
        , startedMatches(0)
        , finishedMatches(0)
    {}

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(notStartedMatches, startedMatches, finishedMatches);
    }

    unsigned int notStartedMatches;
    unsigned int startedMatches;
    unsigned int finishedMatches;
};


class CategoryStore {
public:
    typedef std::vector<std::unique_ptr<MatchStore>> MatchList;
    static constexpr std::chrono::milliseconds MIN_EXPECTED_DURATION = std::chrono::minutes(20); // TODO: Have a more robust way drawing very short categories

    CategoryStore() {}
    CategoryStore(CategoryId id, const std::string &name, std::unique_ptr<Ruleset> ruleset, std::unique_ptr<DrawSystem> drawSystem);
    CategoryStore(const CategoryStore &other);

    const std::string & getName() const;
    std::string getName(MatchType type) const;
    void setName(const std::string &name);

    const CategoryId & getId() const;

    const MatchList & getMatches() const;
    MatchList & getMatches();
    MatchStore & getMatch(MatchId id);
    const MatchStore & getMatch(MatchId id) const;
    void pushMatch(std::unique_ptr<MatchStore> match);
    std::unique_ptr<MatchStore> popMatch();
    bool containsMatch(MatchId id) const;
    MatchList clearMatches();

    // Get the number of non-bye matches of the given type
    size_t getMatchCount(MatchType type) const;

    const std::unordered_set<PlayerId> & getPlayers() const;
    void erasePlayer(PlayerId id);
    void addPlayer(PlayerId id);
    bool containsPlayer(PlayerId id) const;

    std::unique_ptr<Ruleset> setRuleset(std::unique_ptr<Ruleset> ptr);
    Ruleset & getRuleset();
    const Ruleset & getRuleset() const;

    std::unique_ptr<DrawSystem> setDrawSystem(std::unique_ptr<DrawSystem> ptr);
    DrawSystem & getDrawSystem();
    const DrawSystem & getDrawSystem() const;

    std::optional<BlockLocation> getLocation(MatchType type) const;
    void setLocation(MatchType type, std::optional<BlockLocation> location);

    const CategoryStatus& getStatus(MatchType type) const;
    CategoryStatus& getStatus(MatchType type);
    void setStatus(MatchType type, const CategoryStatus &status);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mId);
        ar(mName);
        ar(mPlayers);
        ar(mMatches);
        ar(mMatchMap);
        ar(mMatchCount);
        ar(mStatus);
        ar(mLocation);
        ar(mRuleset);
        ar(mDrawSystem);
    }

    std::chrono::milliseconds expectedDuration(MatchType type) const;

private:
    CategoryId mId;
    std::string mName;
    std::unordered_set<PlayerId> mPlayers;
    MatchList mMatches; // order matters in this case
    std::unordered_map<MatchId, size_t> mMatchMap;
    std::array<size_t, 2> mMatchCount;
    std::array<CategoryStatus, 2> mStatus;
    std::array<std::optional<BlockLocation>, 2> mLocation;
    std::unique_ptr<Ruleset> mRuleset;
    std::unique_ptr<DrawSystem> mDrawSystem;
};

