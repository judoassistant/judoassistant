#pragma once

#include "core/draw_systems/draw_system.hpp"
#include "core/serialize.hpp"

class PoolDrawSystem : public DrawSystem {
public:
    PoolDrawSystem() {}
    PoolDrawSystem(const PoolDrawSystem &) = default;
    virtual ~PoolDrawSystem() {};

    std::unique_ptr<DrawSystem> clone() const override;
    std::string getName() const override;
    bool hasFinalBlock() const override;
    DrawSystemIdentifier getIdentifier() const override;

    std::vector<std::unique_ptr<AddMatchAction>> initCategory(const TournamentStore &tournament, const CategoryStore &category, const std::vector<PlayerId> &playerIds, unsigned int seed) override;
    std::vector<std::unique_ptr<Action>> updateCategory(const TournamentStore &tournament, const CategoryStore &category) const override;
    std::vector<std::pair<PlayerId, std::optional<unsigned int>>> getResults(const TournamentStore &tournament, const CategoryStore &category) const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mMatches, mPlayers);
    }

private:
    std::vector<MatchId> mMatches;
    std::vector<PlayerId> mPlayers;
};

CEREAL_REGISTER_TYPE(PoolDrawSystem)
CEREAL_REGISTER_POLYMORPHIC_RELATION(DrawSystem, PoolDrawSystem)
