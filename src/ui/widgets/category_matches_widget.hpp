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

private:
    StoreManager &mStoreManager;
    CategoryMatchesModel *mModel;
};

