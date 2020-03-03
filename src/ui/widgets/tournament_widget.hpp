#pragma once

#include <stack>
#include <QMetaObject>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDateEdit>

#include "core/core.hpp"

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
    void updateTournamentLocation();
    void updateTournamentDate(const QDate &date);
    void updateTatamiCount(int count);
    void tatamiCountChanged();

    MasterStoreManager &mStoreManager;
    QLineEdit *mNameContent;
    QLineEdit *mLocationContent;
    QDateEdit *mDateContent;
    QComboBox *mLanguageContent;
    QSpinBox *mTatamiCountContent;
    std::stack<QMetaObject::Connection> mConnections;
};

