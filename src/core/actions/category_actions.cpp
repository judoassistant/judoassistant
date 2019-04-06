#include <sstream>
#include <set>

#include "core/actions/category_actions.hpp"
#include "core/actions/match_actions.hpp"
#include "core/draw_systems/draw_systems.hpp"
#include "core/exception.hpp"
#include "core/log.hpp"
#include "core/random.hpp"
#include "core/rulesets/rulesets.hpp"
#include "core/stores/match_store.hpp"
#include "core/stores/tatami/location.hpp"
#include "core/stores/tatami/tatami_list.hpp"
#include "core/stores/tournament_store.hpp"

AddCategoryAction::AddCategoryAction(CategoryId id, const std::string &name, size_t ruleset, size_t drawSystem)
    : mId(id)
    , mName(name)
    , mRuleset(ruleset)
    , mDrawSystem(drawSystem)
{}

AddCategoryAction::AddCategoryAction(TournamentStore & tournament, const std::string &name, size_t ruleset, size_t drawSystem)
    : AddCategoryAction(CategoryId::generate(tournament), name, ruleset, drawSystem)
{}

std::unique_ptr<Action> AddCategoryAction::freshClone() const {
    return std::make_unique<AddCategoryAction>(mId, mName, mRuleset, mDrawSystem);
}

CategoryId AddCategoryAction::getId() const {
    return mId;
}

void AddCategoryAction::redoImpl(TournamentStore & tournament) {
    if (tournament.containsCategory(mId))
        throw ActionExecutionException("Failed to redo AddCategoryAction. Category already exists.");

    const auto &rulesets = Rulesets::getRulesets();
    if (mRuleset > rulesets.size())
        throw ActionExecutionException("Failed to redo AddCategoryAction. Invalid ruleset specified.");
    auto ruleset = rulesets[mRuleset]->clone();

    const auto &drawSystems = DrawSystems::getDrawSystems();
    if (mDrawSystem > drawSystems.size())
        throw ActionExecutionException("Failed to redo AddCategoryAction. Invalid drawSystem specified.");
    auto drawSystem = drawSystems[mDrawSystem]->clone();

    tournament.beginAddCategories({mId});
    tournament.addCategory(std::make_unique<CategoryStore>(mId, mName, std::move(ruleset), std::move(drawSystem)));
    tournament.endAddCategories();
}

void AddCategoryAction::undoImpl(TournamentStore & tournament) {
    tournament.beginEraseCategories({mId});
    tournament.eraseCategory(mId);
    tournament.endEraseCategories();
}

AddPlayersToCategoryAction::AddPlayersToCategoryAction(CategoryId categoryId, const std::vector<PlayerId> &playerIds)
    : AddPlayersToCategoryAction(categoryId, playerIds, getSeed())
{}

AddPlayersToCategoryAction::AddPlayersToCategoryAction(CategoryId categoryId, const std::vector<PlayerId> &playerIds, unsigned int seed)
    : mCategoryId(categoryId)
    , mPlayerIds(playerIds)
    , mSeed(seed)
{}

std::unique_ptr<Action> AddPlayersToCategoryAction::freshClone() const {
    return std::make_unique<AddPlayersToCategoryAction>(mCategoryId, mPlayerIds, mSeed);
}

void AddPlayersToCategoryAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;
    CategoryStore & category = tournament.getCategory(mCategoryId);

    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId)) continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        if (player.containsCategory(mCategoryId))
            continue;

        mAddedPlayerIds.push_back(playerId);
        player.addCategory(mCategoryId);
        category.addPlayer(playerId);
    }

    tournament.addPlayersToCategory(mCategoryId, mAddedPlayerIds);

    if (!mAddedPlayerIds.empty()) {
        mDrawAction = std::make_unique<DrawCategoryAction>(mCategoryId, mSeed);
        mDrawAction->redo(tournament);
    }
}

void AddPlayersToCategoryAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;

    tournament.erasePlayersFromCategory(mCategoryId, mAddedPlayerIds);

    CategoryStore & category = tournament.getCategory(mCategoryId);
    for (auto playerId : mAddedPlayerIds) {
        PlayerStore & player = tournament.getPlayer(playerId);
        player.eraseCategory(mCategoryId);
        category.erasePlayer(playerId);
    }

    if (mDrawAction != nullptr) {
        mDrawAction->undo(tournament);
        mDrawAction.reset();
    }

    mAddedPlayerIds.clear();
}

ErasePlayersFromCategoryAction::ErasePlayersFromCategoryAction(CategoryId categoryId, const std::vector<PlayerId> &playerIds)
    : ErasePlayersFromCategoryAction(categoryId, playerIds, getSeed())
{}

ErasePlayersFromCategoryAction::ErasePlayersFromCategoryAction(CategoryId categoryId, const std::vector<PlayerId> &playerIds, unsigned int seed)
    : mCategoryId(categoryId)
    , mPlayerIds(playerIds)
    , mSeed(seed)
{}

std::unique_ptr<Action> ErasePlayersFromCategoryAction::freshClone() const {
    return std::make_unique<ErasePlayersFromCategoryAction>(mCategoryId, mPlayerIds, mSeed);
}

void ErasePlayersFromCategoryAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;

    CategoryStore & category = tournament.getCategory(mCategoryId);

    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId)) continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        if (!player.containsCategory(mCategoryId))
            continue;

        mErasedPlayerIds.push_back(playerId);
        player.eraseCategory(mCategoryId);
        category.erasePlayer(playerId);
    }

    tournament.erasePlayersFromCategory(mCategoryId, mErasedPlayerIds);

    if (!mErasedPlayerIds.empty()) {
        mDrawAction = std::make_unique<DrawCategoryAction>(mCategoryId, mSeed);
        mDrawAction->redo(tournament);
    }
}

void ErasePlayersFromCategoryAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;

    CategoryStore & category = tournament.getCategory(mCategoryId);

    tournament.addPlayersToCategory(mCategoryId, mErasedPlayerIds);

    for (auto playerId : mErasedPlayerIds) {
        PlayerStore & player = tournament.getPlayer(playerId);
        player.addCategory(mCategoryId);
        category.addPlayer(playerId);
    }

    if (mDrawAction != nullptr) {
        mDrawAction->undo(tournament);
        mDrawAction.reset();
    }

    mErasedPlayerIds.clear();
}

EraseCategoriesAction::EraseCategoriesAction(const std::vector<CategoryId> &categoryIds)
    : mCategoryIds(categoryIds)
{}

std::unique_ptr<Action> EraseCategoriesAction::freshClone() const {
    return std::make_unique<EraseCategoriesAction>(mCategoryIds);
}

struct BlockLocationIndexComp {
    typedef std::tuple<CategoryId, MatchType, BlockLocation> Type;
    bool operator()(const Type &a, const Type &b) const {
        const auto &aLocation = std::get<2>(a);
        const auto &bLocation = std::get<2>(b);

        if (aLocation.sequentialGroup.concurrentGroup.tatami.handle.index != bLocation.sequentialGroup.concurrentGroup.tatami.handle.index)
            return aLocation.sequentialGroup.concurrentGroup.tatami.handle.index < bLocation.sequentialGroup.concurrentGroup.tatami.handle.index;
        if (aLocation.sequentialGroup.concurrentGroup.handle.index != bLocation.sequentialGroup.concurrentGroup.handle.index)
            return aLocation.sequentialGroup.concurrentGroup.handle.index < bLocation.sequentialGroup.concurrentGroup.handle.index;
        if (aLocation.sequentialGroup.handle.index != bLocation.sequentialGroup.handle.index)
            return aLocation.sequentialGroup.handle.index < bLocation.sequentialGroup.handle.index;
        return aLocation.pos < bLocation.pos;
    }
};

