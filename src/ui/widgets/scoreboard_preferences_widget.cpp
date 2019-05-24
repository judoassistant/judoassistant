#include "core/actions/change_scoreboard_style_preference_action.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/scoreboard_preferences_widget.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>

ScoreboardPreferencesWidget::ScoreboardPreferencesWidget(StoreManager &storeManager, QWidget *parent)
    : QWidget(parent)
    , mStoreManager(storeManager)
{
    const auto &preferences = mStoreManager.getTournament().getPreferences();
    QFormLayout *layout = new QFormLayout;

    mLanguageContent = new QComboBox;
    mLanguageContent->addItem("National Competition", QVariant::fromValue(ScoreboardStylePreference::NATIONAL));
    mLanguageContent->addItem("International Competition", QVariant::fromValue(ScoreboardStylePreference::INTERNATIONAL));

    mLanguageContent->setCurrentIndex(preferences.getScoreboardStyle() == ScoreboardStylePreference::NATIONAL ? 0 : 1);
    connect(mLanguageContent, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) { changeStyle(); });


    layout->addRow(new QLabel(tr("Scoreboard Style")), mLanguageContent);

    setLayout(layout);
    setMinimumWidth(300);
}

void ScoreboardPreferencesWidget::changeStyle() {
    const auto &preferences = mStoreManager.getTournament().getPreferences();

    ScoreboardStylePreference preference = mLanguageContent->currentData().value<ScoreboardStylePreference>();
    if (preference == preferences.getScoreboardStyle())
        return;

    mStoreManager.dispatch(std::make_unique<ChangeScoreboardStylePreferenceAction>(preference));
}

