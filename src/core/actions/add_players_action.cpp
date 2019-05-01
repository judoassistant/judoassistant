#include "core/actions/add_players_action.hpp"
#include "core/stores/tournament_store.hpp"

AddPlayersAction::AddPlayersAction(TournamentStore & tournament, const std::vector<PlayerFields> &fields)
    : mFields(fields)
{
    for (size_t i = 0; i < fields.size(); ++i)
        mIds.push_back(PlayerId::generate(tournament));
}

AddPlayersAction::AddPlayersAction(const std::vector<PlayerId> &ids, const std::vector<PlayerFields> &fields)
    : mIds(ids)
    , mFields(fields)
{
    assert(mIds.size() == mFields.size());
}

void AddPlayersAction::redoImpl(TournamentStore & tournament) {
    for (auto id : mIds) {
        if (tournament.containsPlayer(id))
            throw ActionExecutionException("Failed to redo AddPlayersAction. Player already exists."); // TODO: Make sure to catch these exceptions
    }

    tournament.beginAddPlayers(mIds);
    auto i = mIds.begin();
    auto j = mFields.begin();
    for (; i != mIds.end() && j != mFields.end(); ++i, ++j)
        tournament.addPlayer(std::make_unique<PlayerStore>(*i, *j));

    tournament.endAddPlayers(mIds);
}

void AddPlayersAction::undoImpl(TournamentStore & tournament) {
    tournament.beginErasePlayers(mIds);
    for (auto id : mIds)
        tournament.erasePlayer(id);
    tournament.endErasePlayers(mIds);
}

std::unique_ptr<Action> AddPlayersAction::freshClone() const {
    return std::make_unique<AddPlayersAction>(mIds, mFields);
}

std::string AddPlayersAction::getDescription() const {
    return "Add players";
}

