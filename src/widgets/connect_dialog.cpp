#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>

#include "widgets/connect_dialog.hpp"
#include "store_managers/client_store_manager.hpp"

ConnectDialog::ConnectDialog(ClientStoreManager &storeManager, QWidget *parent)
    : QDialog(parent)
    , mStoreManager(storeManager)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    {
        QGroupBox *group = new QGroupBox(tr("Server address"));
        QFormLayout *formLayout = new QFormLayout;

        mHostContent = new QLineEdit;
        mHostContent->setText("127.0.0.1");
        // if (host)
        //     mHostContent->setText(*host);

        mPortContent = new QSpinBox;
        mPortContent->setRange(1025, 65535);
        mPortContent->setValue(8000);
        // if (port)
        //     mPortContent->setValue(*port);

        mPasswordContent = new QLineEdit;
        // if (password)
        //     mPasswordContent->setText(*password);

        formLayout->addRow(tr("Host"), mHostContent);
        formLayout->addRow(tr("Port"), mPortContent);
        formLayout->addRow(tr("Password"), mPasswordContent);
        group->setLayout(formLayout);
        mainLayout->addWidget(group);
    }

    {
        QGroupBox *group = new QGroupBox(tr("Retry on connection loss"));
        QFormLayout *formLayout = new QFormLayout;

        mShouldRetryContent = new QCheckBox;
        mShouldRetryContent->setCheckState(Qt::Unchecked);

        mRetryFrequencyContent = new QSpinBox;
        mRetryFrequencyContent->setRange(30, 600);
        mRetryFrequencyContent->setSuffix("s");
        mRetryFrequencyContent->setEnabled(false);
        // if (retryFrequency)
        //     mRetryFrequencyContent->setValue(*port);

        connect(mShouldRetryContent, &QCheckBox::stateChanged, [this](int state) {mRetryFrequencyContent->setEnabled(state == Qt::Checked);});

        formLayout->addRow(tr("Attempt to connect after connection loss"), mShouldRetryContent);
        formLayout->addRow(tr("Retry frequency"), mRetryFrequencyContent);
        group->setLayout(formLayout);
        mainLayout->addWidget(group);
    }

    {
        QDialogButtonBox *buttonBox = new QDialogButtonBox;
        buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
        buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

        connect(buttonBox, &QDialogButtonBox::accepted, this, &ConnectDialog::acceptClick);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &ConnectDialog::cancelClick);

        mainLayout->addWidget(buttonBox);
    }

    connect(&mStoreManager, &ClientStoreManager::connectionAttemptFailed, this, &ConnectDialog::failConnectionAttempt);
    connect(&mStoreManager, &ClientStoreManager::connectionAttemptSucceeded, this, &ConnectDialog::succeedConnectionAttempt);

    setLayout(mainLayout);
    setWindowTitle(tr("Connect to Hub"));
}

void ConnectDialog::acceptClick() {
    mStoreManager.connect(mHostContent->text(), mPortContent->value());
}

void ConnectDialog::cancelClick() {
    reject();
}

void ConnectDialog::succeedConnectionAttempt() {
    log_debug().msg("Succeeded connection attempt");
    accept();
}

void ConnectDialog::failConnectionAttempt() {
    log_debug().msg("Failed connection attempt");

}

