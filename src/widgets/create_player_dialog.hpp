#pragma once

#include "core.hpp"
#include "store_handlers/qstore_handler.hpp"

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QDoubleSpinBox>

class QStoreHandler;

class CreatePlayerDialog : public QDialog {
    Q_OBJECT
public:
    CreatePlayerDialog(QStoreHandler & storeHandler, QWidget *parent = nullptr);

public slots:
    void acceptClick();
    void cancelClick();
private:
    QStoreHandler & mStoreHandler;
    QLineEdit *mFirstNameContent;
    QLineEdit *mLastNameContent;
    QLineEdit *mAgeContent;
    QComboBox *mRankContent;
    QLineEdit *mClubContent;
    QLineEdit *mWeightContent;
    QComboBox *mCountryContent;
};
