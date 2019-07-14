#pragma once

#include "core/core.hpp"
#include "core/rulesets/ruleset_identifier.hpp"
#include "core/serialize.hpp"
#include "core/stores/match_store.hpp"

class Ruleset {
public:
    virtual ~Ruleset() {}

    virtual std::string getName() const = 0;
    virtual RulesetIdentifier getIdentifier() const = 0;

    virtual std::chrono::milliseconds getNormalTime() const = 0;
    virtual std::chrono::milliseconds getExpectedTime() const = 0;

    virtual std::chrono::milliseconds getOsaekomiIpponTime() const = 0;
    virtual std::chrono::milliseconds getOsaekomiWazariTime() const = 0;

    // Pausing
    virtual bool shouldPause(const MatchStore &match, std::chrono::milliseconds masterTime) const;
    virtual bool canPause(const MatchStore &match, std::chrono::milliseconds masterTime) const;
    virtual void pause(MatchStore &match, std::chrono::milliseconds masterTime) const;

    // Resuming
    virtual bool canResume(const MatchStore &match, std::chrono::milliseconds masterTime) const;
    virtual void resume(MatchStore &match, std::chrono::milliseconds masterTime) const;

    // Ippon
    virtual bool canAwardIppon(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual void awardIppon(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const = 0;

    virtual bool canCancelIppon(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual void cancelIppon(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const = 0;

    // Wazari
    virtual bool canAwardWazari(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual void awardWazari(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const = 0;

    virtual bool canCancelWazari(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual void cancelWazari(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const = 0;

    // Shido
    virtual bool canAwardShido(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual void awardShido(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const = 0;

    virtual bool canCancelShido(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual void cancelShido(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const = 0;

    // Hansoku-Make
    virtual bool canAwardHansokuMake(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual void awardHansokuMake(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const = 0;

    virtual bool canCancelHansokuMake(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual void cancelHansokuMake(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const = 0;

    // Osaekomi
    virtual bool canStartOsaekomi(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const;
    virtual void startOsaekomi(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const;

    virtual bool canStopOsaekomi(const MatchStore &match, std::chrono::milliseconds masterTime) const;
    virtual void stopOsaekomi(MatchStore &match, std::chrono::milliseconds masterTime) const;
    virtual bool shouldStopOsaekomi(const MatchStore &match, std::chrono::milliseconds masterTime) const;

    virtual bool shouldAwardOsaekomiWazari(const MatchStore &match, std::chrono::milliseconds masterTime) const;
    virtual bool shouldAwardOsaekomiIppon(const MatchStore &match, std::chrono::milliseconds masterTime) const;

    // Getting status / winner
    virtual bool isFinished(const MatchStore &match, std::chrono::milliseconds masterTime) const = 0;

    virtual std::optional<MatchStore::PlayerIndex> getWinner(const MatchStore &match, std::chrono::milliseconds masterTime) const = 0;
    virtual std::optional<MatchStore::PlayerIndex> getWinner(const MatchStore &match) const = 0; // Get winner when match is finished

    // Misc
    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {}

    virtual std::unique_ptr<Ruleset> clone() const = 0;

    static const std::vector<std::unique_ptr<const Ruleset>> & getRulesets();
    static std::unique_ptr<Ruleset> getRuleset(RulesetIdentifier identifier);
    static std::unique_ptr<Ruleset> getDefaultRuleset();

protected:
    void updateStatus(MatchStore &match, std::chrono::milliseconds masterTime) const;
};

