#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QMetaObject>
#include <QWidget>
#include <stack>
#include <unordered_set>

#include "core/core.hpp"
#include "core/id.hpp"
#include "core/stores/player_store.hpp"

class StoreManager;

class EditPlayerWidget : public QWidget {
    Q_OBJECT
public:
    EditPlayerWidget(StoreManager & storeManager, QWidget *parent);
    void setPlayers(const std::vector<PlayerId> &playerIds);

private:
    const QString EMPTY_TEXT = QObject::tr("-- none --");
    const QString MULTIPLE_TEXT = QObject::tr("-- multiple --");

    void editFirstName();
    void editLastName();
    void editAge();
    void editRank();
    void editClub();
    void editWeight();
    void editCountry();
    void editSex();
    void editBlueJudogiHint();
    void changePlayers(std::vector<PlayerId> ids);
    void beginResetTournament();
    void endResetTournament();

    void updateFirstName();
    void updateLastName();
    void updateAge();
    void updateRank();
    void updateClub();
    void updateWeight();
    void updateCountry();
    void updateSex();
    void updateBlueJudogiHint();

    int getSexIndex();
    std::optional<std::string> getClubString();
    std::optional<std::string> getFirstNameString();
    std::optional<std::string> getLastNameString();
    std::optional<std::optional<PlayerAge>> getAgeValue();
    std::optional<std::optional<PlayerWeight>> getWeightValue();
    std::optional<bool> getBlueJudogiHintValue();
    int getRankIndex();
    int getCountryIndex();

    StoreManager & mStoreManager;
    std::unordered_set<PlayerId> mPlayerIds;
    QLineEdit *mFirstNameContent;
    QLineEdit *mLastNameContent;
    QLineEdit *mAgeContent;
    QComboBox *mRankContent;
    QLineEdit *mClubContent;
    QLineEdit *mWeightContent;
    QComboBox *mCountryContent;
    QComboBox *mSexContent;
    QCheckBox *mBlueJudogiHintContent;

    std::stack<QMetaObject::Connection> mConnections;
};
