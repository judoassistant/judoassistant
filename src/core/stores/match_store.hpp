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

        uint8_t ippon;
        uint8_t wazari;
        uint8_t shido;
        bool hansokuMake;

        template<typename Archive>
        void serialize(Archive& ar, uint32_t const version) {
            ar(ippon, wazari, shido, hansokuMake);
        }
    };

    MatchStore() {}
    MatchStore(const MatchStore &other);
    MatchStore(MatchId id, CategoryId categoryId, MatchType type, const std::string &title, bool permanentBye, std::optional<PlayerId> whitePlayer, std::optional<PlayerId> bluePlayer);

    MatchId getId() const;
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
    const std::vector<MatchEvent> & getEvents() const;

    void finish();
    bool isGoldenScore() const;
    void setGoldenScore(bool val);

    MatchStatus getStatus() const;
    void setStatus(MatchStatus status);

    CategoryId getCategory() const;
    std::pair<CategoryId, MatchId> getCombinedId() const;

    void setDuration(std::chrono::milliseconds duration);
    std::chrono::milliseconds getDuration() const;

    std::chrono::milliseconds currentDuration(std::chrono::milliseconds masterTime) const;

    void setResumeTime(std::chrono::milliseconds resumeTime);
    std::chrono::milliseconds getResumeTime() const;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mId, mCategory, mType, mTitle, mPermanentBye, mBye, mScores, mPlayers, mStatus, mGoldenScore, mResumeTime, mDuration, mEvents, mOsaekomi, mHasAwardedOsaekomiWazari);
    }

    const std::optional<std::pair<PlayerIndex, std::chrono::milliseconds>>& getOsaekomi() const;
    void setOsaekomi(const std::optional<std::pair<PlayerIndex, std::chrono::milliseconds>>& value);

    std::chrono::milliseconds currentOsaekomiTime(std::chrono::milliseconds masterTime) const;

    bool hasAwardedOsaekomiWazari() const;
    void setHasAwardedOsaekomiWazari(bool val);
private:
    MatchId mId;
    CategoryId mCategory;
    MatchType mType;
    std::string mTitle;
    bool mPermanentBye;
    bool mBye;
    std::array<Score,2> mScores;
    std::array<std::optional<PlayerId>,2> mPlayers;
    MatchStatus mStatus;
    bool mGoldenScore; // whether the match is currently in golden score or not
    std::chrono::milliseconds mResumeTime; // the time when the clock was last resumed
    std::chrono::milliseconds mDuration; // the match duration when the clock was last paused
    std::vector<MatchEvent> mEvents;

    std::optional<std::pair<PlayerIndex, std::chrono::milliseconds>> mOsaekomi;
    bool mHasAwardedOsaekomiWazari;
};

enum class MatchEventType {
    IPPON, WAZARI, SHIDO, HANSOKU_MAKE
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

