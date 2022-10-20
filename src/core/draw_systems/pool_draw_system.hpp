#pragma once

#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/serialize.hpp"

class PoolDrawSystem : public DrawSystem {
public:
    PoolDrawSystem(bool composited = false);
    PoolDrawSystem(const PoolDrawSystem &) = default;
    virtual ~PoolDrawSystem() {};

    std::unique_ptr<DrawSystem> clone() const override;
    std::string getName() const override;
    bool hasFinalBlock() const override;
    DrawSystemIdentifier getIdentifier() const override;

    std::vector<std::unique_ptr<AddMatchAction>> initCategory(const TournamentStore &tournament, const CategoryStore &category, const std::vector<PlayerId> &playerIds, unsigned int seed) override;
    std::vector<std::unique_ptr<Action>> updateCategory(const TournamentStore &tournament, const CategoryStore &category) const override;

    ResultList getResults(const TournamentStore &tournament, const CategoryStore &category) const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mMatches, mPlayers, mComposited);
    }

protected:
    std::unique_ptr<AddMatchAction> createMatch(const TournamentStore &tournament, const CategoryStore &category, PlayerId firstPlayer, PlayerId secondPlayer, MatchId::Generator &generator);
    std::vector<std::pair<PlayerId, PlayerId>> createMatchOrderForEvenNumber(const std::vector<PlayerId> &playerIds);
    std::vector<std::pair<PlayerId, PlayerId>> createMatchOrderForOddNumber(const std::vector<PlayerId> &playerIds);

private:
    void orderByWinsWithinGroup(const CategoryStore &category, ResultList &results, size_t begin, size_t end) const;
    void orderByRemainingCriteria(const TournamentStore &tournament, const CategoryStore &category, ResultList &results, size_t begin, size_t end) const;

    std::vector<MatchId> mMatches;
    std::vector<PlayerId> mPlayers;
    bool mComposited;
};

CEREAL_REGISTER_TYPE(PoolDrawSystem)
CEREAL_REGISTER_POLYMORPHIC_RELATION(DrawSystem, PoolDrawSystem)
