#pragma once

#include "core/draw_systems/draw_system.hpp"
#include "core/serialize.hpp"

class KnockoutDrawSystem : public DrawSystem {
public:
    KnockoutDrawSystem() {}
    KnockoutDrawSystem(const KnockoutDrawSystem &) = default;
    virtual ~KnockoutDrawSystem() {};

    std::unique_ptr<DrawSystem> clone() const override;
    std::string getName() const override;
    bool hasFinalBlock() const override;
    DrawSystemIdentifier getIdentifier() const override;

    std::vector<std::unique_ptr<AddMatchAction>> initCategory(const TournamentStore &tournament, const CategoryStore &category, const std::vector<PlayerId> &playerIds, unsigned int seed) override;
    std::vector<std::unique_ptr<Action>> updateCategory(const TournamentStore &tournament, const CategoryStore &category) const override;
    std::vector<std::pair<PlayerId, std::optional<unsigned int>>> getResults(const TournamentStore &tournament, const CategoryStore &category) const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mInitialRoundSize, mMatches, mPlayers);
    }

private:
    size_t mInitialRoundSize;
    std::vector<MatchId> mMatches;
    std::vector<PlayerId> mPlayers;
};

CEREAL_REGISTER_TYPE(KnockoutDrawSystem)
CEREAL_REGISTER_POLYMORPHIC_RELATION(DrawSystem, KnockoutDrawSystem)
