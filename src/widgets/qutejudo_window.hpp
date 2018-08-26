#pragma once

#include <QMainWindow>
#include <QUrl>

#include "core.hpp"
#include "stores/qtournament_store.hpp"
#include "rulesets/twentyeighteen_ruleset.hpp"

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
    void readTournament();
    std::unique_ptr<QTournamentStore> mTournament;
    QString mFileName;
};

