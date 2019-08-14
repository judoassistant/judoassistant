#pragma once

#include <optional>
#include <QDialog>
#include <QLineEdit>

#include "core/core.hpp"
#include "core/web/web_types.hpp"

class MasterStoreManager;

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    LoginDialog(MasterStoreManager &storeManager, QWidget *parent = nullptr);
    const WebToken& getToken() const;

protected:
    void loginClick();
    void cancelClick();
    void succeedLogin(const WebToken &token);
    void failLogin(const WebTokenRequestResponse &response);
private:
    MasterStoreManager &mStoreManager;
    QLineEdit *mEmailContent;
    QLineEdit *mPasswordContent;
    QPushButton *mLoginButton;
    std::optional<WebToken> mToken;
};

