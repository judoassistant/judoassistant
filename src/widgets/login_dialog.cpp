#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include "log.hpp"
#include "store_managers/master_store_manager.hpp"
#include "widgets/login_dialog.hpp"

LoginDialog::LoginDialog(MasterStoreManager &storeManager, QWidget *parent)
    : QDialog(parent)
    , mStoreManager(storeManager)
{
    setWindowTitle(tr("Login"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    {
        QWidget *group = new QWidget(this);
        QFormLayout *formLayout = new QFormLayout(group);

        mEmailContent = new QLineEdit;

        mPasswordContent = new QLineEdit;
        mPasswordContent->setEchoMode(QLineEdit::Password);

        formLayout->addRow(tr("Email"), mEmailContent);
        formLayout->addRow(tr("Password"), mPasswordContent);

        mainLayout->addWidget(group);
    }

    {
        QDialogButtonBox *buttonBox = new QDialogButtonBox;
        mLoginButton = new QPushButton(tr("Login"));
        buttonBox->addButton(mLoginButton, QDialogButtonBox::AcceptRole);
        buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

        connect(buttonBox, &QDialogButtonBox::accepted, this, &LoginDialog::loginClick);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &LoginDialog::cancelClick);

        mainLayout->addWidget(buttonBox);
    }

    auto &webClient = mStoreManager.getWebClient();
    connect(&webClient, &WebClient::logInFailed, this, &LoginDialog::failLogin);
    connect(&webClient, &WebClient::logInSucceeded, this, &LoginDialog::succeedLogin);
}

void LoginDialog::loginClick() {
    mLoginButton->setText(tr("Connecting.."));
    mLoginButton->setEnabled(false);

    auto &webClient = mStoreManager.getWebClient();
    auto email = mEmailContent->text();
    auto password = mPasswordContent->text();
    webClient.loginUser(email, password);
}

void LoginDialog::cancelClick() {
    reject();
}

void LoginDialog::succeedLogin(const QString &token) {
    log_debug().msg("Login suceeded");
    accept();
}

void LoginDialog::failLogin() {
    mLoginButton->setText(tr("Connect"));
    mLoginButton->setEnabled(true);
    log_debug().msg("Login failed");
}

