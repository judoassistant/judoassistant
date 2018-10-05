#pragma once

#include <cereal/types/polymorphic.hpp>

#include "rulesets/ruleset.hpp"
#include "serialize.hpp"

// The IJF senior ruleset from January 2018
class TwentyEighteenRuleset : public Ruleset {
private:
    const std::chrono::high_resolution_clock::duration NORMAL_TIME = std::chrono::minutes(4);
public:
    std::string getName() const override;
    static std::string getStaticName();

    bool addWazari(MatchStore & match, MatchStore::PlayerIndex playerIndex) const override;
    bool subtractWazari(MatchStore & match, MatchStore::PlayerIndex playerIndex) const override;
    bool subtractShido(MatchStore & match, MatchStore::PlayerIndex playerIndex) const override;
    bool addShido(MatchStore & match, MatchStore::PlayerIndex playerIndex) const override;
    bool isFinished(MatchStore & match) const override;
    bool shouldStop(MatchStore & match) const override;
    bool shouldEnterGoldenScore(MatchStore & match) const override;
    std::optional<MatchStore::PlayerIndex> getWinner(MatchStore & match) const override;
    bool stop(MatchStore & match, std::chrono::high_resolution_clock::time_point time, std::chrono::high_resolution_clock::duration clock) const override;
    bool resume(MatchStore & match, std::chrono::high_resolution_clock::time_point time, std::chrono::high_resolution_clock::duration clock) const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {}

    std::unique_ptr<Ruleset> clone() const override;
};

CEREAL_REGISTER_TYPE(TwentyEighteenRuleset)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Ruleset, TwentyEighteenRuleset)
