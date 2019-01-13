#pragma once

#include "core.hpp"

#include <QDialog>
#include <QLineEdit>

class MasterStoreManager;

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    LoginDialog(MasterStoreManager &storeManager, QWidget *parent = nullptr);

protected:
    void loginClick();
    void cancelClick();
    void succeedLogin(const QString &token);
    void failLogin();
private:
    MasterStoreManager &mStoreManager;
    QLineEdit *mEmailContent;
    QLineEdit *mPasswordContent;
    QPushButton *mLoginButton;
};
