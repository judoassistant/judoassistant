#pragma once

#include "store_managers/store_manager.hpp"
#include "web/web_client.hpp"

class MasterStoreManager : public StoreManager {
    Q_OBJECT
public:
    MasterStoreManager();
    ~MasterStoreManager();

    void startServer(int port);
    void stopServer();

    void dispatch(std::unique_ptr<Action> action) override;
    void undo() override;
    void redo() override;
    bool isDirty() const;

    bool read(const QString &path);
    bool write(const QString &path);
    void resetTournament();

    void changeWebStatus(WebClient::Status status);

private:
    bool mDirty;
    WebClient mWebClient;
    WebClient::Status mWebStatus;
};

