#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFormLayout>

#include "core/actions/change_categories_draw_system_action.hpp"
#include "core/actions/change_categories_name_action.hpp"
#include "core/actions/change_categories_ruleset_action.hpp"
#include "core/actions/draw_categories_action.hpp"
#include "core/actions/set_categories_draw_disabled.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/validators/optional_validator.hpp"
#include "ui/widgets/edit_category_widget.hpp"

QString EditCategoryWidget::getDrawSystemText() {
    assert(!mCategoryIds.empty());
    QString res;

    for (auto categoryId : mCategoryIds) {
        const CategoryStore & category = mStoreManager.getTournament().getCategory(categoryId);
        auto name = QString::fromStdString(category.getDrawSystem().getName());
        if (res.isEmpty())
            res = name;
        else if (res != name)
            return MULTIPLE_TEXT;
    }

    return res;
}

QString EditCategoryWidget::getRulesetText() {
    assert(!mCategoryIds.empty());
    QString res;

    for (auto categoryId : mCategoryIds) {
        const CategoryStore & category = mStoreManager.getTournament().getCategory(categoryId);
        auto name = QString::fromStdString(category.getRuleset().getName());
        if (res.isEmpty())
            res = name;
        else if (res != name)
            return MULTIPLE_TEXT;
    }

    return res;
}

EditCategoryWidget::EditCategoryWidget(StoreManager & storeManager, QWidget *parent)
    : QWidget(parent)
    , mStoreManager(storeManager)
{
    mNameContent = new QLineEdit;
    connect(mNameContent, &QLineEdit::editingFinished, this, &EditCategoryWidget::editName);

    mRulesetContent = new QComboBox;
    connect(mRulesetContent, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {editRuleset();});
    for (const auto & ruleset : Ruleset::getRulesets())
        mRulesetContent->addItem(QString::fromStdString(ruleset->getName()));

    mDrawSystemContent = new QComboBox;
    connect(mDrawSystemContent, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {editDrawSystem();});
    for (const auto & system : DrawSystem::getDrawSystems())
        mDrawSystemContent->addItem(QString::fromStdString(system->getName()));

    mDrawDisabledContent = new QCheckBox;
    mDrawDisabledContent->setTristate(true);
    connect(mDrawDisabledContent, &QCheckBox::clicked, this, &EditCategoryWidget::editDrawDisabled);

    mPlayerCountContent = new QLabel("");
    mMatchCountContent = new QLabel("");

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("Name"), mNameContent);
    formLayout->addRow(tr("Ruleset"), mRulesetContent);
    formLayout->addRow(tr("Draw System"), mDrawSystemContent);
    formLayout->addRow(tr("Disable Draw"), mDrawDisabledContent);
    formLayout->addRow(tr("Player Count"), mPlayerCountContent);
    formLayout->addRow(tr("Match Count"), mMatchCountContent);

    setCategories({});
    setLayout(formLayout);

    connect(&mStoreManager, &StoreManager::tournamentReset, this, &EditCategoryWidget::endResetTournament);
    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &EditCategoryWidget::beginResetTournament);

    endResetTournament();
}

void EditCategoryWidget::beginResetTournament() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }

    setCategories({});
}

void EditCategoryWidget::endResetTournament() {
    connect(&(mStoreManager.getTournament()), &QTournamentStore::categoriesChanged, this, &EditCategoryWidget::changeCategories);
    connect(&(mStoreManager.getTournament()), &QTournamentStore::matchesReset, this, &EditCategoryWidget::resetMatches);
    connect(&(mStoreManager.getTournament()), &QTournamentStore::playersAddedToCategory, this, &EditCategoryWidget::changePlayerCount);
    connect(&(mStoreManager.getTournament()), &QTournamentStore::playersErasedFromCategory, this, &EditCategoryWidget::changePlayerCount);
}

void EditCategoryWidget::setCategories(const std::vector<CategoryId> &categoryIds) {
    mCategoryIds.clear();
    mCategoryIds.insert(categoryIds.begin(), categoryIds.end());

    updateName();
    updateRuleset();
    updateDrawSystem();
    updateDrawDisabled();
    updatePlayerCount();
    updateMatchCount();
}

void EditCategoryWidget::resetMatches(const std::vector<CategoryId> &categoryIds) {
    for (auto categoryId : categoryIds) {
        if (mCategoryIds.find(categoryId) != mCategoryIds.end()) {
            updateMatchCount();
            return;
        }
    }
}

void EditCategoryWidget::changePlayerCount(CategoryId categoryId, std::vector<PlayerId> playerIds) {
    if (mCategoryIds.find(categoryId) == mCategoryIds.end())
        return;

    updatePlayerCount();
}

bool doIntersect(const std::vector<CategoryId> &a, const std::unordered_set<CategoryId> &b) {
    for (auto categoryId : a) {
        if (b.find(categoryId) != b.end())
            return true;
    }

    return false;
}

void EditCategoryWidget::changeCategories(std::vector<CategoryId> ids) {
    bool intersect = false;
    for (auto categoryId : ids) {
        if (mCategoryIds.find(categoryId) != mCategoryIds.end()) {
            intersect = true;
            break;
        }
    }

    if (!intersect)
        return;

    updateName();
    updateRuleset();
    updateDrawSystem();
    updateDrawDisabled();
}

void EditCategoryWidget::editDrawDisabled(int state) {
    if (mCategoryIds.empty())
        return;

    bool disabled = mDrawDisabledContent->isChecked();

    mStoreManager.dispatch(std::make_unique<SetCategoriesDrawDisabled>(std::vector<CategoryId>(mCategoryIds.begin(), mCategoryIds.end()), disabled));
}

