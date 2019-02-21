#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFormLayout>

#include "core/actions/category_actions.hpp"
#include "core/draw_systems/draw_systems.hpp"
#include "core/rulesets/rulesets.hpp"
#include "core/stores/category_store.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/validators/optional_validator.hpp"
#include "ui/widgets/edit_category_widget.hpp"

EditCategoryWidget::EditCategoryWidget(StoreManager & storeManager, QWidget *parent)
    : QWidget(parent)
    , mStoreManager(storeManager)
{
    mNameContent = new QLineEdit;
    connect(mNameContent, &QLineEdit::editingFinished, this, &EditCategoryWidget::nameEdited);

    mRulesetContent = new QComboBox;
    connect(mRulesetContent, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {rulesetEdited();});
    for (const auto & ruleset : Rulesets::getRulesets())
        mRulesetContent->addItem(QString::fromStdString(ruleset->getName()));

    mDrawSystemContent = new QComboBox;
    connect(mDrawSystemContent, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {drawSystemEdited();});
    for (const auto & system : DrawSystems::getDrawSystems())
        mDrawSystemContent->addItem(QString::fromStdString(system->getName()));

    mPlayerCountContent = new QLabel("");
    mMatchCountContent = new QLabel("");

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("Name"), mNameContent);
    formLayout->addRow(tr("Ruleset"), mRulesetContent);
    formLayout->addRow(tr("Draw System"), mDrawSystemContent);
    formLayout->addRow(tr("Player Count"), mPlayerCountContent);
    formLayout->addRow(tr("Match Count"), mMatchCountContent);

    setCategories({});
    setLayout(formLayout);

    connect(&(mStoreManager.getTournament()), &QTournamentStore::categoriesChanged, this, &EditCategoryWidget::categoriesChanged);
    connect(&(mStoreManager.getTournament()), &QTournamentStore::matchesReset, this, &EditCategoryWidget::resetMatches);
    connect(&(mStoreManager.getTournament()), &QTournamentStore::playersAddedToCategory, this, &EditCategoryWidget::changePlayerCount);
    connect(&(mStoreManager.getTournament()), &QTournamentStore::playersErasedFromCategory, this, &EditCategoryWidget::changePlayerCount);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &EditCategoryWidget::tournamentReset);
}

void EditCategoryWidget::tournamentAboutToBeReset() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }

    setCategories({});
}

void EditCategoryWidget::tournamentReset() {
    mConnections.push(connect(&(mStoreManager.getTournament()), &QTournamentStore::categoriesChanged, this, &EditCategoryWidget::categoriesChanged));
    mConnections.push(connect(&(mStoreManager.getTournament()), &QTournamentStore::matchesReset, this, &EditCategoryWidget::resetMatches));
    mConnections.push(connect(&(mStoreManager.getTournament()), &QTournamentStore::playersAddedToCategory, this, &EditCategoryWidget::changePlayerCount));
    mConnections.push(connect(&(mStoreManager.getTournament()), &QTournamentStore::playersErasedFromCategory, this, &EditCategoryWidget::changePlayerCount));
}

void EditCategoryWidget::setCategories(const std::vector<CategoryId> &categoryIds) {
    mCategoryIds = categoryIds;

    if (categoryIds.size() != 1) {
        mNameContent->clear();
        mNameContent->setEnabled(false);

        mRulesetContent->setCurrentIndex(0);
        mRulesetContent->setEnabled(false);

        mDrawSystemContent->setCurrentIndex(0);
        mDrawSystemContent->setEnabled(false);

        mPlayerCountContent->setText("");

        mMatchCountContent->setText("");
    }
    else {
        auto id = categoryIds.front();
        const CategoryStore & category = mStoreManager.getTournament().getCategory(id);
        mNameContent->setEnabled(true);
        mNameContent->setText(QString::fromStdString(category.getName()));

        mRulesetContent->setCurrentText(QString::fromStdString(category.getRuleset().getName()));
        mRulesetContent->setEnabled(true);

        mDrawSystemContent->setCurrentText(QString::fromStdString(category.getDrawSystem().getName()));
        mDrawSystemContent->setEnabled(true);

        mPlayerCountContent->setText(QString::number(category.getPlayers().size()));
        mMatchCountContent->setText(QString::number(category.getMatches().size()));
    }
}

