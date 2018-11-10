#pragma once

#include <QObject>
#include <QString>
#include <fstream>
#include <list>

#include "core.hpp"

#include "serializables.hpp"
#include "store_handlers/network_server.hpp"

Q_DECLARE_METATYPE(ActionId);

class MasterStoreHandler : public QStoreHandler {
    Q_OBJECT
public:
    static const size_t REDO_LIST_MAX_SIZE = 20;

    MasterStoreHandler();
    ~MasterStoreHandler();

    QTournamentStore & getTournament() override;
    const QTournamentStore & getTournament() const override;
    void dispatch(std::unique_ptr<Action> && action) override;
    bool canUndo() override;
    void undo() override;
    bool canRedo() override;
    void redo() override;
    bool read(const QString &path);
    bool write(const QString &path);
    void reset();
    bool isDirty() const;

protected slots:
    void receiveAction(ActionId actionId, std::shared_ptr<const Action> action);
    void receiveActionConfirm(ActionId actionId);
    void receiveUndo(ActionId actionId);
    void receiveUndoConfirm(ActionId actionId);
    void receiveSyncConfirm();

private:
    ActionId generateNextActionId();
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

    bool mIsDirty;
    size_t mSyncing;
    NetworkServer mServer;
    ActionId::Generator mActionIdGenerator;
};

