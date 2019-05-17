#pragma once

#include <stack>
#include <QMetaObject>
#include <QWidget>

#include "core/core.hpp"

class StoreManager;

class ScoreboardPreferencesWidget : public QWidget {
    Q_OBJECT
public:
    ScoreboardPreferencesWidget(StoreManager &storeManager, QWidget *parent = nullptr);

private:
    StoreManager &mStoreManager;
};

