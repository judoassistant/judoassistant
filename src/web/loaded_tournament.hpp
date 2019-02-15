#pragma once

#include <boost/asio.hpp> // TODO: Do not include boost convenience headers

#include "core/actions/action.hpp"
#include "core/stores/tournament_store.hpp"

class LoadedTournament {
public:
    LoadedTournament(const std::string &webName, boost::asio::io_context &context);

    void sync(std::unique_ptr<TournamentStore> tournament, SharedActionList actionList);
    void action(ClientActionId actionId, std::unique_ptr<Action> action);
    void undo(ClientActionId actionId);

private:
    boost::asio::io_context &mContext;
    boost::asio::io_context::strand mStrand;

    std::string mWebName;
    std::unique_ptr<TournamentStore> mTournament;
    SharedActionList mActionList;
};
