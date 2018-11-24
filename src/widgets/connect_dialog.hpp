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
    ConnectDialog(ClientStoreManager &client, QWidget *parent = nullptr);

signals:
    void accepted(QString host, unsigned int port);
protected:
    void acceptClick();
    void cancelClick();
    void succeedConnectionAttempt();
    void failConnectionAttempt();
private:
    ClientStoreManager &mStoreManager;
    QLineEdit *mHostContent;
    QSpinBox *mPortContent;
    QLineEdit *mPasswordContent;
    QCheckBox *mShouldRetryContent;
    QSpinBox *mRetryFrequencyContent;
};
