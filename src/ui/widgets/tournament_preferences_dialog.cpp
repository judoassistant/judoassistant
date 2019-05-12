#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QListWidget>
#include <QTableView>

#include "ui/widgets/tournament_preferences_dialog.hpp"
#include "ui/widgets/preferred_draw_systems_widget.hpp"

TournamentPreferencesDialog::TournamentPreferencesDialog(StoreManager & storeManager, QWidget *parent)
    : QDialog(parent)
    , mStoreManager(storeManager)
{
    QStackedWidget *stackedWidget = new QStackedWidget;
    QListWidget *listWidget = new QListWidget;
    listWidget->setMaximumWidth(200);

    // Add general page
    {
        QWidget *widget = new QTableView;

        listWidget->addItem(tr("General Preferences"));
        stackedWidget->addWidget(widget);
    }
    // Add draw systems page
    {
        QWidget *widget = new PreferredDrawSystemsWidget(mStoreManager);

        listWidget->addItem(tr("Draw Systems"));
        stackedWidget->addWidget(widget);
    }

    // listWidget->setCurrentIndex(1);
    stackedWidget->setCurrentIndex(1);

    // Setup layouts
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(listWidget);
    horizontalLayout->addWidget(stackedWidget);

    mainLayout->addLayout(horizontalLayout);

    // Setup button box
    {
        QDialogButtonBox *buttonBox = new QDialogButtonBox;
        buttonBox->addButton(tr("Close"), QDialogButtonBox::AcceptRole);
        mainLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, this, &TournamentPreferencesDialog::closeClick);
    }

    setLayout(mainLayout);
    setWindowTitle(tr("Tournament Preferences"));
}

void TournamentPreferencesDialog::closeClick() {
    accept();
}

