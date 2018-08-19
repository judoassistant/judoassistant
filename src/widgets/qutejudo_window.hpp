#pragma once

#include <QMainWindow>
#include <QUrl>
#include "stores/tournament_store.hpp"

class QutejudoWindow : public QMainWindow {
    Q_OBJECT

public:
    QutejudoWindow();

    void createTournamentMenu();
    void createEditMenu();
    void createViewMenu();
    void createPreferencesMenu();
    void createHelpMenu();
private slots:
    void newTournament();
    void openHomePage();
    void openManual();
    void openReportIssue();
private:
    std::unique_ptr<TournamentStore> mTournament;
};

