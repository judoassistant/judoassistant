#include "core/rulesets/twenty_eighteen_ruleset.hpp"

bool TwentyEighteenRuleset::canAddWazari(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto & score = match.getScore(playerIndex);
    const auto &otherScore = match.getScore(playerIndex == MatchStore::PlayerIndex::WHITE ? MatchStore::PlayerIndex::BLUE : MatchStore::PlayerIndex::WHITE);

    return (otherScore.ippon == 0 && score.ippon == 0);
}

void TwentyEighteenRuleset::addWazari(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const {
    assert(canAddWazari(match, playerIndex));

    auto & score = match.getScore(playerIndex);
    score.wazari = (score.wazari + 1) % 2;
    score.ippon = (score.wazari == 0);

    updateStatus(match, masterTime);
}

// bool TwentyEighteenRuleset::canSubtractWazari(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
//     const auto & score = match.getScore(playerIndex);
//     return (score.ippon > 0 || score.wazari > 0);
// }

// void TwentyEighteenRuleset::subtractWazari(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const {
//     assert(canSubtractWazari(match, playerIndex));

//     auto & score = match.getScore(playerIndex);
//     score.wazari = (score.wazari + 1) % 2;
//     score.ippon = 0;
// }

bool TwentyEighteenRuleset::canAddShido(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto & score = match.getScore(playerIndex);
    return (score.hansokuMake == 0);
}

void TwentyEighteenRuleset::addShido(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const {
    assert(canAddShido(match, playerIndex));

    auto & score = match.getScore(playerIndex);
    score.shido = (score.shido + 1) % 3;
    score.hansokuMake = (score.shido == 0);

    updateStatus(match, masterTime);
}

// bool TwentyEighteenRuleset::canSubtractShido(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
//     const auto & score = match.getScore(playerIndex);
//     return (score.hansokuMake == 1 || score.shido > 0);
// }

// void TwentyEighteenRuleset::subtractShido(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const {
//     assert(canSubtractShido(match, playerIndex));

//     auto & score = match.getScore(playerIndex);
//     score.shido = (score.shido + 2) % 3;
//     score.hansokuMake = 0;
// }

bool TwentyEighteenRuleset::isFinished(const MatchStore &match, std::chrono::milliseconds masterTime) const {
    if (match.isBye())
        return true;
    if (!match.getWhitePlayer().has_value())
        return false;
    if (!match.getBluePlayer().has_value())
        return false;
    if (match.getStatus() == MatchStatus::UNPAUSED)
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

    // TODO: Implement matches ending in a draw
    if (blueScore.hansokuMake == 1)
        return MatchStore::PlayerIndex::WHITE;
    if (whiteScore.hansokuMake == 1)
        return MatchStore::PlayerIndex::BLUE;

    if (whiteScore.ippon == 1)
        return MatchStore::PlayerIndex::WHITE;
    if (blueScore.ippon == 1)
        return MatchStore::PlayerIndex::BLUE;

    if (currentDuration >= getNormalTime() && whiteScore.wazari > blueScore.wazari)
        return MatchStore::PlayerIndex::WHITE;
    if (currentDuration >= getNormalTime() && blueScore.wazari > whiteScore.wazari)
        return MatchStore::PlayerIndex::BLUE;
    return std::nullopt;
}

// TODO: Avoid duplicating code
std::optional<MatchStore::PlayerIndex> TwentyEighteenRuleset::getWinner(const MatchStore &match) const {
    if (!match.getWhitePlayer().has_value())
        return MatchStore::PlayerIndex::BLUE;
    if (!match.getBluePlayer().has_value())
        return MatchStore::PlayerIndex::WHITE;

    assert(match.getStatus() == MatchStatus::FINISHED);

    const auto & whiteScore = match.getWhiteScore();
    const auto & blueScore = match.getBlueScore();

    if (blueScore.hansokuMake == 1)
        return MatchStore::PlayerIndex::WHITE;
    if (whiteScore.hansokuMake == 1)
        return MatchStore::PlayerIndex::BLUE;
    if (whiteScore.ippon == 1)
        return MatchStore::PlayerIndex::WHITE;
    if (blueScore.ippon == 1)
        return MatchStore::PlayerIndex::BLUE;

    if (whiteScore.wazari > blueScore.wazari)
        return MatchStore::PlayerIndex::WHITE;
    if (blueScore.wazari > whiteScore.wazari)
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

std::chrono::milliseconds TwentyEighteenRuleset::getExpectedTime() const {
    return std::chrono::minutes(6);
}

// TODO: Implement banning of players on direct hansoku make
bool TwentyEighteenRuleset::canAddHansokuMake(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto & score = match.getScore(playerIndex);
    const auto &otherScore = match.getScore(playerIndex == MatchStore::PlayerIndex::WHITE ? MatchStore::PlayerIndex::BLUE : MatchStore::PlayerIndex::WHITE);
    return (score.hansokuMake == 0 && otherScore.hansokuMake == 0);
}

void TwentyEighteenRuleset::addHansokuMake(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const {
    assert(canAddHansokuMake(match, playerIndex));

    auto & score = match.getScore(playerIndex);
    score.hansokuMake = 1;

    updateStatus(match, masterTime);
}

// bool TwentyEighteenRuleset::canSubtractHansokuMake(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
//     const auto & score = match.getScore(playerIndex);
//     return (score.hansokuMake == 1);
// }

// void TwentyEighteenRuleset::subtractHansokuMake(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const {
//     assert(canSubtractHansokuMake(match, playerIndex));

//     auto & score = match.getScore(playerIndex);
//     score.hansokuMake = 0;
// }