void EraseCategoriesAction::redoImpl(TournamentStore & tournament) {
    for (auto categoryId : mCategoryIds) {
        if (!tournament.containsCategory(categoryId))
            continue;

        mErasedCategoryIds.push_back(categoryId);
    }

    // Find all locations and keep them in increasing order
    std::set<std::tuple<CategoryId, MatchType, BlockLocation>, BlockLocationIndexComp> locations;

    tournament.beginEraseCategories(mErasedCategoryIds);
    const auto &tatamis = tournament.getTatamis();

    for (auto categoryId : mErasedCategoryIds) {
        const CategoryStore & category = tournament.getCategory(categoryId);

        for (auto type : {MatchType::KNOCKOUT, MatchType::FINAL}) {
            if (category.getLocation(type)) {
                auto block = std::make_pair(categoryId, type);
                auto location = tatamis.refreshLocation(*category.getLocation(type), block);
                locations.insert({categoryId, type, location});
            }
        }
    }

    // move categories away from the tatami in increasing order
    for (const auto &tuple : locations) {
        auto block = std::make_pair(std::get<0>(tuple), std::get<1>(tuple));
        auto location = std::get<2>(tuple);
        tournament.getTatamis().moveBlock(tournament, block, location, std::nullopt);
    }

    // erase the categories
    for (auto categoryId : mErasedCategoryIds) {
        CategoryStore & category = tournament.getCategory(categoryId);

        for (auto playerId : category.getPlayers()) {
            PlayerStore & player = tournament.getPlayer(playerId);
            player.eraseCategory(categoryId);
        }

        for (const std::unique_ptr<MatchStore> &match : category.getMatches()) {
            auto whitePlayerId = match->getPlayer(MatchStore::PlayerIndex::WHITE);
            if (whitePlayerId)
                tournament.getPlayer(*whitePlayerId).eraseMatch(categoryId, match->getId());

            auto bluePlayerId = match->getPlayer(MatchStore::PlayerIndex::BLUE);
            if (bluePlayerId)
                tournament.getPlayer(*bluePlayerId).eraseMatch(categoryId, match->getId());
        }

        mCategories.push(tournament.eraseCategory(categoryId));
    }

    tournament.endEraseCategories();

    for (auto tuple : locations) {
        mBlocks.push_back({std::get<0>(tuple), std::get<1>(tuple)});
        mLocations.push_back(std::get<2>(tuple));
    }

    if (!locations.empty())
        tournament.changeTatamis(mLocations, mBlocks);
}

void EraseCategoriesAction::undoImpl(TournamentStore & tournament) {
    tournament.beginAddCategories(mErasedCategoryIds);

    // Update players that belong to the category
    while (!mCategories.empty()) {
        std::unique_ptr<CategoryStore> category = std::move(mCategories.top());

        for (auto playerId : category->getPlayers()) {
            PlayerStore & player = tournament.getPlayer(playerId);
            player.addCategory(category->getId());
        }

        for (const std::unique_ptr<MatchStore> &match : category->getMatches()) {
            auto whitePlayerId = match->getPlayer(MatchStore::PlayerIndex::WHITE);
            if (whitePlayerId)
                tournament.getPlayer(*whitePlayerId).addMatch(category->getId(), match->getId());

            auto bluePlayerId = match->getPlayer(MatchStore::PlayerIndex::BLUE);
            if (bluePlayerId)
                tournament.getPlayer(*bluePlayerId).addMatch(category->getId(), match->getId());
        }

        tournament.addCategory(std::move(category));
        mCategories.pop();
    }

    // Update tatamis in non-decreasing order
    // TODO: Consider adding a feature to support making multiple moves at once
    for (size_t i = 0; i < mBlocks.size(); ++i)
        tournament.getTatamis().moveBlock(tournament, mBlocks[i], std::nullopt, mLocations[i]);

    tournament.endAddCategories();
    if (!mLocations.empty())
        tournament.changeTatamis(mLocations, mBlocks);

    mErasedCategoryIds.clear();
    mBlocks.clear();
    mLocations.clear();
}

DrawCategoryAction::DrawCategoryAction(CategoryId categoryId)
    : DrawCategoryAction(categoryId, getSeed())
{}

DrawCategoryAction::DrawCategoryAction(CategoryId categoryId, unsigned int seed)
    : mCategoryId(categoryId)
    , mSeed(seed)
{}

