#pragma once

#include <stack>
#include <QLineEdit>
#include <QPushButton>
#include <QMetaObject>
#include <QGroupBox>

#include "core/core.hpp"
#include "ui/web/web_client.hpp"

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
    void changeWebClientState(WebClientState state);
    void succeedLogin(const WebToken &token);

    void updateButton();
    void buttonClick();

    MasterStoreManager &mStoreManager;
    std::stack<QMetaObject::Connection> mConnections;

    std::optional<WebToken> mToken;

    QPushButton *mSetupButton;
    QLineEdit *mWebNameContent;
    WebClientState mCurrentState;
};

