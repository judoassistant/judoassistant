#pragma once

#include <QWidget>
#include <QTableView>

#include "core.hpp"
#include "store_handlers/qstore_handler.hpp"
#include "models/players_model.hpp"

class EditCategoryPlayersWidget : public QWidget {
    Q_OBJECT
public:
    EditCategoryPlayersWidget (QStoreHandler & storeHandler, QWidget *parent);
    void setCategory(std::optional<CategoryId> categoryId);

protected slots:
    void tournamentReset();

private:
    QStoreHandler &mStoreHandler;
    QTableView *mTableView;
    PlayersProxyModel *mModel;
};
