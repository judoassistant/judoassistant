#pragma once

#include <optional>
#include <QWidget>
#include <QTableView>

#include "core/core.hpp"
#include "core/id.hpp"

class StoreManager;
class PlayersProxyModel;

class EditCategoryMatchesWidget : public QWidget {
    Q_OBJECT
public:
    EditCategoryMatchesWidget (StoreManager & storeManager, QWidget *parent);
    void setCategory(std::optional<CategoryId> categoryId);

protected slots:
    void tournamentAboutToBeReset();

private:
    StoreManager &mStoreManager;
    QTableView *mTableView;
    PlayersProxyModel *mModel;
};
