#include "actions/match_actions.hpp"
#include "exception.hpp"

AddMatchAction::AddMatchAction(TournamentStore & tournament, CategoryId categoryId, std::optional<PlayerId> whitePlayerId, std::optional<PlayerId> bluePlayerId)
    : mId(tournament.generateNextMatchId())
    , mCategoryId(categoryId)
    , mWhitePlayerId(whitePlayerId)
    , mBluePlayerId(bluePlayerId)
{}

void AddMatchAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;

    CategoryStore & category = tournament.getCategory(mCategoryId);
    if (category.containsMatch(mId))
        throw ActionExecutionException("Failed to redo AddMatchAction. Match already exists.");

    tournament.beginAddMatches(mCategoryId, {mId});
    std::optional<PlayerId> whitePlayerId;
    if (mWhitePlayerId && tournament.containsPlayer(*mWhitePlayerId)) {
        whitePlayerId = mWhitePlayerId;
        tournament.getPlayer(*whitePlayerId).addMatch(mCategoryId, mId);
    }

    std::optional<PlayerId> bluePlayerId;
    if (mBluePlayerId && tournament.containsPlayer(*mBluePlayerId)) {
        bluePlayerId = mBluePlayerId;
        tournament.getPlayer(*bluePlayerId).addMatch(mCategoryId, mId);
    }

    category.addMatch(std::make_unique<MatchStore>(mId, mCategoryId, whitePlayerId, bluePlayerId));
    tournament.endAddMatches();
}

void AddMatchAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;

    CategoryStore & category = tournament.getCategory(mCategoryId);

    tournament.beginEraseMatches(mCategoryId, {mId});

    if (mWhitePlayerId && tournament.containsPlayer(*mWhitePlayerId))
        tournament.getPlayer(*mWhitePlayerId).eraseMatch(mCategoryId, mId);

    if (mBluePlayerId && tournament.containsPlayer(*mBluePlayerId))
        tournament.getPlayer(*mBluePlayerId).eraseMatch(mCategoryId, mId);

    category.eraseMatch(mId);
    tournament.endEraseMatches();
}

MatchId AddMatchAction::getMatchId() {
    return mId;
}
