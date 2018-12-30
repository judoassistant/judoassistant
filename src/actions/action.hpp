#pragma once

#include "core.hpp"
#include "serialize.hpp"

// TODO: Implement serialization for all actions
class TournamentStore;
class CategoryId;
class MatchId;

class Action {
public:
    Action();
    virtual ~Action() {}

    virtual std::string getDescription() const = 0;
    virtual bool shouldDisplay(CategoryId categoryId, MatchId matchId) const;

    virtual std::unique_ptr<Action> freshClone() const = 0;

    void redo(TournamentStore & tournament);
    void undo(TournamentStore & tournament);

    bool isDone() const;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {}

protected:
    virtual void redoImpl(TournamentStore & tournament) = 0;
    virtual void undoImpl(TournamentStore & tournament) = 0;

private:
    bool mDone;
};

