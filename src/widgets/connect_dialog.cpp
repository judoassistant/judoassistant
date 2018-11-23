#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFormLayout>

#include "widgets/connect_dialog.hpp"

ConnectDialog::ConnectDialog(const std::optional<QString> &host, std::optional<unsigned int> port, QWidget *parent)
    : QDialog(parent)
{
    mHostContent = new QLineEdit;
    if (host)
        mHostContent->setText(*host);

    mPortContent = new QSpinBox;
    mPortContent->setRange(1025, 65535);
    if (port)
        mPortContent->setValue(*port);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("Host"), mHostContent);
    formLayout->addRow(tr("Port"), mPortContent);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    setWindowTitle(tr("Connect to Hub"));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ConnectDialog::acceptClick);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ConnectDialog::cancelClick);
}

void ConnectDialog::acceptClick() {
    accept();
}

void ConnectDialog::cancelClick() {
    reject();
}

