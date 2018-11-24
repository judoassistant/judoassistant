#pragma once

#include <QWidget>
#include <QHBoxLayout>

#include "core.hpp"

// TODO: Display whenever two matches conflict with player
class StoreManager;
class TatamiWidget;

class TatamisWidget : public QWidget {
    Q_OBJECT

public:
    TatamisWidget(StoreManager &storeManager);

public slots:
    void tournamentAboutToBeReset();
    void tournamentReset();

private:
    StoreManager &mStoreManager;
    QHBoxLayout *mTatamiLayout;
    std::vector<TatamiWidget*> mTatamis;
};

