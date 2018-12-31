#pragma once

#include "core.hpp"

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>

class ClientStoreManager;

class ConnectDialog : public QDialog {
    Q_OBJECT
public:
    ConnectDialog(ClientStoreManager &storeManager, QString host, std::optional<int> port, std::optional<std::chrono::seconds> retryInterval, QWidget *parent = nullptr);

    QString getHost() const;
    std::optional<int> getPort() const;
    std::optional<std::chrono::seconds> getRetryInterval() const;

protected:
    void connectClick();
    void cancelClick();
    void succeedConnectionAttempt();
    void failConnectionAttempt();
private:
    ClientStoreManager &mStoreManager;
    QLineEdit *mHostContent;
    QSpinBox *mPortContent;
    QLineEdit *mPasswordContent;
    QCheckBox *mShouldRetryContent;
    QSpinBox *mRetryIntervalContent;
    QPushButton *mConnectButton;
};
