#pragma once

#include "core/core.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpinBox>

class StoreManager;

class CreatePlayerDialog : public QDialog {
    Q_OBJECT
public:
    CreatePlayerDialog(StoreManager & storeManager, QWidget *parent = nullptr);

public slots:
    void acceptClick();
    void cancelClick();
private:
    StoreManager & mStoreManager;
    QLineEdit *mFirstNameContent;
    QLineEdit *mLastNameContent;
    QLineEdit *mAgeContent;
    QComboBox *mRankContent;
    QLineEdit *mClubContent;
    QLineEdit *mWeightContent;
    QComboBox *mCountryContent;
    QComboBox *mSexContent;
    QCheckBox *mBlueJudogiHintContent;
};

