#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QStatusBar>
#include <QUrl>

#include <fstream>

#include "core/exception.hpp"
#include "core/log.hpp"
#include "core/version.hpp"
#include "ui/constants/homepage.hpp"
#include "ui/constants/network.hpp"
#include "ui/constants/settings.hpp"
#include "ui/import_helpers/csv_reader.hpp"
#include "ui/misc/dark_palette.hpp"
#include "ui/widgets/categories_widget.hpp"
#include "ui/widgets/hub_window.hpp"
#include "ui/widgets/import_players_csv_dialog.hpp"
#include "ui/widgets/tournament_preferences_dialog.hpp"
#include "ui/widgets/judoassistant_preferences_dialog.hpp"
#include "ui/widgets/matches_widget.hpp"
#include "ui/widgets/players_widget.hpp"
#include "ui/widgets/sidebar_widget.hpp"
#include "ui/widgets/tatamis_widget.hpp"
#include "ui/widgets/tournament_widget.hpp"

HubWindow::HubWindow() {
    DarkPalette palette;
    QApplication::setPalette(palette);
    setPalette(palette);

    createStatusBar();
    createTournamentMenu();
    createEditMenu();
    createViewMenu();
    // createPreferencesMenu();
    createHelpMenu();

    SidebarWidget * sidebar = new SidebarWidget(this);
    sidebar->addTab(new TournamentWidget(mStoreManager), tr("Tournament"));
    sidebar->addTab(new PlayersWidget(mStoreManager), tr("Players"));
    sidebar->addTab(new CategoriesWidget(mStoreManager), tr("Categories"));
    sidebar->addTab(new TatamisWidget(mStoreManager), tr("Tatamis"));
    sidebar->addTab(new MatchesWidget(mStoreManager), tr("Matches"));
    sidebar->setCurrentIndex(0);

    setCentralWidget(sidebar);

    resize(250,250);
    setWindowTitle(tr("JudoAssistant"));

    connect(&mStoreManager.getNetworkServer(), &NetworkServer::startFailed, this, &HubWindow::showServerStartFailure);
    connect(&mAutosaveTimer, &QTimer::timeout, this, &HubWindow::autosaveTimerHit);

    const QSettings& settings = mStoreManager.getSettings();
    bool autosave = settings.value(Constants::Settings::AUTOSAVE_ENABLED, true).toBool();

    if (autosave) {
        mAutosaveFrequency = std::chrono::minutes(settings.value(Constants::Settings::AUTOSAVE_FREQUENCY, 5).toInt());
        mAutosaveTimer.start(*mAutosaveFrequency);
    }
}

void HubWindow::startServer() {
    mStoreManager.startServer(Constants::DEFAULT_PORT);
}

void HubWindow::showServerStartFailure() {
    QMessageBox::warning(this, tr("Unable start server"), tr("JudoAssistant was unable to start the server for communication."));
}

void HubWindow::createStatusBar() {
    statusBar();
}

void HubWindow::createTournamentMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Tournament"));

    {
        QAction *action = new QAction(tr("New"), this);
        action->setShortcuts(QKeySequence::New);
        action->setStatusTip(tr("Create a new tournament"));
        connect(action, &QAction::triggered, this, &HubWindow::newTournament);
        menu->addAction(action);
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Open.."), this);
        action->setShortcuts(QKeySequence::Open);
        action->setStatusTip(tr("Load tournament from a file"));
        connect(action, &QAction::triggered, this, &HubWindow::openTournament);
        menu->addAction(action);
    }

    {
        mRecentFilesMenu = menu->addMenu(tr("Open Recent.."));
        refreshRecentFilesMenu();
    }

    menu->addSeparator();

    {
        QMenu *submenu = menu->addMenu(tr("Import.."));

        {
            QAction *action = new QAction(tr("Import players.."), this);
            action->setStatusTip(tr("Import players from a file"));
            connect(action, &QAction::triggered, this, &HubWindow::openImportPlayers);
            submenu->addAction(action);
        }
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Save"), this);
        action->setShortcuts(QKeySequence::Save);
        action->setStatusTip(tr("Save tournament to a file"));
        connect(action, &QAction::triggered, this, &HubWindow::saveTournament);
        menu->addAction(action);
    }

    {
        QAction *action = new QAction(tr("Save As.."), this);
        action->setShortcuts(QKeySequence::SaveAs);
        action->setStatusTip(tr("Save the tournament to file"));
        connect(action, &QAction::triggered, this, &HubWindow::saveAsTournament);
        menu->addAction(action);
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Quit"), this);
        action->setShortcuts(QKeySequence::Quit);
        action->setStatusTip(tr("Quit JudoAssistant"));
        connect(action, &QAction::triggered, this, &HubWindow::quit);
        menu->addAction(action);
    }
}

