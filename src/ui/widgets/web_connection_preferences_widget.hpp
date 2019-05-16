#pragma once

#include <stack>
#include <QMetaObject>
#include <QWidget>

#include "core/core.hpp"

class MasterStoreManager;
class QLineEdit;
class QSpinBox;
class QCheckBox;

class WebConnectionPreferencesWidget : public QWidget {
    Q_OBJECT
public:
    WebConnectionPreferencesWidget(MasterStoreManager &storeManager, QWidget *parent = nullptr);

private:
    void setCustomServer(bool value);
    void setHostname(const QString &value);
    void setPort(int value);
    void setSSL(bool value);
    MasterStoreManager &mStoreManager;

    QLineEdit *mHostnameContent;
    QSpinBox *mPortContent;
    QCheckBox *mSSLContent;
};

