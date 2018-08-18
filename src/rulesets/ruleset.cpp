#include "rulesets/ruleset.hpp"

bool Ruleset::enterGoldenScore(std::unique_ptr<MatchStore> & match) {
    match->setGoldenScore(true);
}

bool Ruleset::exitGoldenScore(std::unique_ptr<MatchStore> & match) {
    match->setGoldenScore(false);
}

bool Ruleset::stop(std::unique_ptr<MatchStore> & match, std::chrono::high_resolution_clock::time_point time, std::chrono::high_resolution_clock::duration clock) {
    match->setTime(time);
    match->setClock(clock);
    match->stop();
}

bool Ruleset::resume(std::unique_ptr<MatchStore> & match, std::chrono::high_resolution_clock::time_point time, std::chrono::high_resolution_clock::duration clock) {
    match->setTime(time);
    match->setClock(clock);
    match->resume();
}

bool Ruleset::subtractIppon(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) {
    PlayerScore & score = match->getPlayerScore(playerIndex);
    if (score.ippon < 1)
        return false;
    score.ippon -= 1;
    return true;
}

bool Ruleset::addIppon(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) {
    PlayerScore & score = match->getPlayerScore(playerIndex);
    score.ippon += 1;
    return true;
}

