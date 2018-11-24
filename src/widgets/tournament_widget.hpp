#pragma once

#include <stack>
#include <QMetaObject>
#include <QLineEdit>
#include <QComboBox>

#include "core.hpp"

class StoreManager;

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

