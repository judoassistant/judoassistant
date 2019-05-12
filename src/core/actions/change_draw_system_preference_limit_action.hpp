#pragma once

#include "core/actions/action.hpp"

class TournamentStore;

class ChangeDrawSystemPreferenceLimitAction : public Action {
public:
    ChangeDrawSystemPreferenceLimitAction() = default;
    ChangeDrawSystemPreferenceLimitAction(std::size_t row, std::size_t limit);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mRow, mLimit);
    }

private:
    std::size_t mRow;
    std::size_t mLimit;

    // undo members
    std::size_t mPrevLimit;
};

CEREAL_REGISTER_TYPE(ChangeDrawSystemPreferenceLimitAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeDrawSystemPreferenceLimitAction)

