#pragma once

#include <stack>

#include <QWidget>
#include <QHBoxLayout>

#include "core.hpp"

// TODO: Display whenever two matches conflict with player
// TODO: Consider updating more efficiently when tatami count is changed
// TODO: Handle resizing when changing tatami count
class StoreManager;
class TatamiWidget;

class TatamisWidget : public QWidget {
    Q_OBJECT

public:
    TatamisWidget(StoreManager &storeManager);

public slots:
    void tournamentAboutToBeReset();
    void tournamentReset();

    void tatamiCountAboutToBeChanged();
    void tatamiCountChanged();

private:
    StoreManager &mStoreManager;
    QHBoxLayout *mTatamiLayout;
    std::vector<TatamiWidget*> mTatamis;
    std::stack<QMetaObject::Connection> mConnections;
};

