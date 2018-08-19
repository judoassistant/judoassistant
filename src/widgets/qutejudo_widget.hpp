#pragma once

#include <QMainWindow>

class QutejudoWidget : public QMainWindow {
    Q_OBJECT

public:
    QutejudoWidget();

    void createTournamentMenu();
    void createViewMenu();
    void createPreferencesMenu();
    void createHelpMenu();
private slots:
    void newFile();
private:
};

