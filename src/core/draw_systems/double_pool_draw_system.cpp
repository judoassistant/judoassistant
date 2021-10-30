#include <queue>

#include "core/actions/add_match_action.hpp"
#include "core/actions/set_match_player_action.hpp"
#include "core/draw_systems/double_pool_draw_system.hpp"
#include "core/misc/merge_queue_element.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/shuffle.hpp"
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

    std::mt19937 randomEng(seed);
    shuffle(mPlayers.begin(), mPlayers.end(), randomEng);

    auto middle = std::next(mPlayers.begin(), mPlayers.size() / 2);
    std::vector<PlayerId> firstPoolPlayers(mPlayers.begin(), middle);
    std::vector<PlayerId> secondPoolPlayers(middle, mPlayers.end());

    mFirstPool = std::make_unique<PoolDrawSystem>(true);
    auto firstPoolActions = mFirstPool->initCategory(tournament, category, firstPoolPlayers, randomEng());

    mSecondPool = std::make_unique<PoolDrawSystem>(true);
    auto secondPoolActions = mSecondPool->initCategory(tournament, category, secondPoolPlayers, randomEng());

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
    auto firstSemiFinal = std::make_unique<AddMatchAction>(CombinedId(category.getId(), MatchId::generate(category, generator)), MatchType::FINAL, "Semi-Final", false, std::nullopt, std::nullopt);
    auto secondSemiFinal = std::make_unique<AddMatchAction>(CombinedId(category.getId(), MatchId::generate(category, generator)), MatchType::FINAL, "Semi-Final", false, std::nullopt, std::nullopt);
    auto finals = std::make_unique<AddMatchAction>(CombinedId(category.getId(), MatchId::generate(category, generator)), MatchType::FINAL, "Final", false, std::nullopt, std::nullopt);

    mMatches.push_back(firstSemiFinal->getMatchId());
    mMatches.push_back(secondSemiFinal->getMatchId());
    mMatches.push_back(finals->getMatchId());

    actions.push_back(std::move(firstSemiFinal));
    actions.push_back(std::move(secondSemiFinal));
    actions.push_back(std::move(finals)); // final is a reserved keyword. Hence the weird varname

    return actions;
}

