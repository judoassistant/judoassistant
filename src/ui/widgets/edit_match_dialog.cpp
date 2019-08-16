#include "ui/store_managers/store_manager.hpp"
#include "ui/widgets/edit_match_dialog.hpp"

EditMatchDialog::EditMatchDialog(StoreManager &storeManager, std::pair<CategoryId, MatchId> combinedId, QWidget *parent = nullptr)
    : QDialog(parent)
    , mStoreManager(storeManager)
    , mCombinedId(combinedId)
{}

