#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QGroupBox>
#include <QHeaderView>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QVBoxLayout>

#include "ui/constants/homepage.hpp"
#include "ui/misc/dark_palette.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/kiosk_window.hpp"
#include "ui/widgets/matches_widget.hpp"
#include "ui/widgets/warning_widget.hpp"

KioskWindow::KioskWindow()
{
    QWidget *centralWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    mWarningWidget = new WarningWidget(tr("This display is not connected. The information shown may not be up to date!"));
    layout->addWidget(mWarningWidget);

    auto *matches = new MatchesWidget(mStoreManager);

    layout->addWidget(matches);

    createTournamentMenu();
    createPreferencesMenu();
    createHelpMenu();

    setCentralWidget(centralWidget);
    setWindowTitle(tr("JudoAssistant Kiosk"));
}

void KioskWindow::createTournamentMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Tournament"));

    mConnectAction = new QAction(tr("Connect.."), this);
    mConnectAction->setShortcuts(QKeySequence::New);
    mConnectAction->setStatusTip(tr("Connect to hub"));
    connect(mConnectAction, &QAction::triggered, this, &KioskWindow::showConnectDialog);
    menu->addAction(mConnectAction);

    mDisconnectAction = new QAction(tr("Disconnect"), this);
    mDisconnectAction->setShortcuts(QKeySequence::New);
    mDisconnectAction->setStatusTip(tr("Disconnect from hub"));
    connect(mDisconnectAction, &QAction::triggered, this, &KioskWindow::disconnect);
    menu->addAction(mDisconnectAction);

    changeNetworkClientState(mStoreManager.getNetworkClientState());
    connect(&mStoreManager.getNetworkClient(), &NetworkClient::stateChanged, this, &KioskWindow::changeNetworkClientState);
}

void KioskWindow::createPreferencesMenu() {
    DarkPalette palette;
    QApplication::setPalette(palette);
    setPalette(palette);

    // QMenu *menu = menuBar()->addMenu(tr("Preferences"));
    // // {
    // //     QMenu *submenu = menu->addMenu("Language");
    // //     QAction *englishAction = new QAction(tr("English"), this);
    // //     submenu->addAction(englishAction);
    // // }
    // {
    //     {
    //         DarkPalette palette;
    //         QApplication::setPalette(palette);
    //         setPalette(palette);
    //     }

    //     QMenu *submenu = menu->addMenu("Color Scheme");
    //     auto *actionGroup = new QActionGroup(this);

    //     QAction *darkAction = new QAction(tr("Dark"), this);
    //     darkAction->setCheckable(true);
    //     darkAction->setChecked(true);
    //     actionGroup->addAction(darkAction);

    //     connect(darkAction, &QAction::triggered, [this]() {
    //         DarkPalette palette;
    //         setPalette(palette);
    //     });

    //     QAction *lightAction = new QAction(tr("Light"), this);
    //     lightAction->setCheckable(true);
    //     actionGroup->addAction(lightAction);

    //     connect(lightAction, &QAction::triggered, [this]() {
    //         auto palette = this->style()->standardPalette();
    //         QApplication::setPalette(palette);
    //         setPalette(palette);
    //     });

    //     submenu->addAction(darkAction);
    //     submenu->addAction(lightAction);
    // }
}

void KioskWindow::createHelpMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Help"));
    {
        QAction *action = new QAction(tr("JudoAssistant Home Page"), this);
        connect(action, &QAction::triggered, this, &KioskWindow::openHomePage);
        menu->addAction(action);
    }
    {
        QAction *action = new QAction(tr("JudoAssistant Manual"), this);
        connect(action, &QAction::triggered, this, &KioskWindow::openManual);
        menu->addAction(action);
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Report an Issue"), this);
        connect(action, &QAction::triggered, this, &KioskWindow::openReportIssue);

        menu->addAction(action);
    }
    {
        QAction *action = new QAction(tr("About"), this);
        connect(action, &QAction::triggered, this, &KioskWindow::showAboutDialog);
        menu->addAction(action);
    }
}

void KioskWindow::openHomePage() {
    if(QDesktopServices::openUrl(Constants::HOME_PAGE_URL))
        return;

    // QMessageBox::warning(this, tr("Unable to open home page."));
}

void KioskWindow::openManual() {
    if(QDesktopServices::openUrl(Constants::MANUAL_URL))
        return;

    // QMessageBox::warning(this, tr("Unable to open manual."), tr("The manual is available at"));
}

void KioskWindow::openReportIssue() {
    if(QDesktopServices::openUrl(Constants::REPORT_ISSUE_URL))
        return;

    // QMessageBox::warning(this, tr("Unable to open report issue page."));
}

void KioskWindow::quit() {
    QCoreApplication::exit();
}

void KioskWindow::showAboutDialog() {
    QMessageBox::about(this, tr("JudoAssistant - About"), tr("TODO"));
}

void KioskWindow::silentConnect(QString host, int port) {
    mStoreManager.connect(host, port);
}

void KioskWindow::changeNetworkClientState(NetworkClientState state) {
    mConnectAction->setEnabled(state == NetworkClientState::NOT_CONNECTED);
    mDisconnectAction->setEnabled(state == NetworkClientState::CONNECTED);

    mWarningWidget->setVisible(state != NetworkClientState::CONNECTED);
}

