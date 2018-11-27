#include "network/network_interface.hpp"
#include "store_managers/store_manager.hpp"
#include "stores/qtournament_store.hpp"

StoreManager::StoreManager()
    : mId(ClientId::generate())
    , mTournament(std::make_unique<QTournamentStore>())
    , mUndoneUnconfirmedActions(0)
    , mSyncing(0)
{
    registerMetatypes();
}

StoreManager::~StoreManager() {
    if (mNetworkInterface)
        stopInterface();
}

void StoreManager::startInterface(std::shared_ptr<NetworkInterface> interface) {
    if (mNetworkInterface != nullptr)
        throw std::runtime_error("Attempted to start StoreManager interface with another one running");

    mNetworkInterface = std::move(interface);

    connect(mNetworkInterface.get(), &NetworkInterface::actionReceived, this, &StoreManager::receiveAction);
    connect(mNetworkInterface.get(), &NetworkInterface::actionConfirmReceived, this, &StoreManager::receiveActionConfirm);

    connect(mNetworkInterface.get(), &NetworkInterface::undoReceived, this, &StoreManager::receiveUndo);
    connect(mNetworkInterface.get(), &NetworkInterface::undoConfirmReceived, this, &StoreManager::receiveUndoConfirm);

    connect(mNetworkInterface.get(), &NetworkInterface::syncReceived, this, &StoreManager::receiveSync);
    connect(mNetworkInterface.get(), &NetworkInterface::syncConfirmed, this, &StoreManager::confirmSync);

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
    mUndoneUnconfirmedActions = 0;

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
    return mUndoActionId.has_value();
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

void StoreManager::confirmSync() {
    mSyncing -= 1;
    log_debug().field("mSyncing", mSyncing).msg("Sync confirmed");
}

void StoreManager::dispatch(std::unique_ptr<Action> action) {
    ClientActionId actionId;

    while (true) {
        actionId = ClientActionId(mId, ActionId::generate());

        if (containsConfirmedAction(actionId))
            continue;
        if (containsUnconfirmedAction(actionId))
            continue;

        break;
    }

    log_debug().field("actionId", actionId).msg("Dispatching action");

    auto clone = action->freshClone();
    action->redo(*mTournament);

    size_t pos = mConfirmedActionList.size() + mUnconfirmedActionList.size();
    emit actionAboutToBeAdded(actionId, pos);
    mUnconfirmedActionList.push_back({actionId, std::move(action)});
    mUnconfirmedActionMap[actionId] = std::prev(mUnconfirmedActionList.end());
    emit actionAdded(actionId, pos);

    bool hadUndoAction = mUndoActionId.has_value();
    mUndoActionId = actionId;

    mNetworkInterface->postAction(actionId, std::move(clone));

    if (!hadUndoAction)
        emit undoStatusChanged(true);
}


void StoreManager::receiveAction(ClientActionId actionId, ActionPtr sharedAction) {
    if (mSyncing > 0)
        return;

    auto action = sharedAction->freshClone(); // Unique ptrs can't be passed to signals, so instead a shared_ptr is used
    log_debug().field("actionId", actionId).msg("Received action");

    size_t pos = mConfirmedActionList.size() - (mUnconfirmedUndos.size() - mUndoneUnconfirmedActions);
    emit actionAboutToBeAdded(actionId, pos);
    log_debug().field("actionId", actionId).msg("StoreManager::receiveAction");

    // TODO: Trim stack size
    for (auto it = mUnconfirmedActionList.rbegin(); it != mUnconfirmedActionList.rend(); ++it) {
        auto &a = *(it->second);
        if (a.isDone())
            a.undo(*mTournament);
    }

    action->redo(*mTournament);
    mConfirmedActionList.push_back({actionId, std::move(action)});
    mConfirmedActionMap[actionId] = std::prev(mConfirmedActionList.end());

    for (auto it = mUnconfirmedActionList.begin(); it != mUnconfirmedActionList.end(); ++it) {
        // if the action is an unconfirmed undo then leave it undone
        if (mUnconfirmedUndos.find(it->first) != mUnconfirmedUndos.end())
            continue;

        it->second->redo(*mTournament);
    }
    emit actionAdded(actionId, pos);
}

void StoreManager::undo() {
    assert(canUndo());

    auto actionId = *mUndoActionId;

    log_debug().field("actionId", actionId).msg("Undo called");

    mUnconfirmedUndos.insert(actionId);

    std::unique_ptr<Action> clone;

    emit actionAboutToBeErased(actionId);
    // unroll unconfirmed stack
    for (auto it = mUnconfirmedActionList.rbegin(); it != mUnconfirmedActionList.rend(); ++it) {
        auto &a = *(it->second);

        if (a.isDone())
            a.undo(*mTournament);

        if (it->first == actionId) {
            clone = a.freshClone();
            ++mUndoneUnconfirmedActions;
            break;
        }
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
        if (a.isDone()) continue; // in case break was called in the first loop

        a.redo(*mTournament);
    }

    emit actionErased(actionId);

    // Find the new mUndoActionId
    {
        mUndoActionId = std::nullopt;

        // Iterate over both lists using one loop
        bool iteratingUnconfirmed = (mUnconfirmedActionMap.find(actionId) != mUnconfirmedActionMap.end());
        auto it = (iteratingUnconfirmed ? mUnconfirmedActionMap.find(actionId)->second : mConfirmedActionMap.find(actionId)->second);

        while (true) {
            if (it->second->isDone() && it->first.getClientId() == mId) {
                mUndoActionId = it->first;
                break;
            }

            // Update iterator
            if (!iteratingUnconfirmed && it == mConfirmedActionList.begin()) {
                break;
            }

            if (iteratingUnconfirmed && it == mUnconfirmedActionList.begin()) {
                if (mConfirmedActionList.empty())
                    break;

                it = std::prev(mConfirmedActionList.end());
                iteratingUnconfirmed = false;
                continue;
            }

            it = std::prev(it);
        }
    }


    // Update redo list
    assert(clone != nullptr);
    mRedoList.push_back(std::move(clone));

    if (mRedoList.size() > REDO_LIST_MAX_SIZE)
        mRedoList.pop_front();

    mNetworkInterface->postUndo(actionId);

    if (!mUndoActionId.has_value())
        emit undoStatusChanged(false);
    if (mRedoList.size() == 1) // list was empty before
        emit redoStatusChanged(true);
}

void StoreManager::receiveActionConfirm(ClientActionId actionId) {
    if (mSyncing > 0)
        return;

    log_debug().field("actionId", actionId).msg("Received action confirm");

    auto front = std::move(mUnconfirmedActionList.front());
    mUnconfirmedActionList.pop_front();
    mUnconfirmedActionMap.erase(actionId);

    if (front.first != actionId)
        throw std::runtime_error("Received confirmation in wrong order");

    if (!front.second->isDone()) {
        assert(mUndoneUnconfirmedActions > 0);
        --mUndoneUnconfirmedActions;
    }

    mConfirmedActionList.push_back(std::move(front));
    mConfirmedActionMap[actionId] = std::prev(mConfirmedActionList.end());
}

void StoreManager::receiveUndo(ClientActionId actionId) {
    // TODO: Refactor code
    if (mSyncing > 0)
        return;

    log_debug().field("actionId", actionId).msg("Received undo");
    emit actionAboutToBeErased(actionId);

    // An action can never be unconfirmed if the server sends an undo for it
    assert(mConfirmedActionMap.find(actionId) != mConfirmedActionMap.end());

    // Undo the actions above the action to undo
    for (auto it = mUnconfirmedActionList.rbegin(); it != mUnconfirmedActionList.rend(); ++it) {
        auto &action = *(it->second);
        if (action.isDone())
            action.undo(*mTournament);
    }

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
    {
        auto tmp = it1;
        it1 = std::next(it1);

        mConfirmedActionList.erase(tmp);
        mConfirmedActionMap.erase(actionId);
    }

    // Update undo action id if neccesary
    if (mUndoActionId == actionId) {
        mUndoActionId = std::nullopt;

        if (it1 != mConfirmedActionList.begin()) {
            auto it2 = std::prev(it1);
            while (true) {
                if (it2->second->isDone() && it2->first.getClientId() == mId) {
                    mUndoActionId = it2->first;
                    break;
                }

                if(it2 == mConfirmedActionList.begin())
                    break;
                else
                    --it2;
            }
        }

        if (!mUndoActionId.has_value())
            emit undoStatusChanged(false);
    }

    // Redo the actions above on the confirmed action list
    while (it1 != mConfirmedActionList.end()) {
        if (mUnconfirmedUndos.find(it1->first) == mUnconfirmedUndos.end())
            it1->second->redo(*mTournament);

        it1 = std::next(it1);
    }

    for (auto it = mUnconfirmedActionList.begin(); it != mUnconfirmedActionList.end(); ++it) {
        if (mUnconfirmedUndos.find(it->first) == mUnconfirmedUndos.end())
            it->second->redo(*mTournament);
    }

    emit actionErased(actionId);
}

void StoreManager::receiveUndoConfirm(ClientActionId actionId) {
    if (mSyncing > 0)
        return;

    log_debug().field("actionId", actionId).msg("Received undo confirm");

    mUnconfirmedUndos.erase(actionId);
    auto it = mConfirmedActionMap.find(actionId);

    if (it != mConfirmedActionMap.end()) {
        mConfirmedActionList.erase(it->second);
        mConfirmedActionMap.erase(it);
    }
}

bool StoreManager::containsConfirmedAction(ClientActionId action) const {
    return mConfirmedActionMap.find(action) != mConfirmedActionMap.end();
}

bool StoreManager::containsUnconfirmedAction(ClientActionId action) const {
    return mUnconfirmedActionMap.find(action) != mUnconfirmedActionMap.end();
}

void StoreManager::receiveSync(SyncPayloadPtr payload) {
    emit tournamentAboutToBeReset();
    log_debug().msg("Received sync");

    if (mTournament->getId() != payload->tournament->getId())
        mRedoList.clear();

    mTournament = std::move(payload->tournament);

    mUndoActionId = std::nullopt;

    mConfirmedActionList = std::move(*(payload->confirmedActionList));
    mConfirmedActionMap.clear();
    for (auto it = mConfirmedActionList.begin(); it != mConfirmedActionList.end(); ++it) {
        if (it->first.getClientId() == mId)
            mUndoActionId = it->first;

        mConfirmedActionMap[it->first] = it;
    }

    mUnconfirmedActionList = std::move(*(payload->unconfirmedActionList));
    mUnconfirmedActionMap.clear();
    mUndoneUnconfirmedActions = 0;

    for (auto it = mUnconfirmedActionList.begin(); it != mUnconfirmedActionList.end(); ++it) {
        assert(it->first.getClientId() == mId);
        if (payload->unconfirmedUndos->find(it->first) != payload->unconfirmedUndos->end())
            ++mUndoneUnconfirmedActions;
        mUndoActionId = it->first;
    }

    mUnconfirmedUndos = std::move(*(payload->unconfirmedUndos));

    emit tournamentReset();
    emit redoStatusChanged(!mRedoList.empty());
    emit undoStatusChanged(mUndoActionId.has_value());
}

const Action & StoreManager::getAction(ClientActionId actionId) const {
    auto it = mUnconfirmedActionMap.find(actionId);
    if (it != mUnconfirmedActionMap.end())
        return *(it->second->second);

    it = mConfirmedActionMap.find(actionId);
    if (it != mConfirmedActionMap.end())
        return *(it->second->second);

    throw std::out_of_range("The action with the specified id does not exist");
}

ConstActionListIterator StoreManager::actionsBegin() const {
    return ConstActionListIterator(*this, mConfirmedActionList.begin(), true);
}

ConstActionListIterator StoreManager::actionsEnd() const {
    return ConstActionListIterator(*this, mUnconfirmedActionList.begin(), false);
}

ConstActionListIterator::ConstActionListIterator(const StoreManager &storeManager, UniqueActionList::const_iterator it, bool iteratingConfirmedActions)
    : mStoreManager(storeManager)
    , mIteratingConfirmedActions(iteratingConfirmedActions)
    , mIt(it)
{
    makeValid();
}

void ConstActionListIterator::makeValid() {
    while (true) {
        if (mIteratingConfirmedActions && mIt == mStoreManager.mConfirmedActionList.end()) {
            mIt = mStoreManager.mUnconfirmedActionList.begin();
            mIteratingConfirmedActions = false;
        }

        if (!mIteratingConfirmedActions && mIt == mStoreManager.mUnconfirmedActionList.end())
            return;

        if (getAction().isDone())
            return;

        ++mIt;
    }
}

ConstActionListIterator & ConstActionListIterator::operator++() {
    ++mIt;
    makeValid();

    return *this;
}

const Action & ConstActionListIterator::getAction() const {
    return *(mIt->second);
}

ClientActionId ConstActionListIterator::getActionId() const {
    return mIt->first;
}

bool ConstActionListIterator::operator!=(const ConstActionListIterator &other) const {
    return mIteratingConfirmedActions != other.mIteratingConfirmedActions || mIt != other.mIt;
}

bool ConstActionListIterator::operator==(const ConstActionListIterator  &other) const {
    return mIteratingConfirmedActions == other.mIteratingConfirmedActions && mIt == other.mIt;
}

