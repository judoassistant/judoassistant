#pragma once

#include "core.hpp"
#include "store_handlers/qstore_handler.hpp"

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>

class QStoreHandler;

class CreateCategoryDialog : public QDialog {
    Q_OBJECT
public:
    CreateCategoryDialog(QStoreHandler & storeHandler, QWidget *parent);

public slots:
    void acceptClick();
    void cancelClick();
private:
    QStoreHandler & mStoreHandler;
    QLineEdit *mNameContent;
    QComboBox *mRulesetContent;
    QComboBox *mDrawStrategyContent;
};
