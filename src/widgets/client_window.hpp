#pragma once

#include <QMainWindow>
#include <QTimer>

#include "store_managers/client_store_manager.hpp"

// Class containing UI code for handling connecting to server and
// automatic retries
class ClientWindow : public QMainWindow {
    Q_OBJECT
public:
    ClientWindow();

protected:
    void showConnectDialog();
    void disconnect();
    ClientStoreManager mStoreManager;

private:
    void retryTimerHit();
    void loseConnection();
    void succeedConnectionAttempt();

    QString mPreviousHost;
    QTimer mRetryTimer;
    std::optional<int> mPreviousPort;
    std::optional<std::chrono::seconds> mRetryInterval;
};