std::unique_ptr<Action> DrawCategoryAction::freshClone() const {
    return std::make_unique<DrawCategoryAction>(mCategoryId, mSeed);
}

void DrawCategoryAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;

    // Delete all existing matches
    tournament.beginResetMatches(mCategoryId);
    CategoryStore & category = tournament.getCategory(mCategoryId);

    for (const std::unique_ptr<MatchStore> &match : category.getMatches()) {
        std::optional<PlayerId> whitePlayer = match->getPlayer(MatchStore::PlayerIndex::WHITE);
        if (whitePlayer && tournament.containsPlayer(*whitePlayer))
            tournament.getPlayer(*whitePlayer).eraseMatch(mCategoryId, match->getId());

        std::optional<PlayerId> bluePlayer = match->getPlayer(MatchStore::PlayerIndex::BLUE);
        if (bluePlayer && tournament.containsPlayer(*bluePlayer))
            tournament.getPlayer(*bluePlayer).eraseMatch(mCategoryId, match->getId());
    }

    mOldMatches = std::move(category.clearMatches());
    mOldDrawSystem = category.getDrawSystem().clone();
    mOldStatus[static_cast<size_t>(MatchType::KNOCKOUT)] = category.getStatus(MatchType::KNOCKOUT);
    mOldStatus[static_cast<size_t>(MatchType::FINAL)] = category.getStatus(MatchType::FINAL);

    std::vector<PlayerId> playerIds(category.getPlayers().begin(), category.getPlayers().end());
    std::vector<std::unique_ptr<Action>> actions = category.getDrawSystem().initCategory(tournament, category, playerIds, mSeed);

    for (size_t i = 0; i < actions.size(); ++i) {
        actions[i]->redo(tournament);
        mActions.push(std::move(actions[i]));
    }

    CategoryStatus knockoutStatus;
    knockoutStatus.notStartedMatches = category.getMatchCount(MatchType::KNOCKOUT);
    category.setStatus(MatchType::KNOCKOUT, knockoutStatus);

    CategoryStatus finalStatus;
    finalStatus.notStartedMatches = category.getMatchCount(MatchType::FINAL);
    category.setStatus(MatchType::FINAL, finalStatus);

    tournament.endResetMatches(mCategoryId);

    std::vector<BlockLocation> changedLocations;
    std::vector<std::pair<CategoryId, MatchType>> changedBlocks;

    for (MatchType type : {MatchType::FINAL, MatchType::KNOCKOUT}) {
        std::optional<BlockLocation> location = category.getLocation(type);
        if (location) {
            tournament.getTatamis().recomputeBlock(tournament, *location);
            changedLocations.push_back(*location);
            changedBlocks.push_back({mCategoryId, type});
        }
    }

    if (!changedLocations.empty())
        tournament.changeTatamis(changedLocations, changedBlocks);
}

void DrawCategoryAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;

    tournament.beginResetMatches(mCategoryId);
    CategoryStore & category = tournament.getCategory(mCategoryId);

    while (!mActions.empty()) {
        mActions.top()->undo(tournament);
        mActions.pop();
    }

    category.setDrawSystem(std::move(mOldDrawSystem));
    category.setStatus(MatchType::KNOCKOUT, mOldStatus[static_cast<size_t>(MatchType::KNOCKOUT)]);
    category.setStatus(MatchType::FINAL, mOldStatus[static_cast<size_t>(MatchType::FINAL)]);

    for (std::unique_ptr<MatchStore> & match : mOldMatches) {
        std::optional<PlayerId> whitePlayer = match->getPlayer(MatchStore::PlayerIndex::WHITE);
        if (whitePlayer && tournament.containsPlayer(*whitePlayer))
            tournament.getPlayer(*whitePlayer).eraseMatch(mCategoryId, match->getId());

        std::optional<PlayerId> bluePlayer = match->getPlayer(MatchStore::PlayerIndex::BLUE);
        if (bluePlayer && tournament.containsPlayer(*bluePlayer))
            tournament.getPlayer(*bluePlayer).eraseMatch(mCategoryId, match->getId());

        category.pushMatch(std::move(match));
    }

    mOldMatches.clear();

    tournament.endResetMatches(mCategoryId);

    std::vector<BlockLocation> changedLocations;
    std::vector<std::pair<CategoryId, MatchType>> changedBlocks;

    for (MatchType type : {MatchType::FINAL, MatchType::KNOCKOUT}) {
        std::optional<BlockLocation> location = category.getLocation(type);
        if (location) {
            tournament.getTatamis().recomputeBlock(tournament, *location);
            changedLocations.push_back(*location);
            changedBlocks.push_back({mCategoryId, type});
        }
    }

    if (!changedLocations.empty())
        tournament.changeTatamis(changedLocations, changedBlocks);
}

