#pragma once

#include <stack>
#include <QLineEdit>
#include <QPushButton>
#include <QMetaObject>
#include <QGroupBox>

#include "core.hpp"
#include "web/web_client.hpp"

class MasterStoreManager;

class WebClientWidget : public QGroupBox {
    Q_OBJECT
public:
    WebClientWidget(MasterStoreManager &storeManager, QWidget *parent = nullptr);

private:
    void addWidgets();

    void changeTournament();
    void beginResetTournament();
    void endResetTournament();
    void changeWebStatus(WebClient::Status status);

    void updateButton();

    void buttonClick();

    MasterStoreManager &mStoreManager;
    std::stack<QMetaObject::Connection> mConnections;
    WebClient::Status mWebStatus;

    std::string mToken;

    QPushButton *mSetupButton;
    QLineEdit *mWebNameContent;
};

