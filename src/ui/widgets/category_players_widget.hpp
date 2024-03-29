#pragma once

#include <optional>
#include <QWidget>
#include <QTableView>

#include "core/core.hpp"
#include "core/id.hpp"

class StoreManager;
class PlayersProxyModel;
class ConfirmableAction;

class CategoryPlayersWidget : public QTableView {
    Q_OBJECT
public:
    CategoryPlayersWidget(StoreManager & storeManager, QWidget *parent = nullptr);
    void setCategory(std::optional<CategoryId> categoryId);

protected:
    void tournamentAboutToBeReset();
    void showContextMenu(const QPoint &pos);
    void eraseSelectedPlayersFromCategory();

private:
    bool userConfirmsAction(const ConfirmableAction &action) const;

    StoreManager &mStoreManager;
    PlayersProxyModel *mModel;
};
