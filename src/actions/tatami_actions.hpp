#pragma once

#include "id.hpp"
#include "actions/action.hpp"
#include "stores/match_store.hpp"
#include "stores/tatami/tatami_location.hpp"

class TournamentStore;
class BlockLocation;

// TODO: Introduce alias for std::pair<CategoryId, MatchType>
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
    std::optional<BlockLocation> mOldLocation;
};

CEREAL_REGISTER_TYPE(SetTatamiLocationAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, SetTatamiLocationAction)

class SetTatamiCountAction : public Action {
public:
    SetTatamiCountAction() = default;
    SetTatamiCountAction(TournamentStore &tournament, size_t count);
    SetTatamiCountAction(const std::vector<TatamiLocation> &locations);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mLocations);
    }

private:
    std::vector<TatamiLocation> mLocations;

    // undo members
    size_t mOldCount;
    // std::stack<std::vector<std::tuple<CategoryId, MatchType, TatamiLocation>>> mOldContents;
};

CEREAL_REGISTER_TYPE(SetTatamiCountAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, SetTatamiCountAction)

// Erase Tatami
// Add Tatami
