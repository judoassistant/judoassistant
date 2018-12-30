#pragma once

#include "core.hpp"
#include "serialize.hpp"
#include "stores/match_store.hpp"

class Ruleset {
public:
    virtual ~Ruleset() {}

    virtual std::string getName() const = 0;

    virtual std::chrono::milliseconds getNormalTime() const = 0;

    virtual bool canAddIppon(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const;
    virtual void addIppon(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const;

    virtual bool canSubtractIppon(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const;
    virtual void subtractIppon(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const;

    virtual bool shouldPause(const MatchStore &match, std::chrono::milliseconds masterTime) const;
    virtual bool canPause(const MatchStore &match, std::chrono::milliseconds masterTime) const;
    virtual void pause(MatchStore &match, std::chrono::milliseconds masterTime) const;

    virtual bool canResume(const MatchStore &match, std::chrono::milliseconds masterTime) const;
    virtual void resume(MatchStore &match, std::chrono::milliseconds masterTime) const;

    virtual bool canAddWazari(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual void addWazari(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const = 0;

    virtual bool canSubtractWazari(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual void subtractWazari(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const = 0;

    virtual bool canAddShido(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual void addShido(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const = 0;

    virtual bool canSubtractShido(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual void subtractShido(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const = 0;

    virtual bool canAddHansokuMake(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual void addHansokuMake(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const = 0;

    virtual bool canSubtractHansokuMake(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual void subtractHansokuMake(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const = 0;

    virtual bool isFinished(const MatchStore &match, std::chrono::milliseconds masterTime) const = 0;

    virtual std::optional<MatchStore::PlayerIndex> getWinner(const MatchStore &match, std::chrono::milliseconds masterTime) const = 0;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {}

    virtual std::unique_ptr<Ruleset> clone() const = 0;

protected:
    void updateStatus(MatchStore &match, std::chrono::milliseconds masterTime) const;
};

