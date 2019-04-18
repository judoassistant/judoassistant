#include <queue>

#include "core/actions/match_actions.hpp"
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
    auto secondPoolActions = mFirstPool->initCategory(tournament, category, firstPoolPlayers, seedDist(randomEng));

    // Merge the list of actions
    std::priority_queue<MergeQueueElement> queue;
    auto firstIterator = firstPoolActions.begin();
    auto secondIterator = secondPoolActions.begin();

    queue.push(MergeQueueElement(0, 0, firstPoolActions.size()));
    queue.push(MergeQueueElement(1, 0, secondPoolActions.size()));

    while (!queue.empty()) {
        auto element = queue.top();
        queue.pop();

        if (element.index == 0)
            actions.push_back(std::move(*(firstIterator++)));
        else
            actions.push_back(std::move(*(secondIterator++)));

        ++(element.matchCount);
        if (element.matchCount == element.totalMatchCount) continue;
        queue.push(element);
    }

    // Add the semi finals and finals
    actions.push_back(std::make_unique<AddMatchAction>(MatchId::generate(category, generator), category.getId(), MatchType::FINAL, "Semi-Final", false, std::nullopt, std::nullopt));
    actions.push_back(std::make_unique<AddMatchAction>(MatchId::generate(category, generator), category.getId(), MatchType::FINAL, "Semi-Final", false, std::nullopt, std::nullopt));
    actions.push_back(std::make_unique<AddMatchAction>(MatchId::generate(category, generator), category.getId(), MatchType::FINAL, "Final", false, std::nullopt, std::nullopt));

    return std::move(actions);
}

std::vector<std::unique_ptr<Action>> DoublePoolDrawSystem::updateCategory(const TournamentStore &tournament, const CategoryStore &category) const {
    return {};
}

std::vector<std::pair<std::optional<unsigned int>, PlayerId>> DoublePoolDrawSystem::getResults(const TournamentStore &tournament, const CategoryStore &category) const {
    return {};
}

bool DoublePoolDrawSystem::hasFinalBlock() const {
    return true;
}

