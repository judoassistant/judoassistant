#pragma once

#include <QMainWindow>
#include <QUrl>
#include "stores/tournament_store.hpp"
#include "rulesets/twentyeighteen_ruleset.hpp"
#include "core.hpp"

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
    void openTournament();
    void saveTournament();
    void saveAsTournament();
    void openHomePage();
    void openManual();
    void openReportIssue();
private:
    void writeTournament();
    std::unique_ptr<TournamentStore> mTournament;
    QString mFileName;
};

