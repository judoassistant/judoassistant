#pragma once

#include <stack>
#include <QMetaObject>
#include <QWidget>

#include "core/core.hpp"

class StoreManager;

class GeneralTournamentPreferencesWidget : public QWidget {
    Q_OBJECT
public:
    GeneralTournamentPreferencesWidget(StoreManager &storeManager, QWidget *parent = nullptr);

private:
    StoreManager &mStoreManager;
};

