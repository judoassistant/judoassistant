#include "ui/models/category_matches_model.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/widgets/category_matches_widget.hpp"
#include "ui/delegates/match_card_delegate.hpp"

CategoryMatchesWidget::CategoryMatchesWidget(StoreManager & storeManager, QWidget *parent)
    : QListView(parent)
    , mStoreManager(storeManager)
{
    mModel = new CategoryMatchesModel(mStoreManager, this);
    setItemDelegate(new MatchCardDelegate(this));
    setModel(mModel);
}

void CategoryMatchesWidget::setCategory(std::optional<CategoryId> categoryId) {
    mModel->setCategory(categoryId);
}

