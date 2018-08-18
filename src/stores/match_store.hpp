#pragma once

#include <optional>
#include <vector>
#include <chrono>

#include "core.hpp"

class MatchStore;
class MatchEvent;
class Ruleset;

struct PlayerScore {
    uint8_t ippon;
    uint8_t wazari;
    uint8_t shido;
    bool hansokuMake;
};

class MatchStore {
public:
    enum PlayerIndex {
        WHITE = 0,
        BLUE = 1
    };

    MatchStore(Id id, std::optional<Id> whitePlayer, std::optional<Id> bluePlayer);

    Id getId() const;
    std::optional<Id> getPlayer(PlayerIndex index) const;

    void pushEvent(std::unique_ptr<MatchEvent> && event);
    const std::vector<std::unique_ptr<MatchEvent>> & getEvents() const;

    // TODO: Make sure clock is serializeable(hint: it's probably not..)
    std::chrono::high_resolution_clock::time_point getTime() const;
    std::chrono::high_resolution_clock::duration getClock() const;
    void setTime(const std::chrono::high_resolution_clock::time_point & time);
    void setClock(const std::chrono::high_resolution_clock::duration & clock);
private:
    Id mId;
    PlayerScore mScores[2];
    std::optional<Id> mPlayers[2];
    bool mIsPaused;
    std::chrono::high_resolution_clock::time_point mTime; // the time when clock was last resumed
    std::chrono::high_resolution_clock::duration mClock; // the value of the clock when it was last resumed
    std::vector<std::unique_ptr<MatchEvent>> mEvents;
};

class MatchEvent {
public:
    virtual bool operator()(std::unique_ptr<MatchStore> & match, std::unique_ptr<Ruleset> & ruleset) const = 0;
    virtual std::unique_ptr<MatchEvent> getInverse() const = 0;
    virtual std::unique_ptr<MatchEvent> clone() const = 0;
};

class PauseMatchEvent : public MatchEvent {
public:
    PauseMatchEvent(std::unique_ptr<MatchStore> & match);
    PauseMatchEvent(std::chrono::high_resolution_clock::time_point newTime, std::chrono::high_resolution_clock::time_point oldTime, std::chrono::high_resolution_clock::duration newClock, std::chrono::high_resolution_clock::duration oldClock);

    virtual bool operator()(std::unique_ptr<MatchStore> & match, std::unique_ptr<Ruleset> & ruleset) const;
    virtual std::unique_ptr<MatchEvent> getInverse() const;
    virtual std::unique_ptr<MatchEvent> clone() const;
private:
    std::chrono::high_resolution_clock::time_point mNewTime;
    std::chrono::high_resolution_clock::time_point mOldTime;
    std::chrono::high_resolution_clock::duration mNewClock;
    std::chrono::high_resolution_clock::duration mOldClock;
};

class ResumeMatchEvent : public MatchEvent {
public:
    ResumeMatchEvent(std::unique_ptr<MatchStore> & match);
    ResumeMatchEvent(std::chrono::high_resolution_clock::time_point newTime, std::chrono::high_resolution_clock::time_point oldTime, std::chrono::high_resolution_clock::duration newClock, std::chrono::high_resolution_clock::duration oldClock);

    virtual bool operator()(std::unique_ptr<MatchStore> & match, std::unique_ptr<Ruleset> & ruleset) const;
    virtual std::unique_ptr<MatchEvent> getInverse() const;
    virtual std::unique_ptr<MatchEvent> clone() const;
private:
    std::chrono::high_resolution_clock::time_point mNewTime;
    std::chrono::high_resolution_clock::time_point mOldTime;
    std::chrono::high_resolution_clock::duration mNewClock;
    std::chrono::high_resolution_clock::duration mOldClock;
};

