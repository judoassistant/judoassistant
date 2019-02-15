#include "core/log.hpp"
#include "web/loaded_tournament.hpp"

LoadedTournament::LoadedTournament(const std::string &webName, boost::asio::io_context &context)
    : mContext(context)
    , mStrand(context)
    , mWebName(webName)
{

}

struct MoveWrapper {
    std::unique_ptr<TournamentStore> tournament;
    SharedActionList actionList;
};

void LoadedTournament::sync(std::unique_ptr<TournamentStore> tournament, SharedActionList actionList) {
    auto wrapper = std::make_shared<MoveWrapper>();
    wrapper->tournament = std::move(tournament);
    wrapper->actionList = std::move(actionList);

    boost::asio::post(mStrand, [this, wrapper](){
        mTournament = std::move(wrapper->tournament);
        mActionList = std::move(wrapper->actionList);
    });
}

void LoadedTournament::dispatch(ClientActionId actionId, std::shared_ptr<Action> action) {
    mStrand.post([this, actionId, action](){
        // TODO: Handle exceptions
        action->redo(*mTournament);
        mActionList.push_back({actionId, std::move(action)});
        mActionIds.insert(actionId);
    });
}

void LoadedTournament::undo(ClientActionId actionId) {
    mStrand.post([this, actionId](){
        auto idIt = mActionIds.find(actionId);
        if (idIt == mActionIds.end()) {
            log_warning().msg("Received invalid in tournament. Ignoring");
            return;
        }

        mActionIds.erase(idIt);

        auto it = std::prev(mActionList.end());
        while (it != mActionList.begin() && it->first != actionId) {
            it->second->undo(*mTournament);
            std::advance(it, -1);
        }

        if (it->first != actionId) {
            log_warning().msg("Did not find action when undoing.");
            std::advance(it, 1);
        }
        else {
            auto it2 = it;
            it = std::next(it2);

            it2->second->undo(*mTournament);
            mActionList.erase(it2);
        }

        while (it != mActionList.end()) {
            it->second->redo(*mTournament);
            std::advance(it, 1);
        }
        // TODO: Handle exceptions
    });
}

