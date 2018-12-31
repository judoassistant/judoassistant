#pragma once

#include <QMainWindow>

#include "store_managers/client_store_manager.hpp"

// Class containing UI code for handling connecting to server and
// automatic retries
class ClientWindow : public QMainWindow {
    Q_OBJECT
public:
    ClientWindow();
protected:
    void showConnectDialog();

    ClientStoreManager mStoreManager;
};
