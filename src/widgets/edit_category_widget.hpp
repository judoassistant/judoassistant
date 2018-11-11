#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>

#include "core.hpp"
#include "store_managers/store_manager.hpp"

class StoreManager;

class EditCategoryWidget : public QWidget {
    Q_OBJECT
public:
    EditCategoryWidget(StoreManager & storeManager, QWidget *parent);
    void setCategory(std::optional<CategoryId> id);

private:
    void categoriesChanged(std::vector<CategoryId> ids);
    void resetMatches(CategoryId categoryId);
    void changePlayerCount(CategoryId categoryId, std::vector<PlayerId> playerIds);
    void tournamentReset();
    void nameEdited();
    void rulesetEdited();
    void drawSystemEdited();

    StoreManager & mStoreManager;
    std::optional<CategoryId> mCategoryId;
    QLineEdit *mNameContent;
    QComboBox *mRulesetContent;
    QComboBox *mDrawSystemContent;
    QLabel *mPlayerCountContent;
    QLabel *mMatchCountContent;
};
