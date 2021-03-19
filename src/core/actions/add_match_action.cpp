#include "core/actions/add_match_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "core/stores/tournament_store.hpp"

AddMatchAction::AddMatchAction(const CombinedId &combinedId, MatchType type, const std::string &title, bool bye, std::optional<PlayerId> whitePlayerId, std::optional<PlayerId> bluePlayerId)
    : mCombinedId(combinedId)
    , mType(type)
    , mTitle(title)
    , mBye(bye)
    , mWhitePlayerId(whitePlayerId)
    , mBluePlayerId(bluePlayerId)
{}

void AddMatchAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCombinedId.getCategoryId())) return;

    CategoryStore & category = tournament.getCategory(mCombinedId.getCategoryId());
    if (category.containsMatch(mCombinedId.getMatchId()))
        throw ActionExecutionException("Failed to redo AddMatchAction. Match already exists.");

    std::optional<PlayerId> whitePlayerId;
    if (mWhitePlayerId && tournament.containsPlayer(*mWhitePlayerId)) {
        whitePlayerId = mWhitePlayerId;
        tournament.getPlayer(*whitePlayerId).addMatch(mCombinedId);
    }

    std::optional<PlayerId> bluePlayerId;
    if (mBluePlayerId && tournament.containsPlayer(*mBluePlayerId)) {
        bluePlayerId = mBluePlayerId;
        tournament.getPlayer(*bluePlayerId).addMatch(mCombinedId);
    }

    category.pushMatch(std::make_unique<MatchStore>(mCombinedId, mType, mTitle, mBye, whitePlayerId, bluePlayerId));
}

void AddMatchAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCombinedId.getCategoryId())) return;

    CategoryStore & category = tournament.getCategory(mCombinedId.getCategoryId());

    if (mWhitePlayerId && tournament.containsPlayer(*mWhitePlayerId))
        tournament.getPlayer(*mWhitePlayerId).eraseMatch(mCombinedId);

    if (mBluePlayerId && tournament.containsPlayer(*mBluePlayerId))
        tournament.getPlayer(*mBluePlayerId).eraseMatch(mCombinedId);

    category.popMatch();
}

MatchId AddMatchAction::getMatchId() {
    return mCombinedId.getMatchId();
}

std::unique_ptr<Action> AddMatchAction::freshClone() const {
    return std::make_unique<AddMatchAction>(mCombinedId, mType, mTitle, mBye, mWhitePlayerId, mBluePlayerId);
}

std::string AddMatchAction::getDescription() const {
    return "Add match";
}

