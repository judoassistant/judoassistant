#pragma once

#include "core.hpp"
#include "store_managers/store_manager.hpp"

#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>

class StoreManager;

class AutoAddCategoryDialog : public QDialog {
    Q_OBJECT
public:
    AutoAddCategoryDialog(StoreManager & storeManager, const std::vector<PlayerId> &playerIds, QWidget *parent = nullptr);

public slots:
    void acceptClick();
    void cancelClick();
private:
    StoreManager & mStoreManager;
    std::vector<PlayerId> mPlayerIds;
    QLineEdit *mBaseNameContent;
    QDoubleSpinBox *mMaxDifferenceContent;
    QSpinBox *mMaxSizeContent;
};
