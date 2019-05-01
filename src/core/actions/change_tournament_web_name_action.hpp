#pragma once

#include "core/core.hpp"
#include "core/actions/action.hpp"

class TournamentStore;

class ChangeTournamentWebNameAction : public Action {
public:
    ChangeTournamentWebNameAction() = default;
    ChangeTournamentWebNameAction(const std::string &name);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mWebName);
    }

private:
    std::string mWebName;

    // undo members
    std::string mOldWebName;
};

CEREAL_REGISTER_TYPE(ChangeTournamentWebNameAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeTournamentWebNameAction)

