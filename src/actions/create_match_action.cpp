#include "actions/create_match_action.hpp"

CreateMatchAction::CreateMatchAction(TournamentStore * tournament, CategoryStore * category, std::optional<Id> whitePlayer, std::optional<Id> bluePlayer)
    : mId(tournament->generateNextMatchId())
    , mCategory(category->getId())
    , mWhitePlayer(whitePlayer)
    , mBluePlayer(bluePlayer)
{}

bool CreateMatchAction::operator()(TournamentStore * tournament) const {
    std::unique_ptr<CategoryStore> & category = tournament->getCategory(mCategory);
    category->addMatch(std::make_unique<MatchStore>(mId, mWhitePlayer, mBluePlayer));
    return true;
}

std::unique_ptr<Action> CreateMatchAction::getInverse() const {
    // TODO
}

Id CreateMatchAction::getId() const {
    return mId;
}