void HubWindow::createEditMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Edit"));

    {
        QAction *action = new QAction(tr("Undo"), this);
        action->setShortcuts(QKeySequence::Undo);
        action->setStatusTip(tr("Undo last action"));
        action->setEnabled(mStoreManager.canUndo());
        menu->addAction(action);
        connect(&mStoreManager, &MasterStoreManager::undoStatusChanged, action, &QAction::setEnabled);
        connect(action, &QAction::triggered, &mStoreManager, &MasterStoreManager::undo);
    }

    {
        QAction *action = new QAction(tr("Redo"), this);
        action->setShortcuts(QKeySequence::Redo);
        action->setStatusTip(tr("Redo the last undone action"));
        action->setEnabled(mStoreManager.canRedo());
        menu->addAction(action);
        connect(&mStoreManager, &MasterStoreManager::redoStatusChanged, action, &QAction::setEnabled);
        connect(action, &QAction::triggered, &mStoreManager, &MasterStoreManager::redo);
    }

    menu->addSeparator();
    {
        QAction *action = new QAction(tr("Tournament Preferences.."), this);
        action->setStatusTip(tr("Open the Tournament Preferences dialog"));
        menu->addAction(action);
        connect(action, &QAction::triggered, this, &HubWindow::showTournamentPreferences);
    }
    {
        QAction *action = new QAction(tr("JudoAssistant Preferences.."), this);
        action->setStatusTip(tr("Open the JudoAssistant Preferences dialog"));
        menu->addAction(action);
        connect(action, &QAction::triggered, this, &HubWindow::showJudoAssistantPreferences);
    }
}

void HubWindow::createViewMenu() {
    // QMenu *menu = menuBar()->addMenu(tr("View"));
}

void HubWindow::createPreferencesMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Preferences"));
    {
        QMenu *submenu = menu->addMenu("Language");
        auto *actionGroup = new QActionGroup(this);

        QAction *englishAction = new QAction(tr("English"), this);
        actionGroup->addAction(englishAction);

        englishAction->setCheckable(true);
        englishAction->setChecked(true);
        submenu->addAction(englishAction);
    }
    {
        {
            DarkPalette palette;
            QApplication::setPalette(palette);
            setPalette(palette);
        }

        QMenu *submenu = menu->addMenu("Color Scheme");
        auto *actionGroup = new QActionGroup(this);

        QAction *darkAction = new QAction(tr("Dark"), this);
        darkAction->setCheckable(true);
        darkAction->setChecked(true);
        actionGroup->addAction(darkAction);

        connect(darkAction, &QAction::triggered, [this]() {
            DarkPalette palette;
            QApplication::setPalette(palette);
            setPalette(palette);
        });

        QAction *lightAction = new QAction(tr("Light"), this);
        lightAction->setCheckable(true);
        actionGroup->addAction(lightAction);

        connect(lightAction, &QAction::triggered, [this]() {
            auto palette = this->style()->standardPalette();
            QApplication::setPalette(palette);
            setPalette(palette);
        });

        submenu->addAction(darkAction);
        submenu->addAction(lightAction);
    }
}

