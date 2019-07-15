#include "core/actions/change_scoreboard_style_preference_action.hpp"
#include "core/actions/change_match_card_style_preference_action.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/display_preferences_widget.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>

DisplayPreferencesWidget::DisplayPreferencesWidget(StoreManager &storeManager, QWidget *parent)
    : QWidget(parent)
    , mStoreManager(storeManager)
{
    const auto &preferences = mStoreManager.getTournament().getPreferences();
    QFormLayout *layout = new QFormLayout;

    mScoreboardContent = new QComboBox;
    mScoreboardContent->addItem("National Competition", QVariant::fromValue(ScoreboardStylePreference::NATIONAL));
    // mScoreboardContent->addItem("International Competition", QVariant::fromValue(ScoreboardStylePreference::INTERNATIONAL));

    mScoreboardContent->setCurrentIndex(preferences.getScoreboardStyle() == ScoreboardStylePreference::NATIONAL ? 0 : 1);
    connect(mScoreboardContent, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) { changeScoreboardStyle(); });

    layout->addRow(new QLabel(tr("Scoreboard Style")), mScoreboardContent);

    mMatchCardContent = new QComboBox;
    mMatchCardContent->addItem("National Competition", QVariant::fromValue(MatchCardStylePreference::NATIONAL));
    // mMatchCardContent->addItem("International Competition", QVariant::fromValue(MatchCardStylePreference::INTERNATIONAL));

    mMatchCardContent->setCurrentIndex(preferences.getMatchCardStyle() == MatchCardStylePreference::NATIONAL ? 0 : 1);
    connect(mMatchCardContent, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) { changeMatchCardStyle(); });

    layout->addRow(new QLabel(tr("Match Card Style")), mMatchCardContent);

    setLayout(layout);
    setMinimumWidth(300);
}

void DisplayPreferencesWidget::changeScoreboardStyle() {
    const auto &preferences = mStoreManager.getTournament().getPreferences();

    ScoreboardStylePreference preference = mScoreboardContent->currentData().value<ScoreboardStylePreference>();
    if (preference == preferences.getScoreboardStyle())
        return;

    mStoreManager.dispatch(std::make_unique<ChangeScoreboardStylePreferenceAction>(preference));
}

void DisplayPreferencesWidget::changeMatchCardStyle() {
    const auto &preferences = mStoreManager.getTournament().getPreferences();

    MatchCardStylePreference preference = mMatchCardContent->currentData().value<MatchCardStylePreference>();
    if (preference == preferences.getMatchCardStyle())
        return;

    mStoreManager.dispatch(std::make_unique<ChangeMatchCardStylePreferenceAction>(preference));
}

