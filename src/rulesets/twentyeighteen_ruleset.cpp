#include "rulesets/twentyeighteen_ruleset.hpp"

bool TwentyEighteenRuleset::addWazari(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) const {
    PlayerScore & score = match->getPlayerScore(playerIndex);
    if (score.ippon == 1)
        return false;
    score.wazari = (score.wazari + 1) % 2;
    score.ippon = (score.wazari == 0);
    return true;
}

bool TwentyEighteenRuleset::subtractWazari(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) const {
    PlayerScore & score = match->getPlayerScore(playerIndex);
    if (score.ippon == 0 && score.wazari == 0)
        return false;
    score.wazari = (score.wazari + 1) % 2;
    score.ippon = 0;

    return true;
}

bool TwentyEighteenRuleset::addShido(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) const {
    PlayerScore & score = match->getPlayerScore(playerIndex);
    if (score.hansokuMake == 1)
        return false;
    score.shido = (score.shido + 1) % 2;
    score.hansokuMake = (score.shido == 0);
    return true;
}

bool TwentyEighteenRuleset::subtractShido(std::unique_ptr<MatchStore> & match, MatchStore::PlayerIndex playerIndex) const {
    PlayerScore & score = match->getPlayerScore(playerIndex);
    if (score.hansokuMake == 0 && score.shido == 0)
        return false;
    score.wazari = (score.wazari + 2) % 3;
    score.hansokuMake = 0;
    return true;
}


bool TwentyEighteenRuleset::isFinished(std::unique_ptr<MatchStore> & match) const {
    if (!match->isStopped())
        return false;

    PlayerScore & whiteScore = match->getPlayerScore(MatchStore::PlayerIndex::WHITE);
    PlayerScore & blueScore = match->getPlayerScore(MatchStore::PlayerIndex::BLUE);

    if (whiteScore.hansokuMake || blueScore.hansokuMake) // also handle the case when both players are disqualified
        return true;
    return getWinner(match).has_value();
}

bool TwentyEighteenRuleset::shouldStop(std::unique_ptr<MatchStore> & match) const {
    if (match->getCurrentClock() > NORMAL_TIME && !match->isGoldenScore())
        return true;
    if (getWinner(match).has_value())
        return true;
    return false;
}

bool TwentyEighteenRuleset::shouldEnterGoldenScore(std::unique_ptr<MatchStore> & match) const {
    if (getWinner(match).has_value())
        return false;
    if (match->getCurrentClock() >= NORMAL_TIME && !match->isGoldenScore())
        return true;
    return false;
}

std::optional<MatchStore::PlayerIndex> TwentyEighteenRuleset::getWinner(std::unique_ptr<MatchStore> & match) const {
    PlayerScore & whiteScore = match->getPlayerScore(MatchStore::PlayerIndex::WHITE);
    PlayerScore & blueScore = match->getPlayerScore(MatchStore::PlayerIndex::BLUE);
    auto currentClock = match->getCurrentClock();

    if (whiteScore.ippon == 1)
        return MatchStore::PlayerIndex::WHITE;
    if (whiteScore.hansokuMake == 0 && blueScore.hansokuMake == 1) // also handle the case when both players are disqualified
        return MatchStore::PlayerIndex::WHITE;
    if (currentClock >= NORMAL_TIME && whiteScore.wazari > blueScore.wazari)
        return MatchStore::PlayerIndex::WHITE;

    if (blueScore.ippon == 1)
        return MatchStore::PlayerIndex::BLUE;
    if (blueScore.hansokuMake == 0 && whiteScore.hansokuMake == 1) // also handle the case when both players are disqualified
        return MatchStore::PlayerIndex::BLUE;
    if (currentClock >= NORMAL_TIME && blueScore.wazari > whiteScore.wazari)
        return MatchStore::PlayerIndex::BLUE;
    return std::nullopt;
}

bool TwentyEighteenRuleset::stop(std::unique_ptr<MatchStore> & match, std::chrono::high_resolution_clock::time_point time, std::chrono::high_resolution_clock::duration clock) const {
    if (match->isStopped())
        return false;
    match->setTime(time);
    if (clock > NORMAL_TIME && !match->isGoldenScore())
        match->setClock(NORMAL_TIME);
    else
        match->setClock(clock);
    match->stop();
    return true;
}

bool TwentyEighteenRuleset::resume(std::unique_ptr<MatchStore> & match, std::chrono::high_resolution_clock::time_point time, std::chrono::high_resolution_clock::duration clock) const {
    if (!match->isStopped())
        return false;
    match->setTime(time);
    match->setClock(clock);
    match->resume();
    return true;
}

