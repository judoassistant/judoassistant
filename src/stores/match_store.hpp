#pragma once

#include <optional>
#include <vector>
#include <chrono>

#include "serialize.hpp"
#include "core.hpp"

#include "stores/match_event.hpp"

class MatchStore;
class MatchEvent;
class Ruleset;

struct PlayerScore {
    uint8_t ippon;
    uint8_t wazari;
    uint8_t shido;
    bool hansokuMake;


    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar("ippon", ippon);
        ar("wazari", wazari);
        ar("shido", shido);
        ar("hansokuMake", hansokuMake);
    }
};

class MatchStore {
public:
    enum class PlayerIndex {
        WHITE = 0,
        BLUE = 1
    };

    MatchStore() {}
    MatchStore(Id id, std::optional<Id> whitePlayer, std::optional<Id> bluePlayer);

    Id getId() const;
    std::optional<Id> getPlayer(PlayerIndex index) const;
    PlayerScore & getPlayerScore(PlayerIndex index);

    void pushEvent(std::unique_ptr<MatchEvent> && event);
    const std::vector<std::unique_ptr<MatchEvent>> & getEvents() const;

    // TODO: Make sure clock is serializeable(hint: it's probably not..)
    // TODO: rename time and clock to something more logical
    std::chrono::high_resolution_clock::time_point getTime() const;
    std::chrono::high_resolution_clock::duration getClock() const;
    std::chrono::high_resolution_clock::duration getCurrentClock() const;
    void setTime(const std::chrono::high_resolution_clock::time_point & time);
    void setClock(const std::chrono::high_resolution_clock::duration & clock);
    void stop();
    void resume();
    bool isStopped() const;
    bool isGoldenScore() const;
    void setGoldenScore(bool val);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar("id", mId);
        ar("scores", mScores);
        ar("players", mPlayers);
        ar("isStopped", mIsStopped);
        ar("goldenScore", mGoldenScore);
        ar("time", mTime);
        ar("clock", mClock);

        // TODO: serialize match events
        // ar("events", mEvents);
    }
private:
    Id mId;
    std::array<PlayerScore,2> mScores;
    std::array<std::optional<Id>,2> mPlayers;
    bool mIsStopped;
    bool mGoldenScore; // whether the match is currently in golden score or not
    std::chrono::high_resolution_clock::time_point mTime; // the time when clock was last resumed
    std::chrono::high_resolution_clock::duration mClock; // the value of the clock when it was last resumed
    std::vector<std::unique_ptr<MatchEvent>> mEvents;
};