ErasePlayersFromAllCategoriesAction::ErasePlayersFromAllCategoriesAction(const std::vector<PlayerId> &playerIds)
    : ErasePlayersFromAllCategoriesAction(playerIds, getSeed())
{}

ErasePlayersFromAllCategoriesAction::ErasePlayersFromAllCategoriesAction(const std::vector<PlayerId> &playerIds, unsigned int seed)
    : mPlayerIds(playerIds)
    , mSeed(seed)
{}

std::unique_ptr<Action> ErasePlayersFromAllCategoriesAction::freshClone() const {
    return std::make_unique<ErasePlayersFromAllCategoriesAction>(mPlayerIds, mSeed);
}

void ErasePlayersFromAllCategoriesAction::redoImpl(TournamentStore & tournament) {
    std::vector<PlayerId> playerIds;
    std::unordered_set<CategoryId> categoryIds;

    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId)) continue;

        playerIds.push_back(playerId);
        const PlayerStore &player = tournament.getPlayer(playerId);
        const auto &playerCategories = player.getCategories();
        categoryIds.insert(playerCategories.begin(), playerCategories.end());
    }

    for (auto categoryId : categoryIds) {
        auto action = std::make_unique<ErasePlayersFromCategoryAction>(categoryId, playerIds, mSeed);
        action->redo(tournament);
        mActions.push(std::move(action));
    }
}

void ErasePlayersFromAllCategoriesAction::undoImpl(TournamentStore & tournament) {
    while (!mActions.empty()) {
        mActions.top()->undo(tournament);
        mActions.pop();
    }
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
        weights.push_back({player.getWeight()->toFloat(), playerId});
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

    const auto &rulesets = Rulesets::getRulesets();
    const auto &drawSystems = DrawSystems::getDrawSystems();
    tournament.beginAddCategories(mCategoryIds);
    for (size_t i = 0; i < mCategoryIds.size(); ++i) {
        // TODO: Use different drawsystems for different size groups
        std::stringstream ss;
        ss << mBaseName << " " << i+1;
        auto ruleset = rulesets[0]->clone();
        auto drawSystem = drawSystems[0]->clone();
        tournament.addCategory(std::make_unique<CategoryStore>(mCategoryIds[i], ss.str(), std::move(ruleset), std::move(drawSystem)));
    }
    tournament.endAddCategories();

    for (size_t i = 0; i < mCategoryIds.size(); ++i) {
        AddPlayersToCategoryAction playersAction(mCategoryIds[i], mPlayerIds[i], mSeed);
        playersAction.redo(tournament);
    }
}

void AutoAddCategoriesAction::undoImpl(TournamentStore & tournament) {
    EraseCategoriesAction eraseAction(mCategoryIds);
    eraseAction.redo(tournament);
}

ChangeCategoriesNameAction::ChangeCategoriesNameAction(std::vector<CategoryId> categoryIds, const std::string &value)
    : mCategoryIds(categoryIds)
    , mValue(value)
{}

std::unique_ptr<Action> ChangeCategoriesNameAction::freshClone() const {
    return std::make_unique<ChangeCategoriesNameAction>(mCategoryIds, mValue);
}

void ChangeCategoriesNameAction::redoImpl(TournamentStore & tournament) {
    for (auto categoryId : mCategoryIds) {
        if (!tournament.containsCategory(categoryId))
            continue;

        CategoryStore & category = tournament.getCategory(categoryId);
        mOldValues.push_back(category.getName());
        category.setName(mValue);
    }
    tournament.changeCategories(mCategoryIds);
}

void ChangeCategoriesNameAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto categoryId : mCategoryIds) {
        if (!tournament.containsCategory(categoryId))
            continue;
        assert(i != mOldValues.end());

        CategoryStore & category = tournament.getCategory(categoryId);

        category.setName(*i);

        std::advance(i, 1);
    }

    tournament.changeCategories(mCategoryIds);
    mOldValues.clear();
}

ChangeCategoriesRulesetAction::ChangeCategoriesRulesetAction(std::vector<CategoryId> categoryIds, size_t ruleset)
    : ChangeCategoriesRulesetAction(categoryIds, ruleset, getSeed())
{}

ChangeCategoriesRulesetAction::ChangeCategoriesRulesetAction(std::vector<CategoryId> categoryIds, size_t ruleset, unsigned int seed)
    : mCategoryIds(categoryIds)
    , mRuleset(ruleset)
    , mSeed(seed)
{}

std::unique_ptr<Action> ChangeCategoriesRulesetAction::freshClone() const {
    return std::make_unique<ChangeCategoriesRulesetAction>(mCategoryIds, mRuleset, mSeed);
}

void ChangeCategoriesRulesetAction::redoImpl(TournamentStore & tournament) {
    const auto &rulesets = Rulesets::getRulesets();
    if (mRuleset > rulesets.size())
        throw ActionExecutionException("Failed to redo ChangeCategoriesRulesetAction. Invalid ruleset specified.");
    const auto &ruleset = rulesets[mRuleset];

    for (auto categoryId : mCategoryIds) {
        if (!tournament.containsCategory(categoryId))
            continue;

        CategoryStore & category = tournament.getCategory(categoryId);

        mOldRulesets.push_back(category.setRuleset(ruleset->clone()));

        auto drawAction = std::make_unique<DrawCategoryAction>(categoryId, mSeed);
        drawAction->redo(tournament);
        mDrawActions.push_back(std::move(drawAction));
    }

    tournament.changeCategories(mCategoryIds);
}

void ChangeCategoriesRulesetAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldRulesets.rbegin();
    auto j = mDrawActions.rbegin();
    for (auto k = mCategoryIds.rbegin(); k != mCategoryIds.rend(); ++k) {
        auto categoryId = *k;

        if (!tournament.containsCategory(categoryId))
            continue;

        assert(i != mOldRulesets.rend());
        assert(j != mDrawActions.rend());

        CategoryStore & category = tournament.getCategory(categoryId);

        category.setRuleset(std::move(*i));
        (*j)->undo(tournament);

        std::advance(i, 1);
        std::advance(j, 1);
    }

    mOldRulesets.clear();
    mDrawActions.clear();
    tournament.changeCategories(mCategoryIds);
}

ChangeCategoriesDrawSystemAction::ChangeCategoriesDrawSystemAction(std::vector<CategoryId> categoryIds, size_t drawSystem)
    : ChangeCategoriesDrawSystemAction(categoryIds, drawSystem, getSeed())
{}

ChangeCategoriesDrawSystemAction::ChangeCategoriesDrawSystemAction(std::vector<CategoryId> categoryIds, size_t drawSystem, unsigned int seed)
    : mCategoryIds(categoryIds)
    , mDrawSystem(drawSystem)
    , mSeed(seed)
{}

std::unique_ptr<Action> ChangeCategoriesDrawSystemAction::freshClone() const {
    return std::make_unique<ChangeCategoriesDrawSystemAction>(mCategoryIds, mDrawSystem, mSeed);
}

void ChangeCategoriesDrawSystemAction::redoImpl(TournamentStore & tournament) {
    const auto &drawSystems = DrawSystems::getDrawSystems();
    if (mDrawSystem > drawSystems.size())
        throw ActionExecutionException("Failed to redo ChangeCategoriesDrawSystemAction. Invalid drawSystem specified.");
    const auto &drawSystem = drawSystems[mDrawSystem];

    for (auto categoryId : mCategoryIds) {
        if (!tournament.containsCategory(categoryId))
            continue;

        CategoryStore & category = tournament.getCategory(categoryId);

        mOldDrawSystems.push_back(category.setDrawSystem(drawSystem->clone()));

        auto drawAction = std::make_unique<DrawCategoryAction>(categoryId, mSeed);
        drawAction->redo(tournament);
        mDrawActions.push_back(std::move(drawAction));
    }

    tournament.changeCategories(mCategoryIds);
}

void ChangeCategoriesDrawSystemAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldDrawSystems.rbegin();
    auto j = mDrawActions.rbegin();
    for (auto k = mCategoryIds.rbegin(); k != mCategoryIds.rend(); ++k) {
        auto categoryId = *k;

        if (!tournament.containsCategory(categoryId))
            continue;

        assert(i != mOldDrawSystems.rend());
        assert(j != mDrawActions.rend());

        CategoryStore & category = tournament.getCategory(categoryId);

        (*j)->undo(tournament);
        category.setDrawSystem(std::move(*i));

        std::advance(i, 1);
        std::advance(j, 1);
    }

    mOldDrawSystems.clear();
    mDrawActions.clear();
    tournament.changeCategories(mCategoryIds);
}

std::string AddCategoryAction::getDescription() const {
    return "Add category";
}

std::string DrawCategoryAction::getDescription() const {
    return "Draw category";
}

std::string AddPlayersToCategoryAction::getDescription() const {
    return "Add players to category";
}

std::string ErasePlayersFromCategoryAction::getDescription() const {
    return "Erase players from category";
}

std::string EraseCategoriesAction::getDescription() const {
    return "Erase players from category";
}

std::string ErasePlayersFromAllCategoriesAction::getDescription() const {
    return "Erase players from all categories";
}

std::string AutoAddCategoriesAction::getDescription() const {
    return "Automatically add categories for players";
}

std::string ChangeCategoriesNameAction::getDescription() const {
    return "Change categories name";
}

std::string ChangeCategoriesRulesetAction::getDescription() const {
    return "Change categories ruleset";
}

std::string ChangeCategoriesDrawSystemAction::getDescription() const {
    return "Change categories draw system";
}

AddCategoryWithPlayersAction::AddCategoryWithPlayersAction(CategoryId id, const std::string &name, size_t ruleset, size_t drawSystem, const std::vector<PlayerId> &playerIds, unsigned int seed)
    : mId(id)
    , mName(name)
    , mRuleset(ruleset)
    , mDrawSystem(drawSystem)
    , mPlayerIds(playerIds)
    , mSeed(seed)
{}

AddCategoryWithPlayersAction::AddCategoryWithPlayersAction(TournamentStore & tournament, const std::string &name, size_t ruleset, size_t drawSystem, const std::vector<PlayerId> &playerIds)
    : AddCategoryWithPlayersAction(CategoryId::generate(tournament), name, ruleset, drawSystem, playerIds, getSeed())
{}


std::unique_ptr<Action> AddCategoryWithPlayersAction::freshClone() const {
    return std::make_unique<AddCategoryWithPlayersAction>(mId, mName, mRuleset, mDrawSystem, mPlayerIds, mSeed);
}

void AddCategoryWithPlayersAction::redoImpl(TournamentStore & tournament) {
    mCategoryAction = std::make_unique<AddCategoryAction>(mId, mName, mRuleset, mDrawSystem);
    mCategoryAction->redo(tournament);

    if (!mPlayerIds.empty()) {
        mPlayersAction = std::make_unique<AddPlayersToCategoryAction>(mId, mPlayerIds, mSeed);
        mPlayersAction->redo(tournament);
    }
}

void AddCategoryWithPlayersAction::undoImpl(TournamentStore & tournament) {
    if (!mPlayerIds.empty()) {
        mPlayersAction->undo(tournament);
        mPlayersAction.reset();
    }

    mCategoryAction->undo(tournament);
    mCategoryAction.reset();
}

std::string AddCategoryWithPlayersAction::getDescription() const {
    if (mPlayerIds.empty())
        return "Add category";
    return "Add category with players";
}

