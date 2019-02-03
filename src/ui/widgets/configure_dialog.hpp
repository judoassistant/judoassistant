#pragma once

#include "core/core.hpp"
#include "core/web/web_types.hpp"

#include <QDialog>
#include <QLineEdit>

class MasterStoreManager;

// TODO: Implement live name checks
class ConfigureDialog : public QDialog {
    Q_OBJECT
public:
    ConfigureDialog(MasterStoreManager &storeManager, QWidget *parent = nullptr);
    const std::string& getWebName() const;

protected:
    void checkWebName();

    void registerClick();
    void cancelClick();
    void succeedRegistration(const QString &webName);
    void failRegistration(WebNameRegistrationResponse response);
    void showCheckResult(const QString &webName, WebNameCheckResponse status);
private:
    MasterStoreManager &mStoreManager;
    QLineEdit *mWebNameContent;
    QPushButton *mRegisterButton;
};

