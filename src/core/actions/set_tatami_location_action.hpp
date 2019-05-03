#pragma once

#include "core/id.hpp"
#include "core/actions/action.hpp"
#include "core/stores/match_store.hpp"
#include "core/stores/tatami/location.hpp"

class TournamentStore;
class BlockLocation;

class SetTatamiLocationAction : public Action {
public:
    SetTatamiLocationAction() = default;
    SetTatamiLocationAction(std::pair<CategoryId, MatchType> block, std::optional<BlockLocation> location);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mBlock, mLocation);
    }

private:
    std::pair<CategoryId, MatchType> mBlock;
    std::optional<BlockLocation> mLocation;

    // undo members
    bool mPositionSet;
    std::optional<BlockLocation> mOldLocation;
};

CEREAL_REGISTER_TYPE(SetTatamiLocationAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, SetTatamiLocationAction)

