#pragma once

#include <QWidget>

class StoreManager;

class NewMatchesWidget : public QWidget {
    Q_OBJECT
public:
    NewMatchesWidget(StoreManager &storeManager);

private:
    void beginTournamentReset();
    void endTournamentReset();

    void beginTatamiCountChange();
    void endTatamiCountChange();

    StoreManager &mStoreManager;
};
