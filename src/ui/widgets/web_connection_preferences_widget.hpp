#pragma once

#include <stack>
#include <QMetaObject>
#include <QWidget>

#include "core/core.hpp"

class StoreManager;
class QLineEdit;
class QSpinBox;
class QCheckBox;

class WebConnectionPreferencesWidget : public QWidget {
    Q_OBJECT
public:
    WebConnectionPreferencesWidget(StoreManager &storeManager, QWidget *parent = nullptr);

private:
    void useCustomServer(bool checked);
    StoreManager &mStoreManager;

    QLineEdit *mHostnameContent;
    QSpinBox *mPortContent;
    QCheckBox *mSSLContent;
};

