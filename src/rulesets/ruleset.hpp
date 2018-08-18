#pragma

#include "core.hpp"
#include "stores/match_store.hpp"

// TODO: Handle matches that end in a draw
class Ruleset {
public:
    virtual bool addWazari(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) = 0;
    virtual bool subtractWazari(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) = 0;
    virtual bool subtractShido(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) = 0;
    virtual bool addShido(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) = 0;
    virtual bool isFinished(std::unique_ptr<MatchStore> & match) = 0;
    virtual bool shouldStop(std::unique_ptr<MatchStore> & match) = 0;
    virtual bool shouldEnterGoldenScore(std::unique_ptr<MatchStore> & match) = 0;

    virtual bool addIppon(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex);
    virtual bool subtractIppon(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex);
    virtual bool enterGoldenScore(std::unique_ptr<MatchStore> & match);
    virtual bool exitGoldenScore(std::unique_ptr<MatchStore> & match); // used for undoing actions
    virtual bool stop(std::unique_ptr<MatchStore> & match, std::chrono::high_resolution_clock::time_point time, std::chrono::high_resolution_clock::duration clock);
    virtual bool resume(std::unique_ptr<MatchStore> & match, std::chrono::high_resolution_clock::time_point time, std::chrono::high_resolution_clock::duration clock);
};

