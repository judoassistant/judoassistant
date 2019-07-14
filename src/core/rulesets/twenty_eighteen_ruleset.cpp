#include "core/rulesets/twenty_eighteen_ruleset.hpp"

bool TwentyEighteenRuleset::canAwardIppon(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto &score = match.getScore(playerIndex);
    const auto &otherScore = match.getScore(playerIndex == MatchStore::PlayerIndex::WHITE ? MatchStore::PlayerIndex::BLUE : MatchStore::PlayerIndex::WHITE);

    return (!otherScore.ippon && !score.ippon);
}

void TwentyEighteenRuleset::awardIppon(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const {
    assert(canAwardIppon(match, playerIndex));

    auto & score = match.getScore(playerIndex);
    score.ippon = true;
    score.directIppon = true;

    updateStatus(match, masterTime);
}

bool TwentyEighteenRuleset::canCancelIppon(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto &score = match.getScore(playerIndex);
    return score.directIppon;
}

void TwentyEighteenRuleset::cancelIppon(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const {
    assert(canCancelIppon(match, playerIndex));

    auto & score = match.getScore(playerIndex);
    score.directIppon = false;
    if (score.wazari < 2)
        score.ippon = false;

    updateStatus(match, masterTime);
}

bool TwentyEighteenRuleset::canAwardWazari(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto &score = match.getScore(playerIndex);
    const auto &otherScore = match.getScore(playerIndex == MatchStore::PlayerIndex::WHITE ? MatchStore::PlayerIndex::BLUE : MatchStore::PlayerIndex::WHITE);

    return (!otherScore.ippon && !score.ippon);
}

void TwentyEighteenRuleset::awardWazari(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const {
    assert(canAwardWazari(match, playerIndex));

    auto & score = match.getScore(playerIndex);
    score.wazari += 1;
    if (score.wazari == 2)
        score.ippon = true;

    updateStatus(match, masterTime);
}

bool TwentyEighteenRuleset::canCancelWazari(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto & score = match.getScore(playerIndex);
    return score.wazari > 0;
}

void TwentyEighteenRuleset::cancelWazari(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const {
    assert(canCancelWazari(match, playerIndex));

    auto & score = match.getScore(playerIndex);
    score.wazari -= 1;
    if (score.ippon && !score.directIppon)
        score.ippon = false;

    updateStatus(match, masterTime);
}

bool TwentyEighteenRuleset::canAwardShido(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto &score = match.getScore(playerIndex);
    const auto &otherScore = match.getScore(playerIndex == MatchStore::PlayerIndex::WHITE ? MatchStore::PlayerIndex::BLUE : MatchStore::PlayerIndex::WHITE);
    return !score.hansokuMake && !otherScore.hansokuMake;
}

void TwentyEighteenRuleset::awardShido(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const {
    assert(canAwardShido(match, playerIndex));

    auto & score = match.getScore(playerIndex);
    score.shido += 1;
    if (score.shido == 3)
        score.hansokuMake = true;

    updateStatus(match, masterTime);
}

bool TwentyEighteenRuleset::canCancelShido(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto & score = match.getScore(playerIndex);
    return score.shido > 0;
}

void TwentyEighteenRuleset::cancelShido(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const {
    assert(canCancelShido(match, playerIndex));

    auto & score = match.getScore(playerIndex);
    score.shido -= 1;
    if (score.hansokuMake && !score.directHansokuMake)
        score.hansokuMake = false;

    updateStatus(match, masterTime);
}

bool TwentyEighteenRuleset::isFinished(const MatchStore &match, std::chrono::milliseconds masterTime) const {
    if (match.isBye())
        return true;
    if (!match.getWhitePlayer().has_value())
        return false;
    if (!match.getBluePlayer().has_value())
        return false;
    if (match.getStatus() == MatchStatus::UNPAUSED)
        return false;
    if (match.getOsaekomi().has_value())
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
    return "2018";
}

std::chrono::milliseconds TwentyEighteenRuleset::getNormalTime() const {
    return std::chrono::minutes(4);
}

std::chrono::milliseconds TwentyEighteenRuleset::getExpectedTime() const {
    return std::chrono::minutes(6);
}

bool TwentyEighteenRuleset::canAwardHansokuMake(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto &score = match.getScore(playerIndex);
    const auto &otherScore = match.getScore(playerIndex == MatchStore::PlayerIndex::WHITE ? MatchStore::PlayerIndex::BLUE : MatchStore::PlayerIndex::WHITE);
    return !score.directHansokuMake && !otherScore.hansokuMake; // We can overwrite a non-direct hansoku. However not if the other player has (possibly indirect) hansoku already
}

void TwentyEighteenRuleset::awardHansokuMake(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const {
    assert(canAwardHansokuMake(match, playerIndex));

    auto & score = match.getScore(playerIndex);
    score.hansokuMake = true;
    score.directHansokuMake = true;

    updateStatus(match, masterTime);
}

bool TwentyEighteenRuleset::canCancelHansokuMake(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto & score = match.getScore(playerIndex);
    return score.directHansokuMake;
}

void TwentyEighteenRuleset::cancelHansokuMake(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const {
    assert(canCancelHansokuMake(match, playerIndex));

    auto & score = match.getScore(playerIndex);
    score.directHansokuMake = false;

    if (score.shido < 3)
        score.hansokuMake = false;

    updateStatus(match, masterTime);
}

std::chrono::milliseconds TwentyEighteenRuleset::getOsaekomiIpponTime() const {
    return std::chrono::seconds(20);
}

std::chrono::milliseconds TwentyEighteenRuleset::getOsaekomiWazariTime() const {
    return std::chrono::seconds(10);
}

RulesetIdentifier TwentyEighteenRuleset::getIdentifier() const {
    return RulesetIdentifier::TWENTY_EIGHTEEN;
}

