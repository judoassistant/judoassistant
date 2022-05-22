#include <QLabel>
#include <QFormLayout>

#include "core/actions/change_categories_draw_system_action.hpp"
#include "core/actions/change_categories_name_action.hpp"
#include "core/actions/change_categories_ruleset_action.hpp"
#include "core/actions/draw_categories_action.hpp"
#include "core/actions/reset_matches_action.hpp"
#include "core/actions/set_categories_matches_hidden_action.hpp"
#include "core/actions/set_tatami_location_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/preferences_store.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/validators/optional_validator.hpp"
#include "ui/widgets/confirm_action_dialog.hpp"
#include "ui/widgets/edit_category_widget.hpp"

std::optional<DrawSystemIdentifier> EditCategoryWidget::getDrawSystemIdentifier() {
    std::optional<DrawSystemIdentifier> res;

    for (CategoryId categoryId : mCategoryIds) {
        const CategoryStore & category = mStoreManager.getTournament().getCategory(categoryId);
        auto drawSystem = category.getDrawSystem().getIdentifier();

        if (!res.has_value())
            res = drawSystem;
        else if (*res != drawSystem)
            return std::nullopt;
    }

    return res;
}

std::optional<RulesetIdentifier> EditCategoryWidget::getRulesetIdentifier() {
    std::optional<RulesetIdentifier> res;

    for (CategoryId categoryId : mCategoryIds) {
        const CategoryStore & category = mStoreManager.getTournament().getCategory(categoryId);
        auto ruleset = category.getRuleset().getIdentifier();

        if (!res.has_value())
            res = ruleset;
        else if (*res != ruleset)
            return std::nullopt;
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
    updateRuleset();
    connect(mRulesetContent, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {editRuleset();});

    mDrawSystemContent = new QComboBox;
    connect(mDrawSystemContent, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {editDrawSystem();});
    updateDrawSystem();

    mMatchesHiddenContent = new QCheckBox;
    mMatchesHiddenContent->setTristate(true);
    connect(mMatchesHiddenContent, &QCheckBox::clicked, this, &EditCategoryWidget::editMatchesHidden);

    mPlayerCountContent = new QLabel("");
    mMatchCountContent = new QLabel("");

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("Name"), mNameContent);
    formLayout->addRow(tr("Ruleset"), mRulesetContent);
    formLayout->addRow(tr("Draw System"), mDrawSystemContent);
    formLayout->addRow(tr("Hide Matches"), mMatchesHiddenContent);
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
    updateMatchesHidden();
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
    updateMatchesHidden();
}

void EditCategoryWidget::editMatchesHidden(int state) {
    if (mCategoryIds.empty())
        return;

    const bool hidden = mMatchesHiddenContent->isChecked();
    auto action = std::make_unique<SetCategoriesMatchesHiddenAction>(std::vector<CategoryId>(mCategoryIds.begin(), mCategoryIds.end()), hidden);

    if (!userConfirmsAction(*action)) {
        updateMatchesHidden(); // Revert content
        return;
    }

    mStoreManager.dispatch(std::move(action));
}

void EditCategoryWidget::editName() {
    if (mCategoryIds.empty())
        return;

    std::string newValue = mNameContent->text().toStdString();

    const TournamentStore &tournament = mStoreManager.getTournament();
    bool changed = false;
    for (auto categoryId : mCategoryIds) {
        const CategoryStore &category = tournament.getCategory(categoryId);
        std::string oldValue = category.getName();

        if (oldValue != newValue) {
            changed = true;
            break;
        }
    }

    if (!changed) return;

    mStoreManager.dispatch(std::make_unique<ChangeCategoriesNameAction>(std::vector<CategoryId>(mCategoryIds.begin(), mCategoryIds.end()), newValue));
}

void EditCategoryWidget::editRuleset() {
    if (mCategoryIds.empty())
        return;

    assert(static_cast<std::size_t>(mRulesetContent->currentIndex()) != Ruleset::getRulesets().size()); // the multiple field is selected

    RulesetIdentifier ruleset = mRulesetContent->currentData().value<RulesetIdentifier>();

    auto action = std::make_unique<ChangeCategoriesRulesetAction>(std::vector<CategoryId>(mCategoryIds.begin(), mCategoryIds.end()), ruleset);
    if (!userConfirmsAction(*action)) {
        updateRuleset(); // Revert content
        return;
    }

    mStoreManager.dispatch(std::move(action));
}

void EditCategoryWidget::editDrawSystem() {
    if (mCategoryIds.empty())
        return;

    assert(static_cast<std::size_t>(mDrawSystemContent->currentIndex()) != DrawSystem::getDrawSystems().size()); // the multiple field is selected

    DrawSystemIdentifier drawSystem = mDrawSystemContent->currentData().value<DrawSystemIdentifier>();
    auto action = std::make_unique<ChangeCategoriesDrawSystemAction>(std::vector<CategoryId>(mCategoryIds.begin(), mCategoryIds.end()), drawSystem);

    if (!userConfirmsAction(*action)) {
        updateDrawSystem(); // Revert content
        return;
    }

    mStoreManager.dispatch(std::move(action));
}

