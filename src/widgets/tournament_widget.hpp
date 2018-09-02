#pragma once

#include <QMainWindow>
#include <QLineEdit>

#include "core.hpp"
#include "store_handlers/qstore_handler.hpp"

class TournamentWidget : public QWidget {
    Q_OBJECT

public slots:
    void tournamentChanged();
    void updateTournamentName();
public:
    TournamentWidget(QStoreHandler &storeHandler);
private:
    QStoreHandler &mStoreHandler;
    QLineEdit *mNameContent;
};

