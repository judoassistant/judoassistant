#include "store_handlers/master_store_handler.hpp"
#include "actions/create_player_action.hpp"

MasterStoreHandler::MasterStoreHandler()
    : mTournament(std::unique_ptr<QTournamentStore>(new QTournamentStore))
{
    dispatch(std::make_unique<CreatePlayerAction>(getTournament(), "Bob", "Johnson", 29));
    dispatch(std::make_unique<CreatePlayerAction>(getTournament(), "Alice", "Carpenter", 27));
}

bool MasterStoreHandler::dispatch(std::unique_ptr<Action> && action) {
    return (*action)(*mTournament);
}

QTournamentStore & MasterStoreHandler::getTournament() {
    return *mTournament;
}

void MasterStoreHandler::reset() {
    mTournament = std::make_unique<QTournamentStore>();
    emit tournamentReset();
}

bool MasterStoreHandler::read(const QString &path) {
    std::ifstream file(path.toStdString(), std::ios::in | std::ios::binary);

    if (!file.is_open())
        return false;

    mTournament = std::make_unique<QTournamentStore>();
    cereal::PortableBinaryInputArchive archive(file);
    archive(*mTournament);
    emit tournamentReset();
    return true;
}

bool MasterStoreHandler::write(const QString &path) {
    std::ofstream file(path.toStdString(), std::ios::out | std::ios::binary | std::ios::trunc);

    if (!file.is_open())
        return false;

    cereal::PortableBinaryOutputArchive archive(file);
    archive(*mTournament);
    return true;
}
