#include <QDialogButtonBox>
#include <QListWidget>
#include <QVBoxLayout>

#include "ui/widgets/network_information_dialog.hpp"

NetworkInformationDialog::NetworkInformationDialog(QWidget *parent)
    : QDialog(parent)
{
    QListWidget *listWidget = new QListWidget;

    listWidget->addItem(tr("General Preferences"));
    listWidget->setCurrentRow(0);
    // connect(listWidget, &QListWidget::currentRowChanged, stackedWidget, &QStackedWidget::setCurrentIndex);

    // Setup layouts
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(listWidget);

    // Setup button box
    {
        QDialogButtonBox *buttonBox = new QDialogButtonBox;
        buttonBox->addButton(tr("Close"), QDialogButtonBox::AcceptRole);
        mainLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, this, &NetworkInformationDialog::accept);
    }

    setLayout(mainLayout);
    setWindowTitle(tr("Network Information"));
    setGeometry(geometry().x(), geometry().y(), 800, 400);
}

