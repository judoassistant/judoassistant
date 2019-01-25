#include "core/draw_systems/pool_draw_system.hpp"
#include "core/actions/match_actions.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"

std::unique_ptr<DrawSystem> PoolDrawSystem::clone() const {
    return std::make_unique<PoolDrawSystem>(*this);
}

std::string PoolDrawSystem::getName() const {
    return "Pool";
}

std::vector<std::unique_ptr<Action>> PoolDrawSystem::initCategory(const std::vector<PlayerId> &playerIds, const TournamentStore &tournament, const CategoryStore &category, unsigned int seed) {
    mMatches.clear();
    mPlayers = playerIds;

    std::vector<std::unique_ptr<Action>> actions;
    MatchId::Generator generator(seed);

    if (mPlayers.size() <= 1)
        return actions;

    std::default_random_engine random_eng(seed);
    std::shuffle(mPlayers.begin(), mPlayers.end(), random_eng);

    // Algorithm described at https://stackoverflow.com/questions/6648512/scheduling-algorithm-for-a-round-robin-tournament
    std::vector<std::optional<PlayerId>> shiftedIds;
    if (mPlayers.size() % 2 != 0)
        shiftedIds.push_back(std::nullopt);
    for (size_t i = 0; i < mPlayers.size(); ++i)
        shiftedIds.emplace_back(mPlayers[i]);

    for (size_t round = 0; round < shiftedIds.size()-1; ++round) {
        for (size_t i = 0; i < shiftedIds.size()/2; ++i) {
            size_t j = shiftedIds.size() - i - 1;
            if (!shiftedIds[i] || !shiftedIds[j]) continue;
            auto action = std::make_unique<AddMatchAction>(MatchId::generate(category, generator), category.getId(), MatchType::KNOCKOUT, "Pool", false, shiftedIds[i], shiftedIds[j]);
            mMatches.push_back(action->getMatchId());
            actions.push_back(std::move(action));
        }

        // shift ids
        std::optional<PlayerId> temp = shiftedIds.back();
        for (size_t i = 1; i < shiftedIds.size(); ++i)
            std::swap(shiftedIds[i], temp);
    }

    return std::move(actions);
}

std::vector<std::unique_ptr<Action>> PoolDrawSystem::updateCategory(const TournamentStore & tournament, const CategoryStore & category) const {
    return {};
}

bool PoolDrawSystem::isFinished(const TournamentStore &tournament, const CategoryStore &category) const {
    for (auto matchId : mMatches) {
        const MatchStore & match = category.getMatch(matchId);
        if (match.getStatus() != MatchStatus::FINISHED)
            return false;
    }

    return true;
}

std::vector<std::pair<std::optional<unsigned int>, PlayerId>> PoolDrawSystem::getResults(const TournamentStore &tournament, const CategoryStore &category) const {
    // TODO: Implement
    std::vector<std::pair<std::optional<unsigned int>, PlayerId>> results;
    for (auto playerId : mPlayers)
        results.emplace_back(std::make_optional(0), playerId);
    return results;
}

bool PoolDrawSystem::hasFinalBlock() const {
    return false;
}