void HubWindow::createHelpMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Help"));
    {
        QAction *action = new QAction(tr("JudoAssistant Home Page"), this);
        connect(action, &QAction::triggered, this, &HubWindow::openHomePage);
        menu->addAction(action);
    }
    {
        QAction *action = new QAction(tr("JudoAssistant Manual"), this);
        connect(action, &QAction::triggered, this, &HubWindow::openManual);
        menu->addAction(action);
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Report an Issue"), this);
        connect(action, &QAction::triggered, this, &HubWindow::openReportIssue);

        menu->addAction(action);
    }
    {
        QAction *action = new QAction(tr("About"), this);
        connect(action, &QAction::triggered, this, &HubWindow::showAboutDialog);
        menu->addAction(action);
    }
}

void HubWindow::openHomePage() {
    if(QDesktopServices::openUrl(Constants::HOME_PAGE_URL))
        return;

    // QMessageBox::warning(this, tr("Unable to open home page."));
}

void HubWindow::openManual() {
    if(QDesktopServices::openUrl(Constants::MANUAL_URL))
        return;

    QMessageBox::warning(this, tr("Unable to open manual."), tr("The manual is available at"));
}

void HubWindow::openReportIssue() {
    if(QDesktopServices::openUrl(Constants::REPORT_ISSUE_URL))
        return;

    // QMessageBox::warning(this, tr("Unable to open report issue page."));
}

void HubWindow::writeTournament() {
    QSettings& settings = mStoreManager.getSettings();
    unsigned int backupAmount = 0;
    if (settings.value(Constants::Settings::BACKUP_ENABLED, false).toBool())
        backupAmount = settings.value(Constants::Settings::BACKUP_AMOUNT, 2).toInt();

    if (!mStoreManager.write(mFileName, backupAmount)) {
        QMessageBox::warning(this, tr("Unable to write file"), tr("Unable to save to the selected tournament file."));
        return;
    }

    addToRecentFiles(mFileName);
    statusBar()->showMessage(tr("Saved tournament to file"));
}

void HubWindow::readTournament(const QString &fileName) {
    mFileName = fileName;
    if (!mStoreManager.read(fileName)) {
        QMessageBox::warning(this, tr("Unable to open file"), tr("The selected file could not be opened."));
        return;
    }

    statusBar()->showMessage(tr("Opened tournament from file"));
    addToRecentFiles(fileName);
}

void HubWindow::newTournament() {
    if (mStoreManager.isDirty()) {
        auto reply = QMessageBox::question(this, tr("Unsaved changes"), tr("The current tournament has unsaved changes. Would you still like to continue?"), QMessageBox::Yes | QMessageBox::Cancel);
        if (reply == QMessageBox::Cancel)
            return;
    }

    mFileName = "";
    mStoreManager.resetTournament();
}

void HubWindow::openTournament() {
    if (mStoreManager.isDirty()) {
        auto reply = QMessageBox::question(this, tr("Unsaved changes"), tr("The current tournament has unsaved changes. Would you still like to continue?"), QMessageBox::Yes | QMessageBox::Cancel);
        if (reply == QMessageBox::Cancel)
            return;
    }
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Tournament"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("Tournament Files (*.judo);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    readTournament(fileName);
}

void HubWindow::quit() {
    if (mStoreManager.isDirty()) {
        auto reply = QMessageBox::question(this, tr("Unsaved changes"), tr("The current tournament has unsaved changes. Would you still like to exit without saving?"), QMessageBox::Yes | QMessageBox::Cancel);
        if (reply == QMessageBox::Cancel)
            return;
    }

    QCoreApplication::exit();
}

void HubWindow::saveTournament() {
    if (mFileName.isEmpty())
        saveAsTournament();
    else
        writeTournament();
}

void HubWindow::saveAsTournament() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Tournament"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("Tournament Files (*.judo);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".judo"))
        fileName += ".judo";

    mFileName = fileName;
    writeTournament();
}

