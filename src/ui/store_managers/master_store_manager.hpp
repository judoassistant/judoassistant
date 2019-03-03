#pragma once

#include "ui/network/network_server.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/web/web_client.hpp"

class MasterStoreManager : public StoreManager {
    Q_OBJECT
public:
    MasterStoreManager();
    ~MasterStoreManager();

    void startServer(int port);
    void stopServer();

    void stop() override;


    void dispatch(std::unique_ptr<Action> action) override;
    void undo() override;
    void redo() override;
    bool isDirty() const;

    bool read(const QString &path);
    bool write(const QString &path);
    void resetTournament();

    NetworkServer& getNetworkServer();
    const NetworkServer& getNetworkServer() const;

    WebClient& getWebClient();
    const WebClient& getWebClient() const;

    NetworkServerState getNetworkServerState() const;
    WebClientState getWebClientState() const;

private:
    void changeNetworkServerState(NetworkServerState state);
    void changeWebClientState(WebClientState state);

    WebClientState mWebClientState;
    WebClient mWebClient;

    NetworkServerState mNetworkServerState;
    std::shared_ptr<NetworkServer> mNetworkServer;

    bool mDirty;
};
