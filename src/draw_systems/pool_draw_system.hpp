#pragma once

#include "draw_systems/draw_system.hpp"
#include "serialize.hpp"

class PoolDrawSystem : public DrawSystem {
public:
    PoolDrawSystem() {}
    PoolDrawSystem(const PoolDrawSystem &) = default;
    virtual ~PoolDrawSystem() {};

    std::unique_ptr<DrawSystem> clone() const override;
    std::string getName() const override;
    bool hasFinalBlock() const override;

    std::vector<std::unique_ptr<Action>> initCategory(const std::vector<PlayerId> &playerIds, TournamentStore & tournament, CategoryStore & category) override;
    std::vector<std::unique_ptr<Action>> updateCategory(TournamentStore & tournament, CategoryStore & category) override;
    bool isFinished(TournamentStore & tournament, CategoryStore & category) const override;
    std::vector<std::pair<size_t, PlayerId>> get_results() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::make_nvp("matchIds", mMatchIds));
    }

private:
    std::vector<MatchId> mMatchIds;
};

CEREAL_REGISTER_TYPE(PoolDrawSystem)
CEREAL_REGISTER_POLYMORPHIC_RELATION(DrawSystem, PoolDrawSystem)
