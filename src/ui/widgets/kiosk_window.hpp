#pragma once

#include "core/core.hpp"
#include "ui/constants/network.hpp"
#include "ui/widgets/client_window.hpp"
#include "ui/widgets/warning_widget.hpp"

class WarningWidget;

class KioskWindow : public ClientWindow {
    Q_OBJECT

public:
    KioskWindow();
    void silentConnect(QString host, int port=Constants::DEFAULT_PORT);

private:
    // TODO: Refactor these into superclass
    void quit();
    void openHomePage();
    void openManual();
    void openReportIssue();
    void showAboutDialog();

    void createStatusBar();
    void createTournamentMenu();
    void createPreferencesMenu();
    void createHelpMenu();

    void changeNetworkClientState(NetworkClientState state);

    QAction *mConnectAction;
    QAction *mDisconnectAction;
    WarningWidget *mWarningWidget;
};