std::vector<std::unique_ptr<Action>> DoublePoolDrawSystem::updateCategory(const TournamentStore &tournament, const CategoryStore &category) const {
    std::vector<std::unique_ptr<Action>> actions;

    if (mPlayers.size() < 4)
        return actions;

    auto &firstSemiFinal = category.getMatch(mMatches[mMatches.size() - 3]);
    auto &secondSemiFinal = category.getMatch(mMatches[mMatches.size() - 2]);
    auto &finals = category.getMatch(mMatches.back());

    const auto &eliminationFinished = category.getStatus(MatchType::ELIMINATION).isFinished();
    if (!eliminationFinished) {
        if (firstSemiFinal.getWhitePlayer())
            actions.push_back(std::make_unique<SetMatchPlayerAction>(firstSemiFinal.getCombinedId(), MatchStore::PlayerIndex::WHITE, std::nullopt));
        if (firstSemiFinal.getBluePlayer())
            actions.push_back(std::make_unique<SetMatchPlayerAction>(firstSemiFinal.getCombinedId(), MatchStore::PlayerIndex::BLUE, std::nullopt));
        if (secondSemiFinal.getWhitePlayer())
            actions.push_back(std::make_unique<SetMatchPlayerAction>(secondSemiFinal.getCombinedId(), MatchStore::PlayerIndex::WHITE, std::nullopt));
        if (secondSemiFinal.getBluePlayer())
            actions.push_back(std::make_unique<SetMatchPlayerAction>(secondSemiFinal.getCombinedId(), MatchStore::PlayerIndex::BLUE, std::nullopt));
        if (finals.getWhitePlayer())
            actions.push_back(std::make_unique<SetMatchPlayerAction>(finals.getCombinedId(), MatchStore::PlayerIndex::WHITE, std::nullopt));
        if (finals.getBluePlayer())
            actions.push_back(std::make_unique<SetMatchPlayerAction>(finals.getCombinedId(), MatchStore::PlayerIndex::BLUE, std::nullopt));
        return actions;
    }

    // Update match players for semi finals
    const auto firstPoolResults = mFirstPool->getResults(tournament, category);
    const auto secondPoolResults = mSecondPool->getResults(tournament, category);

    // Check first semi final players
    PlayerId firstSemiFinalWhitePlayer = firstPoolResults[0].first;
    PlayerId firstSemiFinalBluePlayer = secondPoolResults[1].first;
    if (tournament.getPlayer(firstSemiFinalWhitePlayer).getBlueJudogiHint()) // Try to satisfy blue judogi hints
        std::swap(firstSemiFinalWhitePlayer, firstSemiFinalBluePlayer);

    if (firstSemiFinal.getWhitePlayer() != firstSemiFinalWhitePlayer)
        actions.push_back(std::make_unique<SetMatchPlayerAction>(firstSemiFinal.getCombinedId(), MatchStore::PlayerIndex::WHITE, firstSemiFinalWhitePlayer));
    if (firstSemiFinal.getBluePlayer() != firstSemiFinalBluePlayer)
        actions.push_back(std::make_unique<SetMatchPlayerAction>(firstSemiFinal.getCombinedId(), MatchStore::PlayerIndex::BLUE, firstSemiFinalBluePlayer));

    // Check second semi final players
    PlayerId secondSemiFinalWhitePlayer = secondPoolResults[0].first;
    PlayerId secondSemiFinalBluePlayer = firstPoolResults[1].first;
    if (tournament.getPlayer(secondSemiFinalWhitePlayer).getBlueJudogiHint())
        std::swap(secondSemiFinalWhitePlayer, secondSemiFinalBluePlayer);

    if (secondSemiFinal.getWhitePlayer() != secondSemiFinalWhitePlayer)
        actions.push_back(std::make_unique<SetMatchPlayerAction>(secondSemiFinal.getCombinedId(), MatchStore::PlayerIndex::WHITE, secondSemiFinalWhitePlayer));
    if (secondSemiFinal.getBluePlayer() != secondSemiFinalBluePlayer)
        actions.push_back(std::make_unique<SetMatchPlayerAction>(secondSemiFinal.getCombinedId(), MatchStore::PlayerIndex::BLUE, secondSemiFinalBluePlayer));

    const bool semiFinalsFinished = (firstSemiFinal.getStatus() == MatchStatus::FINISHED && secondSemiFinal.getStatus() == MatchStatus::FINISHED);
    if (!semiFinalsFinished) {
        if (finals.getWhitePlayer())
            actions.push_back(std::make_unique<SetMatchPlayerAction>(finals.getCombinedId(), MatchStore::PlayerIndex::WHITE, std::nullopt));
        if (finals.getBluePlayer())
            actions.push_back(std::make_unique<SetMatchPlayerAction>(finals.getCombinedId(), MatchStore::PlayerIndex::BLUE, std::nullopt));
        return actions;
    }

    // Update match players for finals
    const auto &ruleset = category.getRuleset();

    PlayerId finalWhitePlayer = firstSemiFinal.getPlayer(ruleset.getWinner(firstSemiFinal).value()).value();
    PlayerId finalBluePlayer = secondSemiFinal.getPlayer(ruleset.getWinner(secondSemiFinal).value()).value();

    if (tournament.getPlayer(finalWhitePlayer).getBlueJudogiHint()) // Try to satisfy blue judogi hints
        std::swap(finalWhitePlayer, finalBluePlayer);

    if (finals.getWhitePlayer() != finalWhitePlayer)
        actions.push_back(std::make_unique<SetMatchPlayerAction>(finals.getCombinedId(), MatchStore::PlayerIndex::WHITE, finalWhitePlayer));

    if (finals.getBluePlayer() != finalBluePlayer)
        actions.push_back(std::make_unique<SetMatchPlayerAction>(finals.getCombinedId(), MatchStore::PlayerIndex::BLUE, finalBluePlayer));

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

    auto &finals = category.getMatch(mMatches.back());
    auto &firstSemiFinal = category.getMatch(mMatches[mMatches.size() - 3]);
    auto &secondSemiFinal = category.getMatch(mMatches[mMatches.size() - 2]);

    auto finalWinnerIndex = ruleset.getWinner(finals).value();
    auto finalLoserIndex = (finalWinnerIndex == MatchStore::PlayerIndex::WHITE) ? MatchStore::PlayerIndex::BLUE : MatchStore::PlayerIndex::WHITE;

    auto finalWinnerId = finals.getPlayer(finalWinnerIndex).value();
    auto finalLoserId = finals.getPlayer(finalLoserIndex).value();

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

DrawSystemIdentifier DoublePoolDrawSystem::getIdentifier() const {
    return DrawSystemIdentifier::DOUBLE_POOL;
}

