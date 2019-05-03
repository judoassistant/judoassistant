#include <queue>

#include "core/actions/add_match_action.hpp"
#include "core/actions/set_match_player_action.hpp"
#include "core/draw_systems/double_pool_draw_system.hpp"
#include "core/misc/merge_queue_element.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "core/stores/tournament_store.hpp"

DoublePoolDrawSystem::DoublePoolDrawSystem(const DoublePoolDrawSystem &other)
    : mMatches(other.mMatches)
    , mPlayers(other.mPlayers)
{
    if (other.mFirstPool)
        mFirstPool = std::make_unique<PoolDrawSystem>(*(other.mFirstPool));
    if (other.mSecondPool)
        mSecondPool = std::make_unique<PoolDrawSystem>(*(other.mSecondPool));
}

std::unique_ptr<DrawSystem> DoublePoolDrawSystem::clone() const {
    return std::make_unique<DoublePoolDrawSystem>(*this);
}

std::string DoublePoolDrawSystem::getName() const {
    return "Double Pool";
}

std::vector<std::unique_ptr<AddMatchAction>> DoublePoolDrawSystem::initCategory(const TournamentStore &tournament, const CategoryStore &category, const std::vector<PlayerId> &playerIds, unsigned int seed) {
    assert(playerIds.size() == category.getPlayers().size()); // This draw system is not made to be composed

    mMatches.clear();
    mPlayers = playerIds;

    std::vector<std::unique_ptr<AddMatchAction>> actions;
    MatchId::Generator generator(seed);

    if (mPlayers.size() < 4)
        return actions;

    std::default_random_engine randomEng(seed);
    std::shuffle(mPlayers.begin(), mPlayers.end(), randomEng);

    std::uniform_int_distribution<unsigned int> seedDist;

    auto middle = std::next(mPlayers.begin(), mPlayers.size() / 2);
    std::vector<PlayerId> firstPoolPlayers(mPlayers.begin(), middle);
    std::vector<PlayerId> secondPoolPlayers(middle, mPlayers.end());

    mFirstPool = std::make_unique<PoolDrawSystem>();
    auto firstPoolActions = mFirstPool->initCategory(tournament, category, firstPoolPlayers, seedDist(randomEng));

    mSecondPool = std::make_unique<PoolDrawSystem>();
    auto secondPoolActions = mSecondPool->initCategory(tournament, category, secondPoolPlayers, seedDist(randomEng));

    // Merge the list of actions
    std::priority_queue<MergeQueueElement> queue;
    auto firstIterator = firstPoolActions.begin();
    auto secondIterator = secondPoolActions.begin();

    queue.push(MergeQueueElement(0, 0, firstPoolActions.size()));
    queue.push(MergeQueueElement(1, 0, secondPoolActions.size()));

    while (!queue.empty()) {
        auto element = queue.top();
        queue.pop();

        if (element.index == 0) {
            auto action = std::move(*(firstIterator++));
            mMatches.push_back(action->getMatchId());
            actions.push_back(std::move(action));
        }
        else {
            auto action = std::move(*(secondIterator++));
            mMatches.push_back(action->getMatchId());
            actions.push_back(std::move(action));
        }

        ++(element.matchCount);
        if (element.matchCount == element.totalMatchCount) continue;
        queue.push(element);
    }

    // Add the semi finals and finals
    auto firstSemiFinal = std::make_unique<AddMatchAction>(MatchId::generate(category, generator), category.getId(), MatchType::FINAL, "Semi-Final", false, std::nullopt, std::nullopt);
    auto secondSemiFinal = std::make_unique<AddMatchAction>(MatchId::generate(category, generator), category.getId(), MatchType::FINAL, "Semi-Final", false, std::nullopt, std::nullopt);
    auto finaly = std::make_unique<AddMatchAction>(MatchId::generate(category, generator), category.getId(), MatchType::FINAL, "Final", false, std::nullopt, std::nullopt);

    mMatches.push_back(firstSemiFinal->getMatchId());
    mMatches.push_back(secondSemiFinal->getMatchId());
    mMatches.push_back(finaly->getMatchId());

    actions.push_back(std::move(firstSemiFinal));
    actions.push_back(std::move(secondSemiFinal));
    actions.push_back(std::move(finaly)); // final is a reserved keyword. Hence the weird varname

    return std::move(actions);
}

