#pragma once

#include <QMainWindow>
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
private:
    std::unique_ptr<TournamentStore> mTournament;
};

