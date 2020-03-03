#pragma once

#include "core/core.hpp"
#include "core/actions/action.hpp"

class TournamentStore;

class ChangeTournamentDateAction : public Action {
public:
    ChangeTournamentDateAction() = default;
    ChangeTournamentDateAction(const std::string &date);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mDate);
    }

private:
    std::string mDate;

    // undo members
    std::string mOldDate;
};

CEREAL_REGISTER_TYPE(ChangeTournamentDateAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeTournamentDateAction)

