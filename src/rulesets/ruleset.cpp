#include "rulesets/ruleset.hpp"

bool Ruleset::canAddIppon(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto & score = match.getScore(playerIndex);
    return (score.ippon == 0);
}

void Ruleset::addIppon(MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    assert(canAddIppon(match, playerIndex));

    auto & score = match.getScore(playerIndex);
    score.ippon += 1;
}

bool Ruleset::canSubtractIppon(const MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    const auto & score = match.getScore(playerIndex);
    return (score.ippon > 0);
}

void Ruleset::subtractIppon(MatchStore &match, MatchStore::PlayerIndex playerIndex) const {
    assert(canSubtractIppon(match, playerIndex));

    auto & score = match.getScore(playerIndex);
    score.ippon -= 1;
}

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
    if (isFinished(match, masterTime))
        match.setStatus(MatchStatus::FINISHED);
}

bool Ruleset::canResume(const MatchStore &match, std::chrono::milliseconds masterTime) const {
    return (match.getStatus() != MatchStatus::UNPAUSED && !isFinished(match, masterTime));
}

void Ruleset::resume(MatchStore &match, std::chrono::milliseconds masterTime) const {
    assert(canResume(match, masterTime));

    match.setResumeTime(masterTime);
    match.setStatus(MatchStatus::UNPAUSED);
}

