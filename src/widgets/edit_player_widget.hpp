#pragma once

#include "core.hpp"
#include "store_handlers/qstore_handler.hpp"

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>

class QStoreHandler;

class EditPlayerWidget : public QWidget {
    Q_OBJECT
public:
    EditPlayerWidget(QStoreHandler & storeHandler, QWidget *parent);
    void setPlayer(std::optional<PlayerId> id);

private:
    void firstNameEdited();
    void lastNameEdited();
    void ageEdited();
    void rankEdited();
    void clubEdited();
    void weightEdited();
    void countryEdited();
    void playerChanged();
    void playersChanged(std::vector<PlayerId> ids);
    void tournamentReset();

    QStoreHandler & mStoreHandler;
    std::optional<PlayerId> mPlayerId;
    QLineEdit *mFirstNameContent;
    QLineEdit *mLastNameContent;
    QLineEdit *mAgeContent;
    QComboBox *mRankContent;
    QLineEdit *mClubContent;
    QLineEdit *mWeightContent;
    QComboBox *mCountryContent;
};
