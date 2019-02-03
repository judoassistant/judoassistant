#pragma once

#include <stack>

#include <QWidget>

#include "ui/widgets/fixed_scroll_area.hpp"

class StoreManager;

class MatchesWidget : public QWidget {
    Q_OBJECT

public:
    MatchesWidget(StoreManager &storeManager);

private:
    void beginTournamentReset();
    void endTournamentReset();

    void beginTatamiCountChange();
    void endTatamiCountChange();

    StoreManager & mStoreManager;
    FixedScrollArea *mFixedScrollArea;
    std::stack<QMetaObject::Connection> mConnections;
};

