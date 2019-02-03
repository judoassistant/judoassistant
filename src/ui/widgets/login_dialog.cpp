#include <QDialogButtonBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "core/log.hpp"
#include "ui/store_managers/master_store_manager.hpp"
#include "ui/widgets/login_dialog.hpp"

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
    connect(&webClient, &WebClient::loginFailed, this, &LoginDialog::failLogin);
    connect(&webClient, &WebClient::loginSucceeded, this, &LoginDialog::succeedLogin);
}

void LoginDialog::loginClick() {
    mLoginButton->setText(tr("Logging in.."));
    mLoginButton->setEnabled(false);

    auto &webClient = mStoreManager.getWebClient();
    auto email = mEmailContent->text();
    auto password = mPasswordContent->text();
    webClient.loginUser(email, password);
}

void LoginDialog::cancelClick() {
    reject();
}

void LoginDialog::succeedLogin(const WebToken &token) {
    mToken = token;
    accept();
}

void LoginDialog::failLogin(const WebTokenRequestResponse &response) {
    mLoginButton->setText(tr("Login"));
    mLoginButton->setEnabled(true);

    if (response == WebTokenRequestResponse::SERVER_ERROR)
        QMessageBox::warning(this, tr("Login failed"), tr("The connection attempt to live web failed. Try again later."), QMessageBox::Ok, QMessageBox::Ok);
    else if (response == WebTokenRequestResponse::INCORRECT_CREDENTIALS)
        QMessageBox::warning(this, tr("Login failed"), tr("Wrong email or password"), QMessageBox::Ok, QMessageBox::Ok);
}

const WebToken& LoginDialog::getToken() const {
    return mToken.value();
}

