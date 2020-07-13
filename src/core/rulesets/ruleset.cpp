#include "core/rulesets/ruleset.hpp"
#include "core/rulesets/rulesets.hpp"

bool Ruleset::shouldPause(const MatchStore &match, std::chrono::milliseconds masterTime) const {
    if (match.getStatus() != MatchStatus::UNPAUSED)
        return false;
    if (match.currentDuration(masterTime) > getNormalTime() && !match.isGoldenScore())
        return true;
    if (getWinner(match, masterTime).has_value())
        return true;
    return false;
}

bool Ruleset::canPause(const MatchStore &match, std::chrono::milliseconds masterTime) const {
    return (match.getStatus() == MatchStatus::UNPAUSED);
}

void Ruleset::pause(MatchStore &match, std::chrono::milliseconds masterTime) const {
    assert(canPause(match, masterTime));

    auto currentDuration = match.currentDuration(masterTime);
    if (currentDuration > getNormalTime() && !match.isGoldenScore())
        currentDuration = getNormalTime();

    match.setDuration(currentDuration);

    // The isFinished implementation of twentyeighteen_ruleset returns false if
    // match is unpaused. Therefore match is first paused and then potentially
    // marked as finished.
    match.setStatus(MatchStatus::PAUSED);
    if (isFinished(match, masterTime)) {
        match.setStatus(MatchStatus::FINISHED);
        match.setFinishTime(masterTime);
    }
    else if (currentDuration >= getNormalTime())
        match.setGoldenScore(true);
}

bool Ruleset::canResume(const MatchStore &match, std::chrono::milliseconds masterTime) const {
    return (match.getStatus() != MatchStatus::UNPAUSED && !isFinished(match, masterTime));
}

void Ruleset::resume(MatchStore &match, std::chrono::milliseconds masterTime) const {
    assert(canResume(match, masterTime));

    match.setResumeTime(masterTime);
    match.setStatus(MatchStatus::UNPAUSED);
}

void Ruleset::updateStatus(MatchStore &match, std::chrono::milliseconds masterTime) const {
    if (match.getStatus() == MatchStatus::UNPAUSED)
        return;

    if (isFinished(match, masterTime)) {
        match.setStatus(MatchStatus::FINISHED);
        match.setFinishTime(masterTime);
        if (match.currentDuration(masterTime) <= getNormalTime())
            match.setGoldenScore(false);
    }
    else {
        match.setStatus(MatchStatus::PAUSED);
        if (match.currentDuration(masterTime) >= getNormalTime())
            match.setGoldenScore(true);
    }
}

bool Ruleset::canStartOsaekomi(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto &score = match.getScore(playerIndex);
    const auto &otherScore = match.getScore(playerIndex == MatchStore::PlayerIndex::WHITE ? MatchStore::PlayerIndex::BLUE : MatchStore::PlayerIndex::WHITE);

    return !score.ippon && !otherScore.ippon;
}

void Ruleset::startOsaekomi(MatchStore &match, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime) const {
    assert(canStartOsaekomi(match, playerIndex));

    auto p = std::make_pair(playerIndex, masterTime);
    match.setOsaekomi(p);
    match.setOsaekomiWazari(false);
}

bool Ruleset::canStopOsaekomi(const MatchStore &match, std::chrono::milliseconds /*masterTime*/) const {
    return match.getOsaekomi().has_value();
}

void Ruleset::stopOsaekomi(MatchStore &match, std::chrono::milliseconds masterTime) const {
    match.setOsaekomi(std::nullopt);
    match.setOsaekomiWazari(false);

    if (match.getStatus() == MatchStatus::PAUSED && isFinished(match, masterTime)) {
        match.setStatus(MatchStatus::FINISHED);
        match.setFinishTime(masterTime);
    }
}

bool Ruleset::shouldAwardOsaekomiWazari(const MatchStore &match, std::chrono::milliseconds masterTime) const {
    if (!match.getOsaekomi().has_value())
        return false;
    return (match.currentOsaekomiTime(masterTime) > getOsaekomiWazariTime() && !match.isOsaekomiWazari());
}

bool Ruleset::shouldAwardOsaekomiIppon(const MatchStore &match, std::chrono::milliseconds masterTime) const {
    if (!match.getOsaekomi().has_value())
        return false;
    return (match.currentOsaekomiTime(masterTime) > getOsaekomiIpponTime());
}

bool Ruleset::shouldStopOsaekomi(const MatchStore &match, std::chrono::milliseconds masterTime) const {
    auto osaekomi = match.getOsaekomi();
    if (!osaekomi.has_value())
        return false;
    if (match.getWhiteScore().ippon == 1)
        return true;
    if (match.getBlueScore().ippon == 1)
        return true;

    return false;
}

const std::vector<std::unique_ptr<const Ruleset>> & Ruleset::getRulesets() {
    static std::vector<std::unique_ptr<const Ruleset>> rulesets;

    if (rulesets.empty()) {
        rulesets.push_back(std::make_unique<TwentyEighteenRuleset>());
        rulesets.push_back(std::make_unique<TwentyEighteenTwoMinuteRuleset>());
        rulesets.push_back(std::make_unique<TwentyEighteenThreeMinuteRuleset>());
    }

    return rulesets;
}

std::unique_ptr<Ruleset> Ruleset::getRuleset(RulesetIdentifier identifier) {
    static std::unordered_map<RulesetIdentifier, std::size_t> rulesets;

    if (rulesets.empty()) {
        std::size_t i = 0;
        for (const auto &ruleset : Ruleset::getRulesets())
            rulesets[ruleset->getIdentifier()] = i++;
    }

    return getRulesets()[rulesets.at(identifier)]->clone();
}

std::unique_ptr<Ruleset> Ruleset::getDefaultRuleset() {
    return getRuleset(RulesetIdentifier::TWENTY_EIGHTEEN);
}

