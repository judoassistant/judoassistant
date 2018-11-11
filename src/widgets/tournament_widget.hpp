#pragma once

#include <QMainWindow>
#include <QMetaObject>
#include <QLineEdit>
#include <QComboBox>

#include "core.hpp"
#include "store_managers/store_manager.hpp"

class TournamentWidget : public QWidget {
    Q_OBJECT

public slots:
    void tournamentAboutToBeReset();
    void tournamentReset();
    void tournamentChanged();
    void updateTournamentName();
public:
    TournamentWidget(StoreManager &storeManager);
private:
    StoreManager &mStoreManager;
    QLineEdit *mNameContent;
    QComboBox *mLanguageContent;
    std::stack<QMetaObject::Connection> mConnections;
};

