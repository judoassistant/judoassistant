#include "web/loaded_tournament.hpp"

LoadedTournament::LoadedTournament(const std::string &webName, boost::asio::io_context &context)
    : mContext(context)
    , mStrand(context)
    , mWebName(webName)
{

}

void LoadedTournament::sync(std::unique_ptr<TournamentStore> tournament, SharedActionList actionList) {

}
