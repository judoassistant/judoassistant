#pragma once

#include <QMainWindow>
#include <QString>
#include <QHBoxLayout>

#include "core.hpp"
#include "store_handlers/qstore_handler.hpp"
#include "widgets/tatami_widget.hpp"

// TODO: Display whenever two matches conflict with player

class TatamisWidget : public QWidget {
    Q_OBJECT

public:
    TatamisWidget(QStoreHandler &storeHandler);

public slots:
    void tatamisReset();

private:
    QStoreHandler &mStoreHandler;
    QHBoxLayout *mTatamiLayout;
    std::vector<TatamiWidget*> mTatamis;
};