void EditCategoryWidget::updateMatchesHidden() {
    if (mCategoryIds.empty()) {
        mMatchesHiddenContent->setEnabled(false);
        mMatchesHiddenContent->setCheckState(Qt::Unchecked);
        return;
    }

    mMatchesHiddenContent->setEnabled(true);
    TournamentStore &tournament = mStoreManager.getTournament();

    bool hidden = false;
    for (auto it = mCategoryIds.begin(); it != mCategoryIds.end(); ++it) {
        const auto &category = tournament.getCategory(*it);
        if (it == mCategoryIds.begin()) {
            hidden = category.areMatchesHidden();
        }
        else if (category.areMatchesHidden() != hidden) {
            // Multiple values
            mMatchesHiddenContent->setCheckState(Qt::PartiallyChecked);
            return;
        }
    }

    mMatchesHiddenContent->setCheckState(hidden ? Qt::Checked : Qt::Unchecked);
}

void EditCategoryWidget::updateName() {
    mNameContent->clear();
    mNameContent->setEnabled(!mCategoryIds.empty());

    if (mCategoryIds.empty()) {
        mNameContent->setPlaceholderText("");
        return;
    }

    auto nameString = getNameString();

    if (!nameString.has_value()) {
        mNameContent->setPlaceholderText(MULTIPLE_TEXT);
        return;
    }

    mNameContent->setText(QString::fromStdString(*nameString));
    mNameContent->setPlaceholderText("");
}

void EditCategoryWidget::updateRuleset() {
    mRulesetContent->blockSignals(true);
    mRulesetContent->clear();

    auto currentIdentifier = getRulesetIdentifier();

    std::size_t i = 0;
    std::size_t index = 0;
    for (const auto & ruleset : Ruleset::getRulesets()) {
        auto name = QString::fromStdString(ruleset->getName());

        auto identifier = ruleset->getIdentifier();
        if (currentIdentifier == identifier)
            index = i;
        mRulesetContent->addItem(name, QVariant::fromValue(identifier));
        ++i;
    }

    if (!mCategoryIds.empty() && !currentIdentifier.has_value()) { // Multiple different rulesets
        mRulesetContent->addItem(MULTIPLE_TEXT);
        mRulesetContent->setItemData(mRulesetContent->count() - 1, QBrush(Qt::gray), Qt::ForegroundRole);
        index = i;
    }

    mRulesetContent->setCurrentIndex(index);
    mRulesetContent->setEnabled(!mCategoryIds.empty());
    mRulesetContent->blockSignals(false);
}

void EditCategoryWidget::updateDrawSystem() {
    mDrawSystemContent->blockSignals(true);
    mDrawSystemContent->clear();

    auto currentIdentifier = getDrawSystemIdentifier();
    std::optional<DrawSystemIdentifier> preferredIdentifier;
    if (mCategoryIds.size() == 1) {
        const auto &tournament = mStoreManager.getTournament();
        auto categoryId = *(mCategoryIds.begin());
        const auto &players = tournament.getCategory(categoryId).getPlayers();
        if (!players.empty())
            preferredIdentifier = tournament.getPreferences().getPreferredDrawSystem(players.size());
    }

    std::size_t i = 0;
    std::size_t index = 0;
    for (const auto & drawSystem : DrawSystem::getDrawSystems()) {
        auto name = QString::fromStdString(drawSystem->getName());

        auto identifier = drawSystem->getIdentifier();
        if (preferredIdentifier == identifier)
            name += tr(" (Preferred)");
        if (currentIdentifier == identifier)
            index = i;
        mDrawSystemContent->addItem(name, QVariant::fromValue(identifier));
        ++i;
    }

    if (!mCategoryIds.empty() && !currentIdentifier.has_value()) { // Multiple different draw system
        mDrawSystemContent->addItem(MULTIPLE_TEXT);
        mDrawSystemContent->setItemData(mDrawSystemContent->count() - 1, QBrush(Qt::gray), Qt::ForegroundRole);
        index = i;
    }

    mDrawSystemContent->setCurrentIndex(index);
    mDrawSystemContent->setEnabled(!mCategoryIds.empty());
    mDrawSystemContent->blockSignals(false);
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

std::optional<std::string> EditCategoryWidget::getNameString() {
    const TournamentStore &tournament = mStoreManager.getTournament();

    std::optional<std::string> res;

    for (auto categoryId : mCategoryIds) {
        const CategoryStore &category = tournament.getCategory(categoryId);
        if (!res.has_value())
            res = category.getName();
        else if (res != category.getName())
            return std::nullopt;
    }

    return res;
}

bool EditCategoryWidget::userConfirmsAction(const ConfirmableAction &action) const {
    const auto &tournament = mStoreManager.getTournament();
    if (!action.doesRequireConfirmation(tournament))
        return true;

    return ConfirmActionDialog::confirmAction();
}

