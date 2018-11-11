#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>

#include "core.hpp"
#include "store_managers/store_manager.hpp"

class TournamentWidget : public QWidget {
    Q_OBJECT

public slots:
    void tournamentChanged();
    void updateTournamentName();
public:
    TournamentWidget(StoreManager &storeManager);
private:
    StoreManager &mStoreManager;
    QLineEdit *mNameContent;
    QComboBox *mLanguageContent;
};

