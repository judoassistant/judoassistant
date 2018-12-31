#include "widgets/client_window.hpp"
#include "widgets/connect_dialog.hpp"

ClientWindow::ClientWindow() {

}

void ClientWindow::showConnectDialog() {
    ConnectDialog dialog(mStoreManager);
    dialog.exec();
}

