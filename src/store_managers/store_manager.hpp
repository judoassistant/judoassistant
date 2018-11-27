#pragma once

#include <QObject>
#include <QString>
#include <list>

#include "core.hpp"
#include "id.hpp"
#include "actions/action.hpp"
#include "store_managers/sync_payload.hpp"

class QTournamentStore;
class NetworkInterface;
class StoreManager;

class ConstActionListIterator {
public:
    ConstActionListIterator & operator++();
    const Action & getAction() const;
    ClientActionId getActionId() const;

    bool operator!=(const ConstActionListIterator &other) const;
    bool operator==(const ConstActionListIterator  &other) const;

private:
    ConstActionListIterator(const StoreManager & storeManager, UniqueActionList::const_iterator it, bool iteratingConfirmedActions);
    void makeValid();

    const StoreManager & mStoreManager;
    bool mIteratingConfirmedActions;
    UniqueActionList::const_iterator mIt;

    friend class StoreManager;
};

class StoreManager : public QObject {
    Q_OBJECT
public:
    StoreManager();
    ~StoreManager();

    QTournamentStore & getTournament();
    const QTournamentStore & getTournament() const;

    void dispatch(std::unique_ptr<Action> action);
    bool canUndo();
    void undo();
    bool canRedo();
    void redo();

    bool containsConfirmedAction(ClientActionId action) const;
    bool containsUnconfirmedAction(ClientActionId action) const;

    const Action & getAction(ClientActionId actionId) const;
    ConstActionListIterator actionsBegin() const;
    ConstActionListIterator actionsEnd() const;

signals:
    void tournamentAboutToBeReset();
    void tournamentReset();
    void undoStatusChanged(bool canUndo);
    void redoStatusChanged(bool canRedo);

    void actionAboutToBeErased(ClientActionId actionId);
    void actionErased(ClientActionId actionId);
    void actionAboutToBeAdded(ClientActionId actionId, size_t pos);
    void actionAdded(ClientActionId actionId, size_t pos);

protected:
    void startInterface(std::shared_ptr<NetworkInterface> interface);
    void stopInterface();

    void sync(std::unique_ptr<QTournamentStore> tournament);
    void sync();

    void receiveAction(ClientActionId actionId, ActionPtr action);
    void receiveActionConfirm(ClientActionId actionId);
    void receiveUndo(ClientActionId actionId);
    void receiveUndoConfirm(ClientActionId actionId);
    void receiveSync(SyncPayloadPtr syncPayload);
    void confirmSync();

private:
    static const size_t REDO_LIST_MAX_SIZE = 20;

    ClientId mId;
    std::unique_ptr<QTournamentStore> mTournament;

    UniqueActionList mConfirmedActionList;
    std::unordered_map<ClientActionId, UniqueActionList::iterator> mConfirmedActionMap;

    UniqueActionList mUnconfirmedActionList;
    std::unordered_map<ClientActionId, UniqueActionList::iterator> mUnconfirmedActionMap;

    std::list<std::unique_ptr<Action>> mRedoList;
    std::unordered_set<ClientActionId> mUnconfirmedUndos;
    size_t mUndoneUnconfirmedActions;

    std::optional<ClientActionId> mUndoActionId;

    size_t mSyncing;

    std::shared_ptr<NetworkInterface> mNetworkInterface;

    friend class ConstActionListIterator;
};


