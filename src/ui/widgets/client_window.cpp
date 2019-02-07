#include "ui/widgets/client_window.hpp"
#include "ui/widgets/connect_dialog.hpp"

ClientWindow::ClientWindow() {
    connect(&mStoreManager, &ClientStoreManager::connectionLost, this, &ClientWindow::loseConnection);
    connect(&mStoreManager, &ClientStoreManager::connectionAttemptSucceeded, this, &ClientWindow::succeedConnectionAttempt);
    connect(&mRetryTimer, &QTimer::timeout, this, &ClientWindow::retryTimerHit);
}

void ClientWindow::showConnectDialog() {
    mRetryTimer.stop();
    ConnectDialog dialog(mStoreManager, mPreviousHost, mPreviousPort, mRetryInterval);
    if (dialog.exec() == QDialog::Accepted) {
        mPreviousHost = dialog.getHost();
        mPreviousPort = dialog.getPort();
        mRetryInterval = dialog.getRetryInterval();
    }
}

void ClientWindow::loseConnection() {
    if (mRetryInterval) {
        mRetryTimer.start(*mRetryInterval);
    }
}

void ClientWindow::succeedConnectionAttempt() {
    mRetryTimer.stop();
}

void ClientWindow::retryTimerHit() {
    if (mStoreManager.getNetworkClientState() == NetworkClientState::CONNECTING)
        return;
    if (mStoreManager.getNetworkClientState() == NetworkClientState::CONNECTED)
        return;
    if (!mPreviousPort.has_value())
        return;
    mStoreManager.asyncConnect(mPreviousHost, *mPreviousPort);
}

void ClientWindow::disconnect() {
    mStoreManager.asyncDisconnect();
}

