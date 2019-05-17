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
    QFormLayout *layout = new QFormLayout;

    auto languageContent = new QComboBox;
    languageContent->addItem("Internal Competition");
    languageContent->addItem("National Competition");

    layout->addRow(new QLabel(tr("Scoreboard Style")), languageContent);

    setLayout(layout);
    setMinimumWidth(300);
}

