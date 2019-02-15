#include "web/loaded_tournament.hpp"

LoadedTournament::LoadedTournament(const std::string &webName, boost::asio::io_context &context)
    : mContext(context)
    , mStrand(context)
    , mWebName(webName)
{

}

void LoadedTournament::sync(std::unique_ptr<TournamentStore> tournament, SharedActionList actionList) {
    mStrand.post([this](){});
}

void LoadedTournament::dispatch(ClientActionId actionId, std::shared_ptr<Action> action) {
    mStrand.post([this](){});
}

void LoadedTournament::undo(ClientActionId actionId) {
    mStrand.post([this](){});
}

