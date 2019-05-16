#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/general_tournament_preferences_widget.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>

GeneralTournamentPreferencesWidget::GeneralTournamentPreferencesWidget(StoreManager &storeManager, QWidget *parent)
    : QWidget(parent)
    , mStoreManager(storeManager)
{
    QFormLayout *layout = new QFormLayout;

    auto languageContent = new QComboBox;
    languageContent->addItem("English");

    layout->addRow(new QLabel(tr("Tournament Language")), languageContent);

    setLayout(layout);
    setMinimumWidth(300);
}

