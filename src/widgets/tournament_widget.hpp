#pragma once

#include <stack>
#include <QMetaObject>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>

#include "core.hpp"

class MasterStoreManager;

class TournamentWidget : public QWidget {
    Q_OBJECT

public:
    TournamentWidget(MasterStoreManager &storeManager);

private:
    QWidget* basicInformationSection();

    void tournamentAboutToBeReset();
    void tournamentReset();
    void tournamentChanged();
    void updateTournamentName();
    void updateTatamiCount(int count);
    void tatamiCountChanged();

    MasterStoreManager &mStoreManager;
    QLineEdit *mNameContent;
    QComboBox *mLanguageContent;
    QSpinBox *mTatamiCountContent;
    std::stack<QMetaObject::Connection> mConnections;
};

