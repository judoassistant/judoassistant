#include "core/actions/match_actions.hpp"
#include "core/draw_systems/best_of_three_draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "core/stores/tournament_store.hpp"

std::unique_ptr<DrawSystem> BestOfThreeDrawSystem::clone() const {
    return std::make_unique<BestOfThreeDrawSystem>(*this);
}

std::string BestOfThreeDrawSystem::getName() const {
    return "Best-of-Three";
}

std::vector<std::unique_ptr<AddMatchAction>> BestOfThreeDrawSystem::initCategory(const TournamentStore &tournament, const CategoryStore &category, const std::vector<PlayerId> &playerIds, unsigned int seed) {
    assert(playerIds.size() == category.getPlayers().size()); // This draw system is not made to be composed
    mMatches.clear();
    mPlayers = playerIds;

    std::vector<std::unique_ptr<AddMatchAction>> actions;
    MatchId::Generator generator(seed);

    if (mPlayers.size() != 2)
        return actions;

    std::default_random_engine randomEng(seed);
    std::uniform_int_distribution dist(0, 1);

    size_t whitePlayer = dist(randomEng);
    size_t bluePlayer = 1 - whitePlayer;

    // Algorithm described at https://stackoverflow.com/questions/6648512/scheduling-algorithm-for-a-round-robin-tournament
    std::vector<std::optional<PlayerId>> shiftedIds;
    if (mPlayers.size() % 2 != 0)
        shiftedIds.push_back(std::nullopt);
    for (size_t i = 0; i < mPlayers.size(); ++i)
        shiftedIds.emplace_back(mPlayers[i]);

    for (size_t i = 0; i < 3; ++i) {
        auto action = std::make_unique<AddMatchAction>(MatchId::generate(category, generator), category.getId(), MatchType::ELIMINATION, "Best-of-Three", false, playerIds[whitePlayer], playerIds[bluePlayer]);
        mMatches.push_back(action->getMatchId());
        actions.push_back(std::move(action));
    }

    return std::move(actions);
}

std::vector<std::unique_ptr<Action>> BestOfThreeDrawSystem::updateCategory(const TournamentStore &tournament, const CategoryStore &category) const {
    std::vector<std::unique_ptr<Action>> actions;

    if (mPlayers.size() != 2)
        return actions;

    const auto &firstMatch = category.getMatch(mMatches[0]);
    const auto &secondMatch = category.getMatch(mMatches[1]);

    bool bye = false;
    if (firstMatch.getStatus() == MatchStatus::FINISHED && secondMatch.getStatus() == MatchStatus::FINISHED) {
        const auto &ruleset = category.getRuleset();
        auto firstWinner = ruleset.getWinner(firstMatch);
        auto secondWinner = ruleset.getWinner(secondMatch);

        if (firstWinner.has_value() && secondWinner.has_value() && firstWinner == secondWinner)
            bye = true;
    }

    const auto &thirdMatch = category.getMatch(mMatches[2]);

    if (thirdMatch.isBye() != bye)
        actions.push_back(std::make_unique<SetMatchByeAction>(category.getId(), mMatches[2], bye));
    return actions;
}

std::vector<std::pair<PlayerId, std::optional<unsigned int>>> BestOfThreeDrawSystem::getResults(const TournamentStore &tournament, const CategoryStore &category) const {
    std::vector<std::pair<PlayerId, std::optional<unsigned int>>> results;

    if (mPlayers.size() != 2)
        return results;

    auto status = category.getStatus(MatchType::ELIMINATION);
    if (!status.isFinished()) // not finished
        return results;

    const auto &ruleset = category.getRuleset();

    unsigned int whiteWinCount = 0;

    // get winner of first match
    const auto &firstMatch = category.getMatch(mMatches[0]);
    assert(firstMatch.getStatus() == MatchStatus::FINISHED);
    if (ruleset.getWinner(firstMatch) == MatchStore::PlayerIndex::WHITE)
        ++whiteWinCount;

    // get winner of second match
    const auto &secondMatch = category.getMatch(mMatches[1]);
    if (ruleset.getWinner(secondMatch) == MatchStore::PlayerIndex::WHITE)
        ++whiteWinCount;

    // get winner of third match (if neccesary)
    if (whiteWinCount == 1) {
        const auto &thirdMatch = category.getMatch(mMatches[2]);
        if (ruleset.getWinner(thirdMatch) == MatchStore::PlayerIndex::WHITE)
            ++whiteWinCount;
    }

    // construct results list
    if (whiteWinCount > 1) {
        results.emplace_back(firstMatch.getWhitePlayer().value(), 1);
        results.emplace_back(firstMatch.getBluePlayer().value(), 2);
    }
    else {
        results.emplace_back(firstMatch.getBluePlayer().value(), 1);
        results.emplace_back(firstMatch.getWhitePlayer().value(), 2);
    }

    return results;
}

bool BestOfThreeDrawSystem::hasFinalBlock() const {
    return false;
}
