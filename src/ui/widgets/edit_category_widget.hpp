#pragma once

#include <stack>
#include <unordered_set>
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QMetaObject>

#include "core/core.hpp"
#include "core/id.hpp"

class StoreManager;

class EditCategoryWidget : public QWidget {
    Q_OBJECT
public:
    EditCategoryWidget(StoreManager & storeManager, QWidget *parent);
    void setCategories(const std::vector<CategoryId> &categoryIds);

private:
    const QString MULTIPLE_TEXT = QObject::tr("-- multiple --");

    QString getDrawSystemText();
    QString getRulesetText();

    void changeCategories(std::vector<CategoryId> ids);
    void updateName();
    void updateRuleset();
    void updateDrawDisabled();
    void updateDrawSystem();

    void resetMatches(const std::vector<CategoryId> &categoryIds);
    void updateMatchCount();

    void changePlayerCount(CategoryId categoryId, std::vector<PlayerId> playerIds);
    void updatePlayerCount();

    void beginResetTournament();
    void endResetTournament();

    void editName();
    void editRuleset();
    void editDrawSystem();
    void editDrawDisabled(int state);

    StoreManager & mStoreManager;
    std::unordered_set<CategoryId> mCategoryIds;
    QLineEdit *mNameContent;
    QComboBox *mRulesetContent;
    QComboBox *mDrawSystemContent;
    QCheckBox *mDrawDisabledContent;
    QLabel *mPlayerCountContent;
    QLabel *mMatchCountContent;
    std::stack<QMetaObject::Connection> mConnections;
};
