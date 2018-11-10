#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFormLayout>

#include "widgets/validators/optional_validator.hpp"
#include "widgets/edit_category_widget.hpp"
#include "actions/category_actions.hpp"
#include "rulesets/rulesets.hpp"
#include "draw_systems/draw_systems.hpp"

EditCategoryWidget::EditCategoryWidget(QStoreHandler & storeHandler, QWidget *parent)
    : QWidget(parent)
    , mStoreHandler(storeHandler)
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

    setCategory(std::nullopt);
    setLayout(formLayout);

    connect(&(mStoreHandler.getTournament()), &QTournamentStore::categoriesChanged, this, &EditCategoryWidget::categoriesChanged);
    connect(&(mStoreHandler.getTournament()), &QTournamentStore::matchesReset, this, &EditCategoryWidget::resetMatches);
    connect(&(mStoreHandler.getTournament()), &QTournamentStore::playersAddedToCategory, this, &EditCategoryWidget::changePlayerCount);
    connect(&(mStoreHandler.getTournament()), &QTournamentStore::playersErasedFromCategory, this, &EditCategoryWidget::changePlayerCount);
    connect(&mStoreHandler, &QStoreHandler::tournamentReset, this, &EditCategoryWidget::tournamentReset);
}

void EditCategoryWidget::tournamentReset() {
    connect(&(mStoreHandler.getTournament()), &QTournamentStore::categoriesChanged, this, &EditCategoryWidget::categoriesChanged);
    connect(&(mStoreHandler.getTournament()), &QTournamentStore::matchesReset, this, &EditCategoryWidget::resetMatches);
    connect(&(mStoreHandler.getTournament()), &QTournamentStore::playersAddedToCategory, this, &EditCategoryWidget::changePlayerCount);
    connect(&(mStoreHandler.getTournament()), &QTournamentStore::playersErasedFromCategory, this, &EditCategoryWidget::changePlayerCount);

    setCategory(std::nullopt);
}

void EditCategoryWidget::setCategory(std::optional<CategoryId> id) {
    mCategoryId = id;

    if (!id) {
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
        const CategoryStore & category = mStoreHandler.getTournament().getCategory(*id);
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
    if (mCategoryId != categoryId)
        return;

    const CategoryStore & category = mStoreHandler.getTournament().getCategory(*mCategoryId);
    mMatchCountContent->setText(QString::number(category.getMatches().size()));
}

void EditCategoryWidget::changePlayerCount(CategoryId categoryId, std::vector<PlayerId> playerIds) {
    if (mCategoryId != categoryId)
        return;

    const CategoryStore & category = mStoreHandler.getTournament().getCategory(*mCategoryId);
    mPlayerCountContent->setText(QString::number(category.getPlayers().size()));
}

void EditCategoryWidget::categoriesChanged(std::vector<CategoryId> ids) {
    if (!mCategoryId || std::find(ids.begin(), ids.end(), *mCategoryId) == ids.end())
        return;

    TournamentStore &tournament = mStoreHandler.getTournament();
    CategoryStore &category = tournament.getCategory(*mCategoryId);

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
    if (!mCategoryId)
        return;

    TournamentStore &tournament = mStoreHandler.getTournament();
    CategoryStore &category = tournament.getCategory(*mCategoryId);

    std::string newValue = mNameContent->text().toStdString();
    std::string oldValue = category.getName();
    if (newValue == oldValue) return;

    mStoreHandler.dispatch(std::make_unique<ChangeCategoryNameAction>(*mCategoryId, newValue));
}

void EditCategoryWidget::rulesetEdited() {
    if (!mCategoryId)
        return;

    TournamentStore &tournament = mStoreHandler.getTournament();
    CategoryStore &category = tournament.getCategory(*mCategoryId);

    if (mRulesetContent->currentIndex() == mRulesetContent->findText(QString::fromStdString(category.getRuleset().getName())))
        return

    mStoreHandler.dispatch(std::make_unique<ChangeCategoryRulesetAction>(*mCategoryId, mRulesetContent->currentIndex()));
}

void EditCategoryWidget::drawSystemEdited() {
    if (!mCategoryId)
        return;

    TournamentStore &tournament = mStoreHandler.getTournament();
    CategoryStore &category = tournament.getCategory(*mCategoryId);

    if (mDrawSystemContent->currentIndex() == mDrawSystemContent->findText(QString::fromStdString(category.getDrawSystem().getName())))
        return

    mStoreHandler.dispatch(std::make_unique<ChangeCategoryDrawSystemAction>(*mCategoryId, mDrawSystemContent->currentIndex()));
}

