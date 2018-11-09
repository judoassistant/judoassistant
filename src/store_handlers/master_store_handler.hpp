#pragma once

#include <QObject>
#include <QString>
#include <fstream>
#include <list>

#include "core.hpp"

#include "serializables.hpp"
#include "store_handlers/network_server.hpp"

class MasterStoreHandler : public QStoreHandler {
    Q_OBJECT
public:
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
    void receiveAction(ActionId id, std::shared_ptr<Action> action);
    void receiveActionConfirm(ActionId id);
    void receiveSyncConfirm();

private:
    std::unique_ptr<QTournamentStore> mTournament;
    std::list<std::pair<ActionId, std::shared_ptr<Action>>> mActionStack;
    std::list<std::pair<ActionId, std::shared_ptr<Action>>> mUnconfirmedStack;
    // std::vector<std::shared_ptr<Action>> mRedoStack;
    bool mIsDirty;
    bool mSyncing;
    NetworkServer mServer;
    ActionId::Generator mActionIdGenerator;
};

