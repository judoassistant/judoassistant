#pragma once

#include "store_managers/store_manager.hpp"

class MasterStoreManager : public StoreManager {
    Q_OBJECT
public:
    MasterStoreManager();
    ~MasterStoreManager();

    void startServer(int port);
    void stopServer();

    void dispatch(std::unique_ptr<Action> action);
    void undo();
    void redo();
    bool isDirty() const;

    bool read(const QString &path);
    bool write(const QString &path);
    void resetTournament();

private:
    bool mDirty;
};
