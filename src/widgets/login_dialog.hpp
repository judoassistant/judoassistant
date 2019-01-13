#pragma once

#include "core.hpp"

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>

class MasterStoreManager;

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    LoginDialog(MasterStoreManager &storeManager);

protected:
    void loginClick();
    void cancelClick();
    void logInSucceeded();
    void logInFailed();
private:
    MasterStoreManager &mStoreManager;
    QLineEdit *mEmailContent;
    QLineEdit *mPasswordContent;
    QPushButton *mLoginButton;
};
