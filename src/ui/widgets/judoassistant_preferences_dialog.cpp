#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QListWidget>
#include <QTableView>

#include "ui/widgets/judoassistant_preferences_dialog.hpp"
#include "ui/widgets/web_connection_preferences_widget.hpp"
#include "ui/widgets/save_preferences_widget.hpp"

JudoassistantPreferencesDialog::JudoassistantPreferencesDialog(MasterStoreManager & storeManager, QWidget *parent)
    : QDialog(parent)
    , mStoreManager(storeManager)
{
    QStackedWidget *stackedWidget = new QStackedWidget;
    QListWidget *listWidget = new QListWidget;
    listWidget->setMaximumWidth(150);

    // Add save preferences
    {
        QWidget *widget = new SavePreferencesWidget(storeManager);

        listWidget->addItem(tr("Tournament File Saving"));
        stackedWidget->addWidget(widget);
    }

    // Add web connection page
    {
        QWidget *widget = new WebConnectionPreferencesWidget(storeManager);

        listWidget->addItem(tr("Web Connection"));
        stackedWidget->addWidget(widget);
    }

    listWidget->setCurrentRow(0);
    stackedWidget->setCurrentIndex(0);
    connect(listWidget, &QListWidget::currentRowChanged, stackedWidget, &QStackedWidget::setCurrentIndex);

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

        connect(buttonBox, &QDialogButtonBox::accepted, this, &JudoassistantPreferencesDialog::closeClick);
    }

    setLayout(mainLayout);
    setWindowTitle(tr("JudoAssistant Preferences"));

    setGeometry(geometry().x(), geometry().y(), 800, 400);
}

void JudoassistantPreferencesDialog::closeClick() {
    accept();
}

