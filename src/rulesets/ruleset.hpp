#pragma

#include "core.hpp"
#include "stores/match_store.hpp"

class Ruleset {
public:
    virtual bool addWazari(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) = 0;
    virtual bool subtractWazari(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) = 0;
    virtual bool addIppon(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) = 0;
    virtual bool subtractIppon(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) = 0;
    virtual bool addShido(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) = 0;
    virtual bool subtractShido(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) = 0;
    virtual bool pause(std::unique_ptr<MatchStore> & match, std::chrono::high_resolution_clock::time_point time, std::chrono::high_resolution_clock::duration clock) = 0;
    virtual bool resume(std::unique_ptr<MatchStore> & match, std::chrono::high_resolution_clock::time_point time, std::chrono::high_resolution_clock::duration clock) = 0;
};

