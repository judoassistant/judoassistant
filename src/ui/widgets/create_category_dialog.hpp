#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>

#include "core/core.hpp"

class StoreManager;

class CreateCategoryDialog : public QDialog {
    Q_OBJECT
public:
    CreateCategoryDialog(StoreManager & storeManager, QWidget *parent);

public slots:
    void acceptClick();
    void cancelClick();
private:
    StoreManager & mStoreManager;
    QLineEdit *mNameContent;
    QComboBox *mRulesetContent;
    QComboBox *mDrawSystemContent;
};
