#include <QVBoxLayout>

#include "ui/store_managers/store_manager.hpp"
#include "ui/widgets/edit_match_dialog.hpp"
#include "ui/widgets/score_operator_widget.hpp"

EditMatchDialog::EditMatchDialog(StoreManager &storeManager, std::pair<CategoryId, MatchId> combinedId, QWidget *parent)
    : QDialog(parent)
    , mStoreManager(storeManager)
    , mCombinedId(combinedId)
{
    QVBoxLayout *layout = new QVBoxLayout;

    auto *operatorWidget = new ScoreOperatorWidget(mStoreManager);
    operatorWidget->setMatch(combinedId, false);
    layout->addWidget(operatorWidget);

    setLayout(layout);
    setGeometry(geometry().x(), geometry().y(), 800, 450);
    setWindowTitle(tr("Edit Match"));
}