void HubWindow::showAboutDialog() {
    QMessageBox::about(this, tr("JudoAssistant - About"), tr("JudoAssistant - Version %1").arg(QString::fromStdString(ApplicationVersion::current().toString())));
}

void HubWindow::openImportPlayers() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import players"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("Comma-separated (CSV) Files (*.csv);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    CSVReader reader(fileName);
    if (!reader.isOpen()) {
        QMessageBox::warning(this, tr("Unable to open file"), tr("The given file could not be opened."));
        return;
    }


    ImportPlayersCSVDialog dialog(mStoreManager, &reader);

    dialog.exec();
}

void HubWindow::showTournamentPreferences() {
    TournamentPreferencesDialog dialog(mStoreManager);

    dialog.exec();
}

void HubWindow::showJudoAssistantPreferences() {
    JudoassistantPreferencesDialog dialog(mStoreManager);

    dialog.exec();

    // Update autosave frequency if changed after closing dialog
    const QSettings& settings = mStoreManager.getSettings();
    bool autosave = settings.value(Constants::Settings::AUTOSAVE_ENABLED, true).toBool();

    std::optional<std::chrono::seconds> autosaveFrequency;
    if (autosave)
        autosaveFrequency = std::chrono::minutes(settings.value(Constants::Settings::AUTOSAVE_FREQUENCY, 5).toInt());

    if (autosaveFrequency != mAutosaveFrequency) {
        log_debug().field("freq", settings.value(Constants::Settings::AUTOSAVE_FREQUENCY, 5).toInt()).msg("New frequency");
        mAutosaveFrequency = autosaveFrequency;
        mAutosaveTimer.start(*mAutosaveFrequency);
    }
}

void HubWindow::autosaveTimerHit() {
    if (mFileName.isEmpty())
        return;

    if (!mStoreManager.isDirty())
        return;

    QSettings& settings = mStoreManager.getSettings();
    unsigned int backupAmount = 0;
    if (settings.value(Constants::Settings::BACKUP_ENABLED, false).toBool())
        backupAmount = settings.value(Constants::Settings::BACKUP_AMOUNT, 2).toInt();

    if (!mStoreManager.write(mFileName, backupAmount))
        QMessageBox::warning(this, tr("Unable to autosave"), tr("JudoAssistant was unable to auto-save to the opened tournament file."));
    else
        statusBar()->showMessage(tr("Auto-saved tournament to file"));
}

void HubWindow::createRecentFileAction(const QString &file) {
    const QFileInfo fileInfo(file);

    if (!fileInfo.exists())
        return;

    QAction *action = new QAction(this);
    action->setText(fileInfo.fileName());
    action->setStatusTip(tr("Open the tournament \"%1\"").arg(fileInfo.filePath()));

    connect(action, &QAction::triggered, [=]() {
        this->readTournament(fileInfo.filePath());
    });

    mRecentFilesMenu->addAction(action);
}

void HubWindow::refreshRecentFilesMenu() {
    const QSettings &settings = mStoreManager.getSettings();
    const auto files = settings.value(Constants::Settings::RECENT_FILES).toStringList();

    mRecentFilesMenu->clear();
    for (int i = 0; i < files.size(); ++i) {
        createRecentFileAction(files[i]);
    }

    mRecentFilesMenu->setDisabled(mRecentFilesMenu->isEmpty());
}

void HubWindow::addToRecentFiles(const QString &file) {
    static constexpr int MAX_SIZE = 5;

    QSettings &settings = mStoreManager.getSettings();
    QStringList files = settings.value(Constants::Settings::RECENT_FILES).toStringList();

    files.removeOne(file); // Remove if already exists
    while (files.size() >= MAX_SIZE)
        files.pop_back();
    files.push_front(file);
    settings.setValue(Constants::Settings::RECENT_FILES, files);

    refreshRecentFilesMenu();
}

