#include "rulesets/twentyeighteen_ruleset.hpp"

bool TwentyEighteenRuleset::canAddWazari(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto & score = match.getScore(playerIndex);
    return (score.ippon < 1);
}

void TwentyEighteenRuleset::addWazari(MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    assert(canAddWazari(match, playerIndex));

    auto & score = match.getScore(playerIndex);
    score.wazari = (score.wazari + 1) % 2;
    score.ippon = (score.wazari == 0);
}

bool TwentyEighteenRuleset::canSubtractWazari(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto & score = match.getScore(playerIndex);
    return (score.ippon > 0 || score.wazari > 0);
}

void TwentyEighteenRuleset::subtractWazari(MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    assert(canSubtractWazari(match, playerIndex));

    auto & score = match.getScore(playerIndex);
    score.wazari = (score.wazari + 1) % 2;
    score.ippon = 0;
}

bool TwentyEighteenRuleset::canAddShido(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto & score = match.getScore(playerIndex);
    return (score.hansokuMake == 0);
}

void TwentyEighteenRuleset::addShido(MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    assert(canAddShido(match, playerIndex));

    auto & score = match.getScore(playerIndex);
    score.shido = (score.shido + 1) % 2;
    score.hansokuMake = (score.shido == 0);
}

bool TwentyEighteenRuleset::canSubtractShido(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto & score = match.getScore(playerIndex);
    return (score.hansokuMake == 1 || score.shido > 0);
}

void TwentyEighteenRuleset::subtractShido(MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    assert(canSubtractShido(match, playerIndex));

    auto & score = match.getScore(playerIndex);
    score.wazari = (score.wazari + 2) % 3;
    score.hansokuMake = 0;
}


bool TwentyEighteenRuleset::isFinished(const MatchStore &match, std::chrono::milliseconds masterTime) const {
    if (match.getStatus() == MatchStatus::FINISHED)
        return true;
    if (match.isBye())
        return true;
    if (!match.getWhitePlayer().has_value())
        return false;
    if (!match.getBluePlayer().has_value())
        return false;
    if (match.getStatus() == MatchStatus::NOT_STARTED || match.getStatus() == MatchStatus::UNPAUSED)
        return false;

    const auto & whiteScore = match.getWhiteScore();
    const auto & blueScore = match.getBlueScore();

    if (whiteScore.hansokuMake || blueScore.hansokuMake) // also handle the case when both players are disqualified
        return true;
    return getWinner(match, masterTime).has_value();
}

std::optional<MatchStore::PlayerIndex> TwentyEighteenRuleset::getWinner(const MatchStore &match, std::chrono::milliseconds masterTime) const {
    if (!match.getWhitePlayer().has_value())
        return MatchStore::PlayerIndex::BLUE;
    if (!match.getBluePlayer().has_value())
        return MatchStore::PlayerIndex::WHITE;

    const auto & whiteScore = match.getWhiteScore();
    const auto & blueScore = match.getBlueScore();
    auto currentDuration = match.currentDuration(masterTime);

    if (whiteScore.ippon == 1)
        return MatchStore::PlayerIndex::WHITE;
    if (whiteScore.hansokuMake == 0 && blueScore.hansokuMake == 1) // also handle the case when both players are disqualified
        return MatchStore::PlayerIndex::WHITE;
    if (currentDuration >= getNormalTime() && whiteScore.wazari > blueScore.wazari)
        return MatchStore::PlayerIndex::WHITE;

    if (blueScore.ippon == 1)
        return MatchStore::PlayerIndex::BLUE;
    if (blueScore.hansokuMake == 0 && whiteScore.hansokuMake == 1) // also handle the case when both players are disqualified
        return MatchStore::PlayerIndex::BLUE;
    if (currentDuration >= getNormalTime() && blueScore.wazari > whiteScore.wazari)
        return MatchStore::PlayerIndex::BLUE;
    return std::nullopt;
}

std::unique_ptr<Ruleset> TwentyEighteenRuleset::clone() const {
    return std::make_unique<TwentyEighteenRuleset>();
}

std::string TwentyEighteenRuleset::getName() const {
    // TODO: Handle delayed translation of this
    return "2018";
}

std::chrono::milliseconds TwentyEighteenRuleset::getNormalTime() const {
    return std::chrono::minutes(4);
}
