#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include "log.hpp"
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
}

void LoginDialog::loginClick() {
    log_debug().msg("Login clicked");

}

void LoginDialog::cancelClick() {
    reject();
}

void LoginDialog::logInSucceeded() {
    log_debug().msg("Login suceeded");
}

void LoginDialog::logInFailed() {
    log_debug().msg("Login failed");
}