void EditCategoryWidget::editName() {
    if (mCategoryIds.size() != 1)
        return;

    TournamentStore &tournament = mStoreManager.getTournament();
    CategoryStore &category = tournament.getCategory(*(mCategoryIds.begin()));

    std::string newValue = mNameContent->text().toStdString();
    std::string oldValue = category.getName();
    if (newValue == oldValue) return;

    mStoreManager.dispatch(std::make_unique<ChangeCategoriesNameAction>(std::vector<CategoryId>(mCategoryIds.begin(), mCategoryIds.end()), newValue));
}

void EditCategoryWidget::editRuleset() {
    if (mCategoryIds.empty())
        return;

    auto text = getRulesetText();

    if (mRulesetContent->currentIndex() == mRulesetContent->findText(text))
        return;

    mStoreManager.dispatch(std::make_unique<ChangeCategoriesRulesetAction>(std::vector<CategoryId>(mCategoryIds.begin(), mCategoryIds.end()), mRulesetContent->currentIndex()));
}

void EditCategoryWidget::editDrawSystem() {
    if (mCategoryIds.empty())
        return;

    auto text = getDrawSystemText();

    if (mDrawSystemContent->currentIndex() == mDrawSystemContent->findText(text))
        return;

    mStoreManager.dispatch(std::make_unique<ChangeCategoriesDrawSystemAction>(std::vector<CategoryId>(mCategoryIds.begin(), mCategoryIds.end()), mDrawSystemContent->currentIndex()));
}

void EditCategoryWidget::updateDrawDisabled() {
    if (mCategoryIds.empty()) {
        mDrawDisabledContent->setEnabled(false);
        mDrawDisabledContent->setCheckState(Qt::Unchecked);
        return;
    }

    mDrawDisabledContent->setEnabled(true);
    TournamentStore &tournament = mStoreManager.getTournament();

    bool disabled;
    for (auto it = mCategoryIds.begin(); it != mCategoryIds.end(); ++it) {
        const auto &category = tournament.getCategory(*it);
        if (it == mCategoryIds.begin()) {
            disabled = category.isDrawDisabled();
        }
        else if (category.isDrawDisabled() != disabled) {
            // Multiple values
            mDrawDisabledContent->setCheckState(Qt::PartiallyChecked);
            return;
        }
    }

    mDrawDisabledContent->setCheckState(disabled ? Qt::Checked : Qt::Unchecked);
}

void EditCategoryWidget::updateName() {
    if (mCategoryIds.size() != 1) {
        mNameContent->clear();
        mNameContent->setEnabled(false);
        return;
    }

    TournamentStore &tournament = mStoreManager.getTournament();
    CategoryStore &category = tournament.getCategory(*(mCategoryIds.begin()));

    mNameContent->setEnabled(true);
    mNameContent->setText(QString::fromStdString(category.getName()));
}

void EditCategoryWidget::updateRuleset() {
    if (mCategoryIds.empty()) {
        mRulesetContent->setCurrentIndex(0);
        mRulesetContent->setEnabled(false);
        return;
    }

    auto rulesetText = getRulesetText();
    if (rulesetText == MULTIPLE_TEXT) {
        if (static_cast<size_t>(mRulesetContent->count()) == Ruleset::getRulesets().size()) {
            mRulesetContent->addItem(MULTIPLE_TEXT);
            mRulesetContent->setItemData(mRulesetContent->count() - 1, QBrush(Qt::gray), Qt::ForegroundRole);
        }
    }
    else {
        if (static_cast<size_t>(mRulesetContent->count()) != Ruleset::getRulesets().size())
            mRulesetContent->removeItem(mRulesetContent->count() - 1);
    }

    mRulesetContent->setCurrentText(rulesetText);
    mRulesetContent->setEnabled(true);
}

void EditCategoryWidget::updateDrawSystem() {
    if (mCategoryIds.empty()) {
        mDrawSystemContent->setCurrentIndex(0);
        mDrawSystemContent->setEnabled(false);
        return;
    }

    auto drawSystemText = getDrawSystemText();
    if (drawSystemText == MULTIPLE_TEXT) {
        if (static_cast<size_t>(mDrawSystemContent->count()) == DrawSystem::getDrawSystems().size()) {
            mDrawSystemContent->addItem(MULTIPLE_TEXT);
            mDrawSystemContent->setItemData(mDrawSystemContent->count() - 1, QBrush(Qt::gray), Qt::ForegroundRole);
        }
    }
    else {
        if (static_cast<size_t>(mDrawSystemContent->count()) != DrawSystem::getDrawSystems().size())
            mDrawSystemContent->removeItem(mDrawSystemContent->count() - 1);
    }

    mDrawSystemContent->setCurrentText(drawSystemText);
    mDrawSystemContent->setEnabled(true);
}

void EditCategoryWidget::updatePlayerCount() {
    if (mCategoryIds.empty()) {
        mPlayerCountContent->setText("");
        return;
    }

    const auto & tournament = mStoreManager.getTournament();
    std::unordered_set<PlayerId> players;

    for (auto categoryId : mCategoryIds) {
        const auto &category = tournament.getCategory(categoryId);
        players.insert(category.getPlayers().begin(), category.getPlayers().end());
    }

    mPlayerCountContent->setText(QString::number(players.size()));
}

void EditCategoryWidget::updateMatchCount() {
    if (mCategoryIds.empty()) {
        mMatchCountContent->setText("");
        return;
    }

    const auto & tournament = mStoreManager.getTournament();
    size_t matchCount = 0;

    for (auto categoryId : mCategoryIds) {
        matchCount += tournament.getCategory(categoryId).getMatches().size();
    }

    mMatchCountContent->setText(QString::number(matchCount));
}

