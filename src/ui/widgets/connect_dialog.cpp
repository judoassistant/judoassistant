#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>

#include "ui/widgets/connect_dialog.hpp"
#include "ui/store_managers/client_store_manager.hpp"

ConnectDialog::ConnectDialog(ClientStoreManager &storeManager, QString host, std::optional<int> port, std::optional<std::chrono::seconds> retryInterval, QWidget *parent)
    : QDialog(parent)
    , mStoreManager(storeManager)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    {
        QGroupBox *group = new QGroupBox(tr("Server address"));
        QFormLayout *formLayout = new QFormLayout;

        mHostContent = new QLineEdit;
        mHostContent->setText(host);

        mPortContent = new QSpinBox;
        mPortContent->setRange(1025, 65535);
        mPortContent->setValue(8000);
        if (port)
            mPortContent->setValue(*port);

        // mPasswordContent = new QLineEdit;
        // if (password)
        //     mPasswordContent->setText(*password);

        formLayout->addRow(tr("Host"), mHostContent);
        formLayout->addRow(tr("Port"), mPortContent);
        // formLayout->addRow(tr("Password"), mPasswordContent);
        group->setLayout(formLayout);
        mainLayout->addWidget(group);
    }

    {
        QGroupBox *group = new QGroupBox(tr("Retry on connection loss"));
        QFormLayout *formLayout = new QFormLayout;

        mShouldRetryContent = new QCheckBox;
        mShouldRetryContent->setCheckState(retryInterval ? Qt::Checked : Qt::Unchecked);

        mRetryIntervalContent = new QSpinBox;
        mRetryIntervalContent->setRange(20, 600);
        mRetryIntervalContent->setSuffix("s");
        mRetryIntervalContent->setEnabled(false);
        if (retryInterval) {
            int val = retryInterval->count();
            assert(20 <= val && val <= 600);
            mRetryIntervalContent->setValue(val);
            mRetryIntervalContent->setEnabled(true);
        }

        connect(mShouldRetryContent, &QCheckBox::stateChanged, [this](int state) {mRetryIntervalContent->setEnabled(state == Qt::Checked);});

        formLayout->addRow(tr("Attempt to connect after connection loss"), mShouldRetryContent);
        formLayout->addRow(tr("Retry frequency"), mRetryIntervalContent);
        group->setLayout(formLayout);
        mainLayout->addWidget(group);
    }

    {
        QDialogButtonBox *buttonBox = new QDialogButtonBox;
        mConnectButton = new QPushButton(tr("OK"));
        buttonBox->addButton(mConnectButton, QDialogButtonBox::AcceptRole);
        buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

        connect(buttonBox, &QDialogButtonBox::accepted, this, &ConnectDialog::connectClick);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &ConnectDialog::cancelClick);

        mainLayout->addWidget(buttonBox);
    }

    connect(&mStoreManager, &ClientStoreManager::connectionAttemptFailed, this, &ConnectDialog::failConnectionAttempt);
    connect(&mStoreManager, &ClientStoreManager::connectionAttemptSucceeded, this, &ConnectDialog::succeedConnectionAttempt);

    setLayout(mainLayout);
    setWindowTitle(tr("Connect to Hub"));
}

void ConnectDialog::connectClick() {
    mConnectButton->setText(tr("Connecting.."));
    mConnectButton->setEnabled(false);
    mStoreManager.connect(mHostContent->text(), mPortContent->value());
}

void ConnectDialog::cancelClick() {
    reject();
}

void ConnectDialog::succeedConnectionAttempt() {
    accept();
}

void ConnectDialog::failConnectionAttempt() {
    mConnectButton->setText(tr("Connect"));
    mConnectButton->setEnabled(true);
    QMessageBox::warning(this, tr("Connection failed"), tr("The connection attempt to the hub failed"), QMessageBox::Ok, QMessageBox::Ok);
}

QString ConnectDialog::getHost() const {
    return mHostContent->text();
}

std::optional<int> ConnectDialog::getPort() const {
    return mPortContent->value();
}

std::optional<std::chrono::seconds> ConnectDialog::getRetryInterval() const {
    if (mRetryIntervalContent->isEnabled())
        return std::chrono::seconds(mRetryIntervalContent->value());
    return std::nullopt;
}

