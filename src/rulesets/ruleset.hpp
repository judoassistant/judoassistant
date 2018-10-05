#pragma once

#include "core.hpp"
#include "serialize.hpp"

#include "stores/match_store.hpp"

// TODO: Make const where appropriate
class Ruleset {
public:
    virtual ~Ruleset() {}
    virtual bool addWazari(MatchStore & match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual bool subtractWazari(MatchStore & match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual bool subtractShido(MatchStore & match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual bool addShido(MatchStore & match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual bool isFinished(MatchStore & match) const = 0;
    virtual bool shouldStop(MatchStore & match) const = 0;
    virtual bool shouldEnterGoldenScore(MatchStore & match) const = 0;
    virtual std::optional<MatchStore::PlayerIndex> getWinner(MatchStore & match) const = 0;
    virtual bool stop(MatchStore & match, std::chrono::high_resolution_clock::time_point time, std::chrono::high_resolution_clock::duration clock) const = 0;
    virtual bool resume(MatchStore & match, std::chrono::high_resolution_clock::time_point time, std::chrono::high_resolution_clock::duration clock) const = 0;

    virtual bool addIppon(MatchStore & match, MatchStore::PlayerIndex playerIndex) const;
    virtual bool subtractIppon(MatchStore & match, MatchStore::PlayerIndex playerIndex) const;
    virtual bool enterGoldenScore(MatchStore & match) const;
    virtual bool exitGoldenScore(MatchStore & match) const; // used for undoing actions

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {}

    virtual std::unique_ptr<Ruleset> clone() const = 0;
};

