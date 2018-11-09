#pragma once

#include <optional>
#include <vector>
#include <chrono>

#include "serialize.hpp"
#include "core.hpp"
#include "id.hpp"

#include "stores/match_event.hpp"

class MatchStore;
class MatchEvent;
class Ruleset;

enum class MatchType {
    KNOCKOUT, FINAL
};

std::ostream &operator<<(std::ostream &out, const MatchType &matchType);

class MatchStore {
public:
    enum class PlayerIndex {
        WHITE, BLUE
    };

    struct Score {
        uint8_t ippon;
        uint8_t wazari;
        uint8_t shido;
        bool hansokuMake;

        template<typename Archive>
        void serialize(Archive& ar, uint32_t const version) {
            ar(cereal::make_nvp("ippon", ippon));
            ar(cereal::make_nvp("wazari", wazari));
            ar(cereal::make_nvp("shido", shido));
            ar(cereal::make_nvp("hansokuMake", hansokuMake));
        }
    };

    MatchStore() {}
    MatchStore(const MatchStore &other);
    MatchStore(MatchId id, CategoryId categoryId, MatchType type, const std::string &title, bool bye, std::optional<PlayerId> whitePlayer, std::optional<PlayerId> bluePlayer);

    MatchId getId() const;
    std::optional<PlayerId> getPlayer(PlayerIndex index) const;
    std::optional<PlayerId> getWhitePlayer() const;
    std::optional<PlayerId> getBluePlayer() const;

    Score & getScore(PlayerIndex color);
    Score & getWhiteScore();
    Score & getBlueScore();

    const Score & getScore(PlayerIndex color) const;
    const Score & getWhiteScore() const;
    const Score & getBlueScore() const;

    const std::string & getTitle() const;
    MatchType getType() const;

    bool isBye() const;

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

    CategoryId getCategory() const;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::make_nvp("id", mId));
        ar(cereal::make_nvp("scores", mScores));
        ar(cereal::make_nvp("players", mPlayers));
        ar(cereal::make_nvp("isStopped", mIsStopped));
        ar(cereal::make_nvp("goldenScore", mGoldenScore));
        ar(cereal::make_nvp("time", mTime));
        ar(cereal::make_nvp("clock", mClock));
        ar(cereal::make_nvp("type", mType));
        ar(cereal::make_nvp("bye", mBye));
        ar(cereal::make_nvp("title", mTitle));

        // TODO: serialize match events
        // ar("events", mEvents);
    }
private:
    MatchId mId;
    CategoryId mCategory;
    MatchType mType;
    std::string mTitle;
    bool mBye;
    std::array<Score,2> mScores;
    std::array<std::optional<PlayerId>,2> mPlayers;
    bool mIsStopped;
    bool mGoldenScore; // whether the match is currently in golden score or not
    std::chrono::high_resolution_clock::time_point mTime; // the time when clock was last resumed
    std::chrono::high_resolution_clock::duration mClock; // the value of the clock when it was last resumed
    std::vector<std::unique_ptr<MatchEvent>> mEvents;
};

