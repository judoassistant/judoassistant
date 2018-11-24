#pragma once

#include <QObject>
#include <QString>
#include <list>

#include "core.hpp"
#include "id.hpp"
#include "actions/action.hpp"

Q_DECLARE_METATYPE(ActionId);

class QTournamentStore;
class NetworkInterface;

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

    bool containsConfirmedAction(ActionId action) const;
    bool containsUnconfirmedAction(ActionId action) const;

protected:
    void startInterface(std::shared_ptr<NetworkInterface> interface);
    void stopInterface();

    void sync(std::unique_ptr<QTournamentStore> tournament);
    void sync();

protected slots:
    void receiveAction(ActionId actionId, std::shared_ptr<const Action> action);
    void receiveActionConfirm(ActionId actionId);
    void receiveUndo(ActionId actionId);
    void receiveUndoConfirm(ActionId actionId);
    void receiveSyncConfirm();

signals:
    void tournamentAboutToBeReset();
    void tournamentReset();
    void undoStatusChanged(bool canUndo);
    void redoStatusChanged(bool canRedo);

private:
    static const size_t REDO_LIST_MAX_SIZE = 20;

    typedef std::list<std::pair<ActionId, std::unique_ptr<Action>>> ActionList;

    std::unique_ptr<QTournamentStore> mTournament;

    ActionList mConfirmedActionList;
    std::unordered_map<ActionId, ActionList::iterator> mConfirmedActionMap;

    ActionList mUnconfirmedActionList;
    std::unordered_map<ActionId, ActionList::iterator> mUnconfirmedActionMap;

    std::list<std::unique_ptr<Action>> mRedoList;
    std::unordered_set<ActionId> mUnconfirmedUndos;

    std::list<ActionId> mUndoList;
    std::map<ActionId, std::list<ActionId>::iterator> mUndoListMap;

    size_t mSyncing;

    std::shared_ptr<NetworkInterface> mNetworkInterface;
};

