#pragma once

#include "core.hpp"
#include "serialize.hpp"

class MatchStore;
class Ruleset;

class MatchEvent {
public:
    virtual ~MatchEvent() {}

    virtual bool operator()(std::unique_ptr<MatchStore> & match, std::unique_ptr<Ruleset> & ruleset) const = 0;
    virtual std::unique_ptr<MatchEvent> getInverse() const = 0;
    virtual std::unique_ptr<MatchEvent> clone() const = 0;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {}
};

// TODO: Avoid code duplication for symmetric events
class StopMatchEvent : public MatchEvent {
public:
    StopMatchEvent() {}
    StopMatchEvent(std::unique_ptr<MatchStore> & match);
    StopMatchEvent(std::chrono::high_resolution_clock::time_point newTime, std::chrono::high_resolution_clock::time_point oldTime, std::chrono::high_resolution_clock::duration newClock, std::chrono::high_resolution_clock::duration oldClock);

    bool operator()(std::unique_ptr<MatchStore> & match, std::unique_ptr<Ruleset> & ruleset) const override;
    std::unique_ptr<MatchEvent> getInverse() const override;
    std::unique_ptr<MatchEvent> clone() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mNewTime);
        ar(mOldTime);
        ar(mNewClock);
        ar(mOldClock);
    }
private:
    std::chrono::high_resolution_clock::time_point mNewTime;
    std::chrono::high_resolution_clock::time_point mOldTime;
    std::chrono::high_resolution_clock::duration mNewClock;
    std::chrono::high_resolution_clock::duration mOldClock;
};

class ResumeMatchEvent : public MatchEvent {
public:
    ResumeMatchEvent() {}
    ResumeMatchEvent(std::unique_ptr<MatchStore> & match);
    ResumeMatchEvent(std::chrono::high_resolution_clock::time_point newTime, std::chrono::high_resolution_clock::time_point oldTime, std::chrono::high_resolution_clock::duration newClock, std::chrono::high_resolution_clock::duration oldClock);

    bool operator()(std::unique_ptr<MatchStore> & match, std::unique_ptr<Ruleset> & ruleset) const override;
    std::unique_ptr<MatchEvent> getInverse() const override;
    std::unique_ptr<MatchEvent> clone() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mNewTime);
        ar(mOldTime);
        ar(mNewClock);
        ar(mOldClock);
    }
private:
    std::chrono::high_resolution_clock::time_point mNewTime;
    std::chrono::high_resolution_clock::time_point mOldTime;
    std::chrono::high_resolution_clock::duration mNewClock;
    std::chrono::high_resolution_clock::duration mOldClock;
};

CEREAL_REGISTER_TYPE(StopMatchEvent)
CEREAL_REGISTER_POLYMORPHIC_RELATION(MatchEvent, StopMatchEvent)
CEREAL_REGISTER_TYPE(ResumeMatchEvent)
CEREAL_REGISTER_POLYMORPHIC_RELATION(MatchEvent, ResumeMatchEvent)
