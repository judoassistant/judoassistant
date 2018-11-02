#include "store_handlers/master_store_handler.hpp"
#include "actions/player_actions.hpp"

MasterStoreHandler::MasterStoreHandler()
    : mTournament(std::unique_ptr<QTournamentStore>(new QTournamentStore))
    , mIsDirty(false)
{
    // TODO: Remove this after testing
    mTournament->getTatamis().pushTatami();
    mTournament->getTatamis().pushTatami();
    mTournament->getTatamis().pushTatami();
    mTournament->getTatamis().pushTatami();
}

void MasterStoreHandler::dispatch(std::unique_ptr<Action> && action) {
    mIsDirty = true;
    log_debug().msg("Dispatching action");
    mActionStack.push_back(std::move(action));
    mActionStack.back()->redo(*mTournament);
    if (mActionStack.size() == 1) // mActionStack was empty before pushing the action
        emit undoStatusChanged(true);
    if (!mRedoStack.empty()) {
        mRedoStack.clear();
        emit redoStatusChanged(false);
    }
}

QTournamentStore & MasterStoreHandler::getTournament() {
    return *mTournament;
}

const QTournamentStore & MasterStoreHandler::getTournament() const {
    return *mTournament;
}

void MasterStoreHandler::reset() {
    mTournament = std::make_unique<QTournamentStore>();
    mActionStack.clear();
    mRedoStack.clear();
    emit tournamentReset();
    emit redoStatusChanged(false);
    emit undoStatusChanged(false);
    mIsDirty = false;
}

bool MasterStoreHandler::read(const QString &path) {
    log_debug().field("path", path).msg("Reading tournament from file");
    std::ifstream file(path.toStdString(), std::ios::in | std::ios::binary);

    if (!file.is_open())
        return false;

    mTournament = std::make_unique<QTournamentStore>();
    cereal::PortableBinaryInputArchive archive(file);
    archive(*mTournament);
    mActionStack.clear();
    mRedoStack.clear();
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
    return !mActionStack.empty();
}

void MasterStoreHandler::undo() {
    log_debug().msg("Undoing last action");
    std::unique_ptr<Action> action = std::move(mActionStack.back());
    mActionStack.pop_back();

    action->undo(*mTournament);
    mRedoStack.push_back(std::move(action));
    mIsDirty = true;

    if (mRedoStack.size() == 1) // mRedoStack was empty before pushing the action
        emit redoStatusChanged(true);
    if (mActionStack.empty())
        emit undoStatusChanged(false);
}

bool MasterStoreHandler::canRedo() {
    return !mRedoStack.empty();
}

void MasterStoreHandler::redo() {
    log_debug().msg("Redoing action");
    std::unique_ptr<Action> action = std::move(mRedoStack.back());
    mRedoStack.pop_back();

    action->redo(*mTournament);
    mActionStack.push_back(std::move(action));
    mIsDirty = true;

    if (mRedoStack.empty())
        emit redoStatusChanged(false);
    if (mActionStack.size() == 1)
        emit undoStatusChanged(true); // mActionStack was empty before pushing the action
}

bool MasterStoreHandler::isDirty() const {
    return mIsDirty;
}

