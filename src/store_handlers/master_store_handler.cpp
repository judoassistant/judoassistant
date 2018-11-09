#include "store_handlers/master_store_handler.hpp"
#include "actions/player_actions.hpp"

MasterStoreHandler::MasterStoreHandler()
    : mTournament(std::unique_ptr<QTournamentStore>(new QTournamentStore))
    , mIsDirty(false)
    , mSyncing(false)
    , mServer(8000)
{
    connect(&mServer, &NetworkServer::actionReceived, this, &MasterStoreHandler::receiveAction);
    connect(&mServer, &NetworkServer::actionConfirmReceived, this, &MasterStoreHandler::receiveActionConfirm);
    connect(&mServer, &NetworkServer::syncConfirmed, this, &MasterStoreHandler::receiveSyncConfirm);

    mServer.start();
}

MasterStoreHandler::~MasterStoreHandler() {
    mServer.postQuit();
    mServer.quit();
    mServer.wait();
}

void MasterStoreHandler::dispatch(std::unique_ptr<Action> && action) {
    mIsDirty = true;

    std::shared_ptr<Action> sharedAction = std::move(action);
    auto actionId = mActionIdGenerator();
    mUnconfirmedStack.push_back({actionId, sharedAction});
    sharedAction->redo(*mTournament);
    // mServer.postAction(actionId, sharedAction);
}

QTournamentStore & MasterStoreHandler::getTournament() {
    return *mTournament;
}

const QTournamentStore & MasterStoreHandler::getTournament() const {
    return *mTournament;
}

void MasterStoreHandler::reset() {
    mSyncing = true;

    mTournament = std::make_unique<QTournamentStore>();
    mActionStack.clear();
    mUnconfirmedStack.clear();

    mServer.postSync(std::make_unique<TournamentStore>(*mTournament));

    emit tournamentReset();
    emit redoStatusChanged(false);
    emit undoStatusChanged(false);

    mIsDirty = false;
}

bool MasterStoreHandler::read(const QString &path) {
    mSyncing = true;

    log_debug().field("path", path).msg("Reading tournament from file");
    std::ifstream file(path.toStdString(), std::ios::in | std::ios::binary);

    if (!file.is_open())
        return false;

    mTournament = std::make_unique<QTournamentStore>();
    cereal::PortableBinaryInputArchive archive(file);
    archive(*mTournament);

    mServer.postSync(std::make_unique<TournamentStore>(*mTournament));

    mActionStack.clear();
    mUnconfirmedStack.clear();
    emit tournamentReset();
    emit redoStatusChanged(false);
    emit undoStatusChanged(false);
    mIsDirty = false;
    return true;
}

bool MasterStoreHandler::write(const QString &path) {
    log_debug().field("path", path).msg("Writing tournament to file");
    std::ofstream file(path.toStdString(), std::ios::out | std::ios::binary | std::ios::trunc);

    if (!file.is_open())
        return false;

    cereal::PortableBinaryOutputArchive archive(file);
    archive(*mTournament);
    mIsDirty = false;
    return true;
}

bool MasterStoreHandler::canUndo() {
    return false;
}

void MasterStoreHandler::undo() {
    throw std::runtime_error("Not implemented");
}

bool MasterStoreHandler::canRedo() {
    return false;
}

void MasterStoreHandler::redo() {
    throw std::runtime_error("Not implemented");
}

bool MasterStoreHandler::isDirty() const {
    return mIsDirty;
}

void MasterStoreHandler::receiveAction(ActionId actionId, std::shared_ptr<Action> action) {
    for (auto it = mUnconfirmedStack.rbegin(); it != mUnconfirmedStack.rend(); ++it)
        it->second->undo(*mTournament);

    action->redo(*mTournament);
    mActionStack.push_back({actionId, std::move(action)});

    for (auto it = mUnconfirmedStack.begin(); it != mUnconfirmedStack.end(); ++it)
        it->second->redo(*mTournament);
}

void MasterStoreHandler::receiveActionConfirm(ActionId actionId) {
    auto front = std::move(mUnconfirmedStack.front());
    mUnconfirmedStack.pop_front();

    if (front.first != actionId)
        throw std::runtime_error("Received confirmation in wrong order");

    mActionStack.push_back(std::move(front));
}

void MasterStoreHandler::receiveSyncConfirm() {
    mSyncing = false;
}

