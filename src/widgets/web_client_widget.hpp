#pragma once

#include <stack>
#include <QMetaObject>
#include <QGroupBox>

#include "core.hpp"
#include "web/web_client.hpp"

class MasterStoreManager;

class WebClientWidget : public QGroupBox {
    Q_OBJECT

public:
    WebClientWidget(MasterStoreManager &storeManager, QWidget *parent = nullptr);

    MasterStoreManager &mStoreManager;
    std::stack<QMetaObject::Connection> mConnections;
    WebClient::Status mWebStatus;
};

