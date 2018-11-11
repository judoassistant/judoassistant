#include "store_managers/store_manager.hpp"

StoreManager::StoreManager()
    : mTournament(std::make_unique<QTournamentStore>())
    , mSyncing(0)
{
    qRegisterMetaType<ActionId>();
}

StoreManager::~StoreManager() {
    if (mNetworkInterface)
        stopInterface();
}

void StoreManager::startInterface(std::unique_ptr<NetworkInterface> interface) {
    if (mNetworkInterface != nullptr)
        throw std::runtime_error("Attempted to start StoreManager interface with another one running");

    mNetworkInterface = std::move(interface);

    connect(mNetworkInterface.get(), &NetworkInterface::actionReceived, this, &StoreManager::receiveAction);
    connect(mNetworkInterface.get(), &NetworkInterface::actionConfirmReceived, this, &StoreManager::receiveActionConfirm);

    connect(mNetworkInterface.get(), &NetworkInterface::undoReceived, this, &StoreManager::receiveUndo);
    connect(mNetworkInterface.get(), &NetworkInterface::undoConfirmReceived, this, &StoreManager::receiveUndoConfirm);

    connect(mNetworkInterface.get(), &NetworkInterface::syncConfirmed, this, &StoreManager::receiveSyncConfirm);

    mNetworkInterface->start();
}

void StoreManager::stopInterface() {
    mNetworkInterface->quit();
    mNetworkInterface->wait();
    mNetworkInterface.reset();
}

QTournamentStore & StoreManager::getTournament() {
    return *mTournament;
}

const QTournamentStore & StoreManager::getTournament() const {
    return *mTournament;
}

void StoreManager::sync(std::unique_ptr<QTournamentStore> tournament) {
    mSyncing += 1;

    emit tournamentAboutToBeReset();

    mTournament = std::move(tournament);
    mConfirmedActionList.clear();
    mConfirmedActionMap.clear();

    mUnconfirmedActionList.clear();
    mUnconfirmedActionMap.clear();

    mRedoList.clear();

    mUnconfirmedUndos.clear();
    mUndoList.clear();
    mUndoListMap.clear();

    if (mNetworkInterface)
        mNetworkInterface->postSync(std::make_unique<TournamentStore>(*mTournament));

    emit tournamentReset();
    emit redoStatusChanged(false);
    emit undoStatusChanged(false);
}

void StoreManager::sync() {
    sync(std::move(mTournament));
}

bool StoreManager::canUndo() {
    return !mUndoList.empty();
}

bool StoreManager::canRedo() {
    return !mRedoList.empty();
}

void StoreManager::redo() {
    assert(canRedo());

    log_debug().msg("Redoing action");

    auto action = std::move(mRedoList.back());
    mRedoList.pop_back();

    dispatch(std::move(action));

    if (mRedoList.empty())
        emit redoStatusChanged(false);
}

ActionId StoreManager::generateNextActionId() {
    ActionId id;
    while (true) {
        id = mActionIdGenerator();

        if (mConfirmedActionMap.find(id) != mConfirmedActionMap.end())
            continue;
        if (mUnconfirmedActionMap.find(id) != mUnconfirmedActionMap.end())
            continue;

        return id;
    }
}

void StoreManager::receiveSyncConfirm() {
    mSyncing -= 1;
    log_debug().field("mSyncing", mSyncing).msg("Sync confirmed");
}

void StoreManager::dispatch(std::unique_ptr<Action> action) {
    auto actionId = generateNextActionId();

    log_debug().field("actionId", actionId).msg("Dispatching action");

    auto clone = action->freshClone();
    action->redo(*mTournament);

    mUnconfirmedActionList.push_back({actionId, std::move(action)});
    mUnconfirmedActionMap[actionId] = std::prev(mUnconfirmedActionList.end());

    mUndoList.push_back(actionId);
    mUndoListMap[actionId] = std::prev(mUndoList.end());

    mNetworkInterface->postAction(actionId, std::move(clone));

    if (mUndoList.size() == 1) // undo list was empty
        emit undoStatusChanged(true);
}


void StoreManager::receiveAction(ActionId actionId, std::shared_ptr<const Action> sharedAction) {
    if (mSyncing > 0)
        return;

    auto action = sharedAction->freshClone(); // Unique ptrs can't be passed to signals, so instead a shared_ptr is used
    log_debug().field("actionId", actionId).msg("Received action");

    // TODO: Trim stack size
    for (auto it = mUnconfirmedActionList.rbegin(); it != mUnconfirmedActionList.rend(); ++it) {
        auto &a = *(it->second);
        if (a.isDone())
            a.undo(*mTournament);
    }

    action->redo(*mTournament);
    mConfirmedActionList.push_back({actionId, std::move(action)});
    mConfirmedActionMap[actionId] = std::prev(mUnconfirmedActionList.end());

    for (auto it = mUnconfirmedActionList.begin(); it != mUnconfirmedActionList.end(); ++it) {
        // if the action is an unconfirmed undo then leave it undone
        if (mUnconfirmedUndos.find(it->first) != mUnconfirmedUndos.end())
            continue;

        it->second->redo(*mTournament);
    }
}

