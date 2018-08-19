#include "rulesets/ruleset.hpp"

bool Ruleset::enterGoldenScore(std::unique_ptr<MatchStore> & match) const {
    if (match->isGoldenScore())
        return false;
    match->setGoldenScore(true);
    return true;
}

bool Ruleset::exitGoldenScore(std::unique_ptr<MatchStore> & match) const {
    if (!match->isGoldenScore())
        return false;
    match->setGoldenScore(false);
    return true;
}

bool Ruleset::subtractIppon(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) const {
    PlayerScore & score = match->getPlayerScore(playerIndex);
    if (score.ippon < 1)
        return false;
    score.ippon -= 1;
    return true;
}

bool Ruleset::addIppon(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) const {
    PlayerScore & score = match->getPlayerScore(playerIndex);
    if (score.ippon > 0)
        return false;
    score.ippon += 1;
    return true;
}

