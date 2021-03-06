#pragma once

#include <chrono>
#include <list>

#include <QObject>
#include <QString>

#include "core/actions/action.hpp"
#include "core/core.hpp"
#include "core/id.hpp"
#include "ui/network/network_interface.hpp"
#include "ui/store_managers/worker_thread.hpp"

class QTournamentStore;
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
    virtual ~StoreManager();

    virtual void stop();
    void wait();

    QTournamentStore & getTournament();
    const QTournamentStore & getTournament() const;

    virtual void dispatch(std::unique_ptr<Action> action);
    virtual bool canUndo();
    virtual void undo();
    virtual void undo(ClientActionId action);
    virtual bool canRedo();
    virtual void redo();

    bool containsConfirmedAction(ClientActionId action) const;
    bool containsUnconfirmedAction(ClientActionId action) const;

    const Action & getAction(ClientActionId actionId) const;
    ConstActionListIterator actionsBegin() const;
    ConstActionListIterator actionsEnd() const;

    virtual std::chrono::milliseconds masterTime() const = 0;
    std::chrono::milliseconds localTime() const;

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
    void setInterface(std::shared_ptr<NetworkInterface> interface);

    void sync(std::unique_ptr<QTournamentStore> tournament);
    void sync();

    void receiveAction(ClientActionId actionId, ActionPtr action);
    void receiveActionConfirm(ClientActionId actionId);
    void receiveUndo(ClientActionId actionId);
    void receiveUndoConfirm(ClientActionId actionId);
    void receiveSync(SyncPayloadPtr syncPayload);
    void confirmSync();

    void popActionListFront();

    WorkerThread& getWorkerThread();

private:
    WorkerThread mThread;
    static const size_t REDO_LIST_MAX_SIZE = 20;

    std::shared_ptr<NetworkInterface> mNetworkInterface;

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

    friend class ConstActionListIterator;
};

