#include <QMenu>
#include <QMessageBox>

#include "core/actions/reset_matches_action.hpp"
#include "core/log.hpp"
#include "ui/delegates/match_delegate.hpp"
#include "ui/models/category_matches_model.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/widgets/category_matches_widget.hpp"
#include "ui/widgets/edit_match_dialog.hpp"

CategoryMatchesWidget::CategoryMatchesWidget(StoreManager & storeManager, QWidget *parent)
    : QListView(parent)
    , mStoreManager(storeManager)
{
    mModel = new CategoryMatchesModel(mStoreManager, this);
    setItemDelegate(new MatchDelegate(mStoreManager, this));
    setModel(mModel);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::SingleSelection);

    connect(this, &QListView::customContextMenuRequested, this, &CategoryMatchesWidget::showContextMenu);
}

void CategoryMatchesWidget::setCategory(std::optional<CategoryId> categoryId) {
    mModel->setCategory(categoryId);
}

void CategoryMatchesWidget::showEditDialog() {
    if (!mModel->getCategory().has_value())
        return;
    std::vector<MatchId> matchIds = mModel->getMatches(selectionModel()->selection());
    if (matchIds.empty())
        return;

    const CombinedId combinedId(mModel->getCategory().value(), matchIds.front());
    EditMatchDialog dialog(mStoreManager, combinedId, this);
    dialog.exec();
}

void CategoryMatchesWidget::showResetDialog() {
    auto reply = QMessageBox::question(this, tr("Would you like to reset the match?"), tr("Are you sure you would like to reset the match?"), QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
    if (reply == QMessageBox::Cancel)
        return;

    if (!mModel->getCategory().has_value())
        return;
    std::vector<MatchId> matchIds = mModel->getMatches(selectionModel()->selection());
    if (matchIds.empty())
        return;

    const CombinedId combinedId(mModel->getCategory().value(), matchIds.front());
    mStoreManager.dispatch(std::make_unique<ResetMatchesAction>(combinedId));
}

void CategoryMatchesWidget::showContextMenu(const QPoint &pos) {
    if (!mModel->getCategory().has_value())
        return;
    std::vector<MatchId> matchIds = mModel->getMatches(selectionModel()->selection());
    if (matchIds.empty())
        return;

    QMenu menu;
    {
        QAction *action = menu.addAction(tr("Edit Match"));
        connect(action, &QAction::triggered, this, &CategoryMatchesWidget::showEditDialog);
    }
    {
        QAction *action = menu.addAction(tr("Reset Match"));
        connect(action, &QAction::triggered, this, &CategoryMatchesWidget::showResetDialog);
    }

    menu.exec(QCursor::pos() - QPoint(4, 4));
}