std::vector<std::unique_ptr<Action>> DoublePoolDrawSystem::updateCategory(const TournamentStore &tournament, const CategoryStore &category) const {
    std::vector<std::unique_ptr<Action>> actions;

    if (mPlayers.size() < 4)
        return actions;

    auto &firstSemiFinal = category.getMatch(mMatches[mMatches.size() - 3]);
    auto &secondSemiFinal = category.getMatch(mMatches[mMatches.size() - 2]);
    auto &finaly = category.getMatch(mMatches.back());

    const auto &status = category.getStatus(MatchType::ELIMINATION);
    if (status.isFinished()) { // elimination finished
        auto firstPoolResults = mFirstPool->getResults(tournament, category);
        auto secondPoolResults = mSecondPool->getResults(tournament, category);

        // Check first semi final players
        if (firstSemiFinal.getWhitePlayer() != firstPoolResults[0].first)
            actions.push_back(std::make_unique<SetMatchPlayerAction>(category.getId(), firstSemiFinal.getId(), MatchStore::PlayerIndex::WHITE, firstPoolResults[0].first));
        if (firstSemiFinal.getBluePlayer() != secondPoolResults[1].first)
            actions.push_back(std::make_unique<SetMatchPlayerAction>(category.getId(), firstSemiFinal.getId(), MatchStore::PlayerIndex::BLUE, secondPoolResults[1].first));

        // Check second semi final players
        if (secondSemiFinal.getWhitePlayer() != secondPoolResults[0].first)
            actions.push_back(std::make_unique<SetMatchPlayerAction>(category.getId(), secondSemiFinal.getId(), MatchStore::PlayerIndex::WHITE, secondPoolResults[0].first));
        if (secondSemiFinal.getBluePlayer() != firstPoolResults[1].first)
            actions.push_back(std::make_unique<SetMatchPlayerAction>(category.getId(), secondSemiFinal.getId(), MatchStore::PlayerIndex::BLUE, firstPoolResults[1].first));

        // Check final players
        const auto &ruleset = category.getRuleset();

        // Check white player
        std::optional<PlayerId> whitePlayer = std::nullopt;
        if (firstSemiFinal.getStatus() == MatchStatus::FINISHED)
            whitePlayer = firstSemiFinal.getPlayer(ruleset.getWinner(firstSemiFinal).value());

        if (whitePlayer != finaly.getWhitePlayer())
            actions.push_back(std::make_unique<SetMatchPlayerAction>(category.getId(), finaly.getId(), MatchStore::PlayerIndex::WHITE, whitePlayer));

        // Check blue player
        std::optional<PlayerId> bluePlayer = std::nullopt;
        if (secondSemiFinal.getStatus() == MatchStatus::FINISHED)
            bluePlayer = secondSemiFinal.getPlayer(ruleset.getWinner(secondSemiFinal).value());

        if (bluePlayer != finaly.getBluePlayer())
            actions.push_back(std::make_unique<SetMatchPlayerAction>(category.getId(), finaly.getId(), MatchStore::PlayerIndex::BLUE, bluePlayer));

        return actions;
    }

    // Elimination not finished. Make sure semi-finals and finals have no players
    if (firstSemiFinal.getWhitePlayer())
        actions.push_back(std::make_unique<SetMatchPlayerAction>(category.getId(), firstSemiFinal.getId(), MatchStore::PlayerIndex::WHITE, std::nullopt));
    if (firstSemiFinal.getBluePlayer())
        actions.push_back(std::make_unique<SetMatchPlayerAction>(category.getId(), firstSemiFinal.getId(), MatchStore::PlayerIndex::BLUE, std::nullopt));
    if (secondSemiFinal.getWhitePlayer())
        actions.push_back(std::make_unique<SetMatchPlayerAction>(category.getId(), secondSemiFinal.getId(), MatchStore::PlayerIndex::WHITE, std::nullopt));
    if (secondSemiFinal.getBluePlayer())
        actions.push_back(std::make_unique<SetMatchPlayerAction>(category.getId(), secondSemiFinal.getId(), MatchStore::PlayerIndex::BLUE, std::nullopt));
    if (finaly.getWhitePlayer())
        actions.push_back(std::make_unique<SetMatchPlayerAction>(category.getId(), finaly.getId(), MatchStore::PlayerIndex::WHITE, std::nullopt));
    if (finaly.getBluePlayer())
        actions.push_back(std::make_unique<SetMatchPlayerAction>(category.getId(), finaly.getId(), MatchStore::PlayerIndex::BLUE, std::nullopt));
    return actions;
}

