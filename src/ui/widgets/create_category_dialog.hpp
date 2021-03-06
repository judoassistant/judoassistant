#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>

#include "core/core.hpp"

class StoreManager;
class PlayerId;

class CreateCategoryDialog : public QDialog {
    Q_OBJECT
public:
    CreateCategoryDialog(StoreManager & storeManager, const std::vector<PlayerId> &playerIds, QWidget *parent);

public slots:
    void acceptClick();
    void cancelClick();
private:
    StoreManager & mStoreManager;

    const std::vector<PlayerId> &mPlayerIds;
    QLineEdit *mNameContent;
    QComboBox *mRulesetContent;
    QComboBox *mDrawSystemContent;
};
