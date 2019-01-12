#pragma once

#include <stack>
#include <QMetaObject>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>

#include "core.hpp"

class StoreManager;

class TournamentWidget : public QWidget {
    Q_OBJECT

public:
    TournamentWidget(StoreManager &storeManager);

private:
    QWidget* basicInformationSection();
    QWidget* webSection();

    void tournamentAboutToBeReset();
    void tournamentReset();
    void tournamentChanged();
    void updateTournamentName();
    void updateTatamiCount(int count);
    void tatamiCountChanged();

    StoreManager &mStoreManager;
    QLineEdit *mNameContent;
    QComboBox *mLanguageContent;
    QSpinBox *mTatamiCountContent;
    std::stack<QMetaObject::Connection> mConnections;
};