std::vector<std::pair<PlayerId, std::optional<unsigned int>>> DoublePoolDrawSystem::getResults(const TournamentStore &tournament, const CategoryStore &category) const {
    std::vector<std::pair<PlayerId, std::optional<unsigned int>>> results;

    if (mPlayers.size() < 4)
        return results;

    const auto &status = category.getStatus(MatchType::ELIMINATION) + category.getStatus(MatchType::FINAL);
    if (!status.isFinished()) // not finished
        return results;

    const auto &ruleset = category.getRuleset();

    auto &finaly = category.getMatch(mMatches.back());
    auto &firstSemiFinal = category.getMatch(mMatches[mMatches.size() - 3]);
    auto &secondSemiFinal = category.getMatch(mMatches[mMatches.size() - 2]);

    auto finalWinnerIndex = ruleset.getWinner(finaly).value();
    auto finalLoserIndex = (finalWinnerIndex == MatchStore::PlayerIndex::WHITE) ? MatchStore::PlayerIndex::BLUE : MatchStore::PlayerIndex::WHITE;

    auto finalWinnerId = finaly.getPlayer(finalWinnerIndex).value();
    auto finalLoserId = finaly.getPlayer(finalLoserIndex).value();

    results.emplace_back(finalWinnerId, 1);
    results.emplace_back(finalLoserId, 2);

    auto firstSemiFinalLoserIndex = (ruleset.getWinner(firstSemiFinal).value() == MatchStore::PlayerIndex::WHITE) ? MatchStore::PlayerIndex::BLUE : MatchStore::PlayerIndex::WHITE;
    auto secondSemiFinalLoserIndex = (ruleset.getWinner(secondSemiFinal).value() == MatchStore::PlayerIndex::WHITE) ? MatchStore::PlayerIndex::BLUE : MatchStore::PlayerIndex::WHITE;

    auto firstSemiFinalLoserId = firstSemiFinal.getPlayer(firstSemiFinalLoserIndex).value();
    auto secondSemiFinalLoserId = secondSemiFinal.getPlayer(secondSemiFinalLoserIndex).value();

    results.emplace_back(firstSemiFinalLoserId, 3);
    results.emplace_back(secondSemiFinalLoserId, 3);

    std::unordered_set<PlayerId> finalists;
    finalists.insert(finalWinnerId);
    finalists.insert(finalLoserId);
    finalists.insert(firstSemiFinalLoserId);
    finalists.insert(secondSemiFinalLoserId);

    for (auto playerId : mPlayers) {
        if (finalists.find(playerId) != finalists.end())
            continue;
        results.emplace_back(playerId, std::nullopt);
    }

    return results;
}

bool DoublePoolDrawSystem::hasFinalBlock() const {
    return true;
}

