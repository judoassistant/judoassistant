#pragma once

#include <stack>
#include <QMetaObject>
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>

#include "core/core.hpp"
#include "core/id.hpp"

class StoreManager;

class EditPlayerWidget : public QWidget {
    Q_OBJECT
public:
    EditPlayerWidget(StoreManager & storeManager, QWidget *parent);
    void setPlayer(std::optional<PlayerId> id);

private:
    void firstNameEdited();
    void lastNameEdited();
    void ageEdited();
    void rankEdited();
    void clubEdited();
    void weightEdited();
    void countryEdited();
    void sexEdited();
    void playersChanged(std::vector<PlayerId> ids);
    void tournamentAboutToBeReset();
    void tournamentReset();

    StoreManager & mStoreManager;
    std::optional<PlayerId> mPlayerId;
    QLineEdit *mFirstNameContent;
    QLineEdit *mLastNameContent;
    QLineEdit *mAgeContent;
    QComboBox *mRankContent;
    QLineEdit *mClubContent;
    QLineEdit *mWeightContent;
    QComboBox *mCountryContent;
    QComboBox *mSexContent;

    std::stack<QMetaObject::Connection> mConnections;
};
