#pragma

#include "rulesets/ruleset.hpp"

// The IJF senior ruleset from January 2018
class TwentyEighteenRuleset : public Ruleset {
private:
    const std::chrono::high_resolution_clock::duration NORMAL_TIME = std::chrono::minutes(4);
public:
    virtual bool addWazari(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual bool subtractWazari(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual bool subtractShido(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual bool addShido(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) const = 0;
    virtual bool isFinished(std::unique_ptr<MatchStore> & match) const = 0;
    virtual bool shouldStop(std::unique_ptr<MatchStore> & match) const = 0;
    virtual bool shouldEnterGoldenScore(std::unique_ptr<MatchStore> & match) const = 0;
    virtual std::optional<MatchStore::PlayerIndex> getWinner(std::unique_ptr<MatchStore> & match) const = 0;
    virtual bool stop(std::unique_ptr<MatchStore> & match, std::chrono::high_resolution_clock::time_point time, std::chrono::high_resolution_clock::duration clock) const;
    virtual bool resume(std::unique_ptr<MatchStore> & match, std::chrono::high_resolution_clock::time_point time, std::chrono::high_resolution_clock::duration clock) const;
};

