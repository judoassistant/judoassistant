#pragma once

#include "core.hpp"
#include "store_handlers/qstore_handler.hpp"

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>

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
    QLineEdit *mFirstNameContent;
    QLineEdit *mLastNameContent;
    QSpinBox *mAgeContent;
};
