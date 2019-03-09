#pragma once

#include <stack>

#include <QWidget>
#include <QHBoxLayout>

#include "core/core.hpp"
#include "ui/widgets/fixed_scroll_area.hpp"

// TODO: Display whenever two matches conflict with player
// TODO: Consider updating more efficiently when tatami count is changed
// TODO: Handle resizing when changing tatami count
class StoreManager;
class TatamiWidget;

class NewTatamisWidget : public QWidget {
    Q_OBJECT

public:
    NewTatamisWidget(StoreManager &storeManager);

private:
    void beginTournamentReset();
    void endTournamentReset();

    void beginTatamiCountChange();
    void endTatamiCountChange();

    StoreManager &mStoreManager;
    QGraphicsScene *mScene;
    FixedScrollArea *mFixedScrollArea;
    std::stack<QMetaObject::Connection> mConnections;
};

