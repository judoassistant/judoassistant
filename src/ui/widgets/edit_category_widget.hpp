#pragma once

#include <stack>
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QMetaObject>

#include "core/core.hpp"
#include "core/id.hpp"

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
    void tournamentAboutToBeReset();
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
    std::stack<QMetaObject::Connection> mConnections;
};
