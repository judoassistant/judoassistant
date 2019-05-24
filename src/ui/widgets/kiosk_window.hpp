#pragma once

#include <stack>

#include <QActionGroup>
#include <QMetaObject>
#include <QPushButton>
#include <QTableView>

#include "core/core.hpp"
#include "core/stores/tatami/location.hpp"
#include "ui/constants/network.hpp"
#include "ui/widgets/client_window.hpp"
#include "ui/widgets/match_card_widget.hpp"
#include "ui/widgets/score_display_widget.hpp"
#include "ui/widgets/score_display_window.hpp"

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
    void createEditMenu();
    void createViewMenu();
    void createPreferencesMenu();
    void createHelpMenu();

    void changeNetworkClientState(NetworkClientState state);

    QAction *mConnectAction;
    QAction *mDisconnectAction;
};