void StoreManager::undo() {
    assert(canUndo());

    auto actionId = mUndoList.back();
    mUndoList.pop_back();
    mUndoListMap.erase(actionId);

    log_debug().field("actionId", actionId).msg("Undo called");

    mUnconfirmedUndos.insert(actionId);

    std::unique_ptr<Action> clone;

    // unroll unconfirmed stack
    for (auto it = mUnconfirmedActionList.rbegin(); it != mUnconfirmedActionList.rend(); ++it) {
        auto &a = *(it->second);

        if (it->first == actionId) {
            clone = a.freshClone();
            break;
        }

        if (a.isDone())
            a.undo(*mTournament);
    }

    log_debug().field("mapSize", mUnconfirmedActionMap.size()).msg("Rolling");
    // unroll and roll back the confirmed stack if neccesary
    if (mUnconfirmedActionMap.find(actionId) == mUnconfirmedActionMap.end()) {
        assert(mConfirmedActionMap.find(actionId) != mConfirmedActionMap.end());

        auto it = std::prev(mConfirmedActionList.end());
        while (it->first != actionId) {
            if (it->second->isDone())
                it->second->undo(*mTournament);
            it = std::prev(it);
        }

        assert(it->second->isDone());
        it->second->undo(*mTournament);
        clone = it->second->freshClone();
        it = std::next(it);

        while (it != mConfirmedActionList.end()) {
            if (mUnconfirmedUndos.find(it->first) == mUnconfirmedUndos.end())
                it->second->redo(*mTournament);

            it = std::next(it);
        }
    }

    // roll back the unconfirmed stack
    for (auto it = mUnconfirmedActionList.begin(); it != mUnconfirmedActionList.end(); ++it) {
        // if the action is an unconfirmed undo then leave it undone
        if (mUnconfirmedUndos.find(it->first) != mUnconfirmedUndos.end())
            continue;

        auto &a = *(it->second);
        if (a.isDone()) continue; // incase break was called in the first loop

        a.redo(*mTournament);
    }

    assert(clone != nullptr);
    mRedoList.push_back(std::move(clone));

    if (mRedoList.size() > REDO_LIST_MAX_SIZE)
        mRedoList.pop_front();

    mNetworkInterface->postUndo(actionId);

    if (mUndoList.empty())
        emit undoStatusChanged(false);
    if (mRedoList.size() == 1) // list was empty before
        emit redoStatusChanged(true);
}

void StoreManager::receiveActionConfirm(ActionId actionId) {
    if (mSyncing > 0)
        return;

    log_debug().field("actionId", actionId).msg("Received action confirm");

    auto front = std::move(mUnconfirmedActionList.front());
    mUnconfirmedActionList.pop_front();
    mUnconfirmedActionMap.erase(actionId);

    if (front.first != actionId)
        throw std::runtime_error("Received confirmation in wrong order");

    mConfirmedActionList.push_back(std::move(front));
    mConfirmedActionMap[actionId] = std::prev(mConfirmedActionList.end());
}

void StoreManager::receiveUndo(ActionId actionId) {
    if (mSyncing > 0)
        return;

    log_debug().field("actionId", actionId).msg("Received undo");
    // An action can never be unconfirmed if the server sends an undo for it
    assert(mConfirmedActionMap.find(actionId) != mConfirmedActionMap.end());

    // Undo the action above on the confirmed action list
    auto it1 = std::prev(mConfirmedActionList.end());
    while (it1->first != actionId) {
        if (it1->second->isDone())
            it1->second->undo(*mTournament);
        it1 = std::prev(it1);
    }

    // The local client may have an unconfirmed undo for the same action
    if (it1->second->isDone()) {
        assert(mUnconfirmedUndos.find(actionId) == mUnconfirmedUndos.end());
        it1->second->undo(*mTournament);
    }
    else {
        assert(mUnconfirmedUndos.find(actionId) != mUnconfirmedUndos.end());
        mUnconfirmedUndos.erase(actionId);
    }

    // Remove the action from the next
    auto tmp = it1;
    it1 = std::next(it1);

    mConfirmedActionList.erase(tmp);
    mConfirmedActionMap.erase(actionId);

    // Update undo list if neccesary
    auto it2 = mUndoListMap.find(actionId);
    if (it2 != mUndoListMap.end()) {
        mUndoList.erase(it2->second);
        mUndoListMap.erase(it2);
    }

    // Redo the actions above on the confirmed action list
    while (it1 != mConfirmedActionList.end()) {
        if (mUnconfirmedUndos.find(it1->first) == mUnconfirmedUndos.end())
            it1->second->redo(*mTournament);

        it1 = std::next(it1);
    }
}

void StoreManager::receiveUndoConfirm(ActionId actionId) {
    if (mSyncing > 0)
        return;

    log_debug().field("actionId", actionId).msg("Received undo confirm");
    receiveUndo(actionId);
}

