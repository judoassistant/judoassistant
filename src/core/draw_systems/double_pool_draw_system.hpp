#pragma once

#include "core/draw_systems/draw_system.hpp"
#include "core/draw_systems/pool_draw_system.hpp"
#include "core/serialize.hpp"

class DoublePoolDrawSystem : public DrawSystem {
public:
    DoublePoolDrawSystem() {}
    DoublePoolDrawSystem(const DoublePoolDrawSystem&);
    virtual ~DoublePoolDrawSystem() {};

    std::unique_ptr<DrawSystem> clone() const override;
    std::string getName() const override;
    bool hasFinalBlock() const override;
    DrawSystemIdentifier getIdentifier() const override;

    std::vector<std::unique_ptr<AddMatchAction>> initCategory(const TournamentStore &tournament, const CategoryStore &category, const std::vector<PlayerId> &playerIds, unsigned int seed) override;
    std::vector<std::unique_ptr<Action>> updateCategory(const TournamentStore &tournament, const CategoryStore &category) const override;
    ResultList getResults(const TournamentStore &tournament, const CategoryStore &category) const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mMatches, mPlayers, mFirstPool, mSecondPool);
    }

private:
    std::vector<MatchId> mMatches;
    std::vector<PlayerId> mPlayers;

    std::unique_ptr<PoolDrawSystem> mFirstPool;
    std::unique_ptr<PoolDrawSystem> mSecondPool;
};

CEREAL_REGISTER_TYPE(DoublePoolDrawSystem)
CEREAL_REGISTER_POLYMORPHIC_RELATION(DrawSystem, DoublePoolDrawSystem)

