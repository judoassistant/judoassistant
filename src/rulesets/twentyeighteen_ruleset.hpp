#pragma once

#include <cereal/types/polymorphic.hpp>

#include "rulesets/ruleset.hpp"
#include "serialize.hpp"

// The IJF senior ruleset from January 2018
class TwentyEighteenRuleset : public Ruleset {
private:
    const std::chrono::high_resolution_clock::duration NORMAL_TIME = std::chrono::minutes(4);
public:
    bool addWazari(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) const override;
    bool subtractWazari(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) const override;
    bool subtractShido(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) const override;
    bool addShido(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) const override;
    bool isFinished(std::unique_ptr<MatchStore> & match) const override;
    bool shouldStop(std::unique_ptr<MatchStore> & match) const override;
    bool shouldEnterGoldenScore(std::unique_ptr<MatchStore> & match) const override;
    std::optional<MatchStore::PlayerIndex> getWinner(std::unique_ptr<MatchStore> & match) const override;
    bool stop(std::unique_ptr<MatchStore> & match, std::chrono::high_resolution_clock::time_point time, std::chrono::high_resolution_clock::duration clock) const override;
    bool resume(std::unique_ptr<MatchStore> & match, std::chrono::high_resolution_clock::time_point time, std::chrono::high_resolution_clock::duration clock) const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {}
};

CEREAL_REGISTER_TYPE(TwentyEighteenRuleset)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Ruleset, TwentyEighteenRuleset)
