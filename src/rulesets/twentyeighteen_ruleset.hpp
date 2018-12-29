#pragma once

#include <cereal/types/polymorphic.hpp>

#include "rulesets/ruleset.hpp"
#include "serialize.hpp"

// The IJF senior ruleset from January 2018
class TwentyEighteenRuleset : public Ruleset {
private:
public:
    std::string getName() const override;

    std::chrono::milliseconds getNormalTime() const override;

    bool canAddWazari(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const override;
    void addWazari(MatchStore &match, MatchStore::PlayerIndex playerIndex) const override;

    bool canSubtractWazari(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const override;
    void subtractWazari(MatchStore &match, MatchStore::PlayerIndex playerIndex) const override;

    bool canAddShido(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const override;
    void addShido(MatchStore &match, MatchStore::PlayerIndex playerIndex) const override;

    bool canSubtractShido(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const override;
    void subtractShido(MatchStore &match, MatchStore::PlayerIndex playerIndex) const override;

    bool isFinished(const MatchStore &match, std::chrono::milliseconds masterTime) const override;

    std::optional<MatchStore::PlayerIndex> getWinner(const MatchStore &match, std::chrono::milliseconds masterTime) const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {}

    std::unique_ptr<Ruleset> clone() const override;
};

CEREAL_REGISTER_TYPE(TwentyEighteenRuleset)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Ruleset, TwentyEighteenRuleset)
