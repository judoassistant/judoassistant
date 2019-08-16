#pragma once

#include <QDialog>
#include "core/core.hpp"
#include "core/id.hpp"

class StoreManager;

class EditMatchDialog : public QDialog {
    Q_OBJECT
public:
    EditMatchDialog(StoreManager &storeManager, std::pair<CategoryId, MatchId> combinedId, QWidget *parent);

private:
    StoreManager &mStoreManager;
    std::pair<CategoryId, MatchId> mCombinedId;
};
