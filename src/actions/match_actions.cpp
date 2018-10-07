#include "actions/match_actions.hpp"
#include "exception.hpp"

AddMatchAction::AddMatchAction(TournamentStore & tournament, Id categoryId, std::optional<Id> whitePlayerId, std::optional<Id> bluePlayerId)
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

    tournament.beginAddMatches({mId});
    std::optional<Id> whitePlayerId;
    if (mWhitePlayerId && tournament.containsPlayer(*mWhitePlayerId)) {
        whitePlayerId = mWhitePlayerId;
        tournament.getPlayer(*whitePlayerId).addMatch(mId);
    }

    std::optional<Id> bluePlayerId;
    if (mBluePlayerId && tournament.containsPlayer(*mBluePlayerId)) {
        bluePlayerId = mBluePlayerId;
        tournament.getPlayer(*bluePlayerId).addMatch(mId);
    }

    category.addMatch(std::make_unique<MatchStore>(mId, mCategoryId, whitePlayerId, bluePlayerId));
    tournament.endAddMatches();
}

void AddMatchAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;

    CategoryStore & category = tournament.getCategory(mCategoryId);

    tournament.beginEraseMatches({mId});

    if (mWhitePlayerId && tournament.containsPlayer(*mWhitePlayerId))
        tournament.getPlayer(*mWhitePlayerId).eraseMatch(mId);

    if (mBluePlayerId && tournament.containsPlayer(*mBluePlayerId))
        tournament.getPlayer(*mBluePlayerId).eraseMatch(mId);

    category.eraseMatch(mId);
    tournament.endEraseMatches();
}

