#pragma once

#include <QWidget>

class StoreManager;

class MatchesWidget : public QWidget {
    Q_OBJECT

public:
    MatchesWidget(StoreManager &storeManager);
};

