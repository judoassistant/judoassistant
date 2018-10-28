#pragma once

#include "core.hpp"
#include "store_handlers/qstore_handler.hpp"

#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>

class QStoreHandler;

class AutoAddCategoryDialog : public QDialog {
    Q_OBJECT
public:
    AutoAddCategoryDialog(QStoreHandler & storeHandler, const std::vector<PlayerId> &playerIds, QWidget *parent = nullptr);

public slots:
    void acceptClick();
    void cancelClick();
private:
    QStoreHandler & mStoreHandler;
    std::vector<PlayerId> mPlayerIds;
    QLineEdit *mBaseNameContent;
    QDoubleSpinBox *mMaxDifferenceContent;
    QSpinBox *mMaxSizeContent;
};
