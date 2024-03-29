#pragma once

#include <optional>
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
#include "core/draw_systems/draw_system_identifier.hpp"
#include "core/rulesets/ruleset_identifier.hpp"

class StoreManager;
class ConfirmableAction;

class EditCategoryWidget : public QWidget {
    Q_OBJECT
public:
    EditCategoryWidget(StoreManager & storeManager, QWidget *parent);
    void setCategories(const std::vector<CategoryId> &categoryIds);

private:
    bool userConfirmsAction(const ConfirmableAction &action) const;

    const QString MULTIPLE_TEXT = QObject::tr("-- multiple --");

    std::optional<DrawSystemIdentifier> getDrawSystemIdentifier();
    std::optional<RulesetIdentifier> getRulesetIdentifier();
    std::optional<std::string> getNameString();

    void changeCategories(std::vector<CategoryId> ids);
    void updateName();
    void updateRuleset();
    void updateMatchesHidden();
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
    void editMatchesHidden(int state);

    StoreManager & mStoreManager;
    std::unordered_set<CategoryId> mCategoryIds;
    QLineEdit *mNameContent;
    QComboBox *mRulesetContent;
    QComboBox *mDrawSystemContent;
    QCheckBox *mMatchesHiddenContent;
    QLabel *mPlayerCountContent;
    QLabel *mMatchCountContent;
    std::stack<QMetaObject::Connection> mConnections;
};
