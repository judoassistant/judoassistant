#include "widgets/client_window.hpp"
#include "widgets/connect_dialog.hpp"

ClientWindow::ClientWindow() {

}

void ClientWindow::showConnectDialog() {
    // TODO: disable retry timer
    ConnectDialog dialog(mStoreManager, mPreviousHost, mPreviousPort, mRetryInterval);
    if (dialog.exec() == QDialog::Accepted) {
        mPreviousHost = dialog.getHost();
        mPreviousPort = dialog.getPort();
        mRetryInterval = dialog.getRetryInterval();
    }
}

