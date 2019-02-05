#pragma once

#include "ui/store_managers/store_manager.hpp"
#include "ui/web/web_client.hpp"

class MasterStoreManager : public StoreManager {
    Q_OBJECT
public:
    MasterStoreManager();
    ~MasterStoreManager();

    void asyncStartServer(int port);
    void asyncStopServer();

    void dispatch(std::unique_ptr<Action> action) override;
    void undo() override;
    void redo() override;
    bool isDirty() const;

    bool read(const QString &path);
    bool write(const QString &path);
    void resetTournament();

    WebClient& getWebClient();
    const WebClient& getWebClient() const;

    void stop() override;

private:
    bool mDirty;
    WebClient mWebClient;
};

