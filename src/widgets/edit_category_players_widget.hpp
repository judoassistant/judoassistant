#pragma once

#include <QWidget>
#include <QTableView>

#include "core.hpp"
#include "store_managers/store_manager.hpp"
#include "models/players_model.hpp"

class EditCategoryPlayersWidget : public QWidget {
    Q_OBJECT
public:
    EditCategoryPlayersWidget (StoreManager & storeManager, QWidget *parent);
    void setCategory(std::optional<CategoryId> categoryId);

protected slots:
    void tournamentReset();

private:
    StoreManager &mStoreManager;
    QTableView *mTableView;
    PlayersProxyModel *mModel;
};
