#pragma once

#include <cereal/types/polymorphic.hpp>

#include "core/rulesets/ruleset.hpp"
#include "core/serialize.hpp"

// The IJF senior ruleset from January 2018
class TwentyEighteenRuleset : public Ruleset {
private:
public:
    std::string getName() const override;
    RulesetIdentifier getIdentifier() const override;

    std::chrono::milliseconds getNormalTime() const override;
    std::chrono::milliseconds getExpectedTime() const override;

    std::chrono::milliseconds getOsaekomiIpponTime() const override;
    std::chrono::milliseconds getOsaekomiWazariTime() const override;

    // Ippon
    bool canAwardIppon(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const;
    void awardIppon(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const;

    bool canCancelIppon(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const;
    void cancelIppon(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const;

    // Wazari
    bool canAwardWazari(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const override;
    void awardWazari(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const override;

    bool canCancelWazari(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const override;
    void cancelWazari(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const override;

    // Shido
    bool canAwardShido(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const override;
    void awardShido(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const override;

    bool canCancelShido(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const override;
    void cancelShido(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const override;

    // Hansoku-Make
    bool canAwardHansokuMake(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const override;
    void awardHansokuMake(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const override;

    bool canCancelHansokuMake(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const override;
    void cancelHansokuMake(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const override;

    // Misc
    bool isFinished(const MatchStore &match, std::chrono::milliseconds masterTime) const override;

    std::optional<MatchStore::PlayerIndex> getWinner(const MatchStore &match, std::chrono::milliseconds masterTime) const override;
    std::optional<MatchStore::PlayerIndex> getWinner(const MatchStore &match) const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {}

    std::unique_ptr<Ruleset> clone() const override;
};

CEREAL_REGISTER_TYPE(TwentyEighteenRuleset)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Ruleset, TwentyEighteenRuleset)
