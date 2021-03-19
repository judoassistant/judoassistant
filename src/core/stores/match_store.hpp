#pragma once

#include <optional>
#include <vector>
#include <chrono>

#include "core/core.hpp"
#include "core/id.hpp"
#include "core/serialize.hpp"

class MatchStore;
class Ruleset;

enum class MatchType {
    ELIMINATION, FINAL
};

enum class MatchStatus {
    NOT_STARTED, PAUSED, UNPAUSED, FINISHED
};

struct MatchEvent;

class MatchStore {
public:
    enum class PlayerIndex {
        WHITE, BLUE
    };

    struct Score {
        Score();

        unsigned int wazari;
        unsigned int shido;
        bool ippon;
        bool directIppon; // Used for checks before cancelling ippon
        bool hansokuMake;
        bool directHansokuMake; // Used for checks before cancelling hansoku

        template<typename Archive>
        void serialize(Archive& ar, uint32_t const version) {
            ar(wazari, shido, ippon, directIppon, hansokuMake, directHansokuMake);
        }
    };

    // Struct containing score and related fields.
    // Used for saving state in actions
    struct State {
        State(bool finished = false);

        MatchStatus status;
        bool goldenScore; // whether the match is currently in golden score or not
        std::chrono::milliseconds resumeTime; // the time when the clock was last resumed
        std::chrono::milliseconds duration; // the match duration when the clock was last paused
        std::array<Score,2> scores;
        std::optional<std::pair<PlayerIndex, std::chrono::milliseconds>> osaekomi;
        bool osaekomiWazari;

        template<typename Archive>
        void serialize(Archive& ar, uint32_t const version) {
            ar(status, goldenScore, resumeTime, duration, scores, osaekomi, osaekomiWazari);
        }
    };

    MatchStore() {}
    MatchStore(const MatchStore &other);
    MatchStore(const CombinedId &combinedId, MatchType type, const std::string &title, bool permanentBye, std::optional<PlayerId> whitePlayer, std::optional<PlayerId> bluePlayer);

    std::optional<PlayerId> getPlayer(PlayerIndex index) const;
    std::optional<PlayerId> getWhitePlayer() const;
    std::optional<PlayerId> getBluePlayer() const;

    void setPlayer(PlayerIndex index, std::optional<PlayerId> playerId);

    Score & getScore(PlayerIndex color);
    Score & getWhiteScore();
    Score & getBlueScore();

    const Score & getScore(PlayerIndex color) const;
    const Score & getWhiteScore() const;
    const Score & getBlueScore() const;

    const std::string & getTitle() const;
    MatchType getType() const;

    bool isPermanentBye() const;

    bool isBye() const;
    void setBye(bool bye);

    void pushEvent(const MatchEvent & event);
    void popEvent();
    void clearEvents();
    const std::vector<MatchEvent>& getEvents() const;
    std::vector<MatchEvent>& getEvents();
    void setEvents(const std::vector<MatchEvent> &events);

    void finish();
    bool isGoldenScore() const;
    void setGoldenScore(bool val);

    MatchStatus getStatus() const;
    void setStatus(MatchStatus status);

    MatchId getId() const;
    CategoryId getCategoryId() const;
    CombinedId getCombinedId() const;

    void setDuration(std::chrono::milliseconds duration);
    std::chrono::milliseconds getDuration() const;

    std::chrono::milliseconds currentDuration(std::chrono::milliseconds masterTime) const;

    void setResumeTime(std::chrono::milliseconds resumeTime);
    std::chrono::milliseconds getResumeTime() const;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCombinedId, mType, mTitle, mPermanentBye, mBye, mPlayers, mState, mEvents);
    }

    const std::optional<std::pair<PlayerIndex, std::chrono::milliseconds>>& getOsaekomi() const;
    void setOsaekomi(const std::optional<std::pair<PlayerIndex, std::chrono::milliseconds>>& value);

    std::chrono::milliseconds currentOsaekomiTime(std::chrono::milliseconds masterTime) const;

    bool isOsaekomiWazari() const;
    void setOsaekomiWazari(bool val);

    State& getState();
    const State& getState() const;
    void setState(const State &state);
    void clearState();
private:
    CombinedId mCombinedId;
    MatchType mType;
    std::string mTitle;
    bool mPermanentBye;
    bool mBye;
    std::array<std::optional<PlayerId>,2> mPlayers;
    State mState;
    std::vector<MatchEvent> mEvents;
};

enum class MatchEventType {
    IPPON, WAZARI, SHIDO, HANSOKU_MAKE, IPPON_OSAEKOMI, WAZARI_OSAEKOMI,
    CANCEL_IPPON, CANCEL_WAZARI, CANCEL_SHIDO, CANCEL_HANSOKU_MAKE,
};

struct MatchEvent {
    MatchEventType type;
    MatchStore::PlayerIndex playerIndex;
    std::chrono::milliseconds duration; // match duration at the time of the event

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(type, playerIndex, duration);
    }
};

std::ostream &operator<<(std::ostream &out, const MatchType &matchType);

