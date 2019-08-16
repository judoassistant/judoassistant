#pragma once

#include <optional>
#include <QListView>

#include "core/core.hpp"
#include "core/id.hpp"

class CategoryMatchesModel;
class StoreManager;

class CategoryMatchesWidget : public QListView {
    Q_OBJECT
public:
    CategoryMatchesWidget(StoreManager &storeManager, QWidget *parent = nullptr);
    void setCategory(std::optional<CategoryId> categoryId);

protected:
    void showContextMenu(const QPoint &pos);
    void showEditDialog();
    void showResetDialog();

private:
    StoreManager &mStoreManager;
    CategoryMatchesModel *mModel;
};

