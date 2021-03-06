#include <set>

#include "core/actions/add_players_to_category_action.hpp"
#include "core/actions/auto_add_categories_action.hpp"
#include "core/actions/draw_categories_action.hpp"
#include "core/actions/erase_categories_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/preferences_store.hpp"
#include "core/rulesets/ruleset.hpp"

std::string AutoAddCategoriesAction::getDescription() const {
    return "Automatically add categories for players";
}

struct Solution {
    bool possible;
    std::multiset<size_t> sizes;
    size_t next;

    bool operator<(const Solution &other) {
        if (possible != other.possible)
            return possible; // smaller iff possible

        auto i = sizes.begin();
        auto j = other.sizes.begin();

        while (i != sizes.end() && j != other.sizes.end()) {
            if (*i != *j)
                return *i > *j;

            ++j;
            ++i;
        }

        return false;
    }
};

AutoAddCategoriesAction::AutoAddCategoriesAction(TournamentStore &tournament, std::vector<PlayerId> playerIds, std::string baseName, float maxDifference, size_t maxSize)
    : mBaseName(baseName)
    , mSeed(getSeed())
{
    std::vector<std::pair<float, PlayerId>> weights;
    for (auto playerId : playerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;
        const PlayerStore &player = tournament.getPlayer(playerId);
        if (!player.getWeight())
            continue;
        weights.emplace_back(player.getWeight()->toFloat(), playerId);
    }

    if (weights.empty())
        return;

    std::sort(weights.begin(), weights.end());

    std::vector<Solution> dp(weights.size()+1);

    auto & back = dp.back();
    back.possible = true;

    for (int i = weights.size() - 1; i >= 0; --i) {
        Solution best_so_far;
        best_so_far.possible = false;

        for (size_t j = static_cast<size_t>(i); j < weights.size(); ++j) {
            auto diff = 100.0 * (weights[j].first - weights[i].first)/weights[j].first;
            auto size = j-static_cast<size_t>(i)+1;

            if (size > maxSize)
                break;
            if (diff > maxDifference)
                break;

            const auto & next = dp[j+1];

            Solution current;
            current.possible = next.possible;
            if (current.possible) { // no reason to copy if not possible
                current.sizes = next.sizes;
                current.sizes.insert(size);
                if (current.sizes.size() > 50) // Limit to 50 to avoid n^3 complexity
                    current.sizes.erase(std::prev(current.sizes.end()));
            }
            current.next = j+1;

            if (current < best_so_far)
                best_so_far = current;
        }

        dp[i] = best_so_far;
    }

    if (!dp[0].possible) return;

    size_t next = 0;
    while (next != weights.size()) {
        auto sol = dp[next];
        std::vector<PlayerId> playerIds;
        for (size_t j = next; j < sol.next; ++j)
            playerIds.push_back(weights[j].second);

        mCategoryIds.push_back(CategoryId::generate(tournament));
        mPlayerIds.push_back(std::move(playerIds));

        next = sol.next;
    }
}

AutoAddCategoriesAction::AutoAddCategoriesAction(const std::vector<std::vector<PlayerId>> &playerIds, std::vector<CategoryId> categoryIds, std::string baseName, unsigned int seed)
    : mPlayerIds(playerIds)
    , mCategoryIds(categoryIds)
    , mBaseName(baseName)
    , mSeed(seed)
{}

std::unique_ptr<Action> AutoAddCategoriesAction::freshClone() const {
    return std::make_unique<AutoAddCategoriesAction>(mPlayerIds, mCategoryIds, mBaseName, mSeed);
}

void AutoAddCategoriesAction::redoImpl(TournamentStore & tournament) {
    for (auto categoryId : mCategoryIds) {
        if (tournament.containsCategory(categoryId))
            throw ActionExecutionException("Failed to redo AutoAddCategoriesAction. Category already exists.");
    }

    const auto &preferences = tournament.getPreferences();
    tournament.beginAddCategories(mCategoryIds);
    for (size_t i = 0; i < mCategoryIds.size(); ++i) {
        std::string name = mBaseName + " " + std::to_string(i+1);
        auto ruleset = Ruleset::getDefaultRuleset();
        auto drawSystem = DrawSystem::getDrawSystem(preferences.getPreferredDrawSystem(mPlayerIds[i].size()));
        tournament.addCategory(std::make_unique<CategoryStore>(mCategoryIds[i], name, std::move(ruleset), std::move(drawSystem)));
    }
    tournament.endAddCategories(mCategoryIds);

    for (size_t i = 0; i < mCategoryIds.size(); ++i) {
        AddPlayersToCategoryAction playersAction(mCategoryIds[i], mPlayerIds[i], mSeed);
        playersAction.redo(tournament);
    }
}

void AutoAddCategoriesAction::undoImpl(TournamentStore & tournament) {
    EraseCategoriesAction eraseAction(mCategoryIds);
    eraseAction.redo(tournament);
}