void EditCategoryWidget::resetMatches(CategoryId categoryId) {
    if (mCategoryIds.size() != 1)
        return;
    if (mCategoryIds.front() != categoryId)
        return;

    const CategoryStore & category = mStoreManager.getTournament().getCategory(mCategoryIds.front());
    mMatchCountContent->setText(QString::number(category.getMatches().size()));
}

void EditCategoryWidget::changePlayerCount(CategoryId categoryId, std::vector<PlayerId> playerIds) {
    if (mCategoryIds.size() != 1)
        return;
    if (mCategoryIds.front() != categoryId)
        return;

    const CategoryStore & category = mStoreManager.getTournament().getCategory(mCategoryIds.front());
    mPlayerCountContent->setText(QString::number(category.getPlayers().size()));
}

bool doIntersect(const std::vector<CategoryId> &a, const std::vector<CategoryId> &b) {
    std::unordered_set<CategoryId> aSet(a.begin(), b.end());

    for (auto categoryId : b) {
        if (aSet.find(categoryId) != aSet.end())
            return true;
    }

    return false;
}

void EditCategoryWidget::categoriesChanged(std::vector<CategoryId> ids) {
    if (mCategoryIds.size() != 1)
        return;

    if (!doIntersect(ids, mCategoryIds))
        return;

    TournamentStore &tournament = mStoreManager.getTournament();
    auto id = mCategoryIds.front();
    CategoryStore &category = tournament.getCategory(id);

    QString name = QString::fromStdString(category.getName());
    if (name != mNameContent->text())
        mNameContent->setText(name);

    int rulesetIndex = mRulesetContent->findText(QString::fromStdString(category.getRuleset().getName()));
    if (rulesetIndex != mRulesetContent->currentIndex())
        mRulesetContent->setCurrentIndex(rulesetIndex);

    int drawSystemIndex = mDrawSystemContent->findText(QString::fromStdString(category.getDrawSystem().getName()));
    if (drawSystemIndex != mDrawSystemContent->currentIndex())
        mDrawSystemContent->setCurrentIndex(drawSystemIndex);
}

void EditCategoryWidget::nameEdited() {
    if (mCategoryIds.size() != 1)
        return;

    TournamentStore &tournament = mStoreManager.getTournament();
    CategoryStore &category = tournament.getCategory(mCategoryIds.front());

    std::string newValue = mNameContent->text().toStdString();
    std::string oldValue = category.getName();
    if (newValue == oldValue) return;

    mStoreManager.dispatch(std::make_unique<ChangeCategoriesNameAction>(mCategoryIds, newValue));
}

void EditCategoryWidget::rulesetEdited() {
    if (mCategoryIds.size() != 1)
        return;

    TournamentStore &tournament = mStoreManager.getTournament();
    CategoryStore &category = tournament.getCategory(mCategoryIds.front());

    if (mRulesetContent->currentIndex() == mRulesetContent->findText(QString::fromStdString(category.getRuleset().getName())))
        return;

    mStoreManager.dispatch(std::make_unique<ChangeCategoriesRulesetAction>(mCategoryIds, mRulesetContent->currentIndex()));
}

void EditCategoryWidget::drawSystemEdited() {
    if (mCategoryIds.size() != 1)
        return;

    TournamentStore &tournament = mStoreManager.getTournament();
    CategoryStore &category = tournament.getCategory(mCategoryIds.front());

    if (mDrawSystemContent->currentIndex() == mDrawSystemContent->findText(QString::fromStdString(category.getDrawSystem().getName())))
        return;

    mStoreManager.dispatch(std::make_unique<ChangeCategoriesDrawSystemAction>(mCategoryIds, mDrawSystemContent->currentIndex()));
}

