#pragma once

#include <QWidget>
#include <QHBoxLayout>

#include "core.hpp"
#include "store_managers/store_manager.hpp"
#include "widgets/tatami_widget.hpp"

// TODO: Display whenever two matches conflict with player

class TatamisWidget : public QWidget {
    Q_OBJECT

public:
    TatamisWidget(StoreManager &storeManager);

public slots:
    void tatamisReset();

private:
    StoreManager &mStoreManager;
    QHBoxLayout *mTatamiLayout;
    std::vector<TatamiWidget*> mTatamis;
};

