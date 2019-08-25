#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "core/stores/category_store.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/colors.hpp"
#include "ui/widgets/graphics_items/match_graphics_item.hpp"
#include "ui/widgets/match_widget.hpp"

// TODO: Setup minSize and maxSize for match card
MatchWidget::MatchWidget(const StoreManager &storeManager, QWidget *parent)
    : QWidget(parent)
    , mStoreManager(storeManager)
{
    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &MatchWidget::beginResetTournament);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &MatchWidget::endResetTournament);

    beginResetTournament();
    endResetTournament();
}

void MatchWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QStyleOptionGraphicsItem styleOptions;
    if (mCombinedId) {
        MatchGraphicsItem graphicsItem(mStoreManager, mCombinedId->first, mCombinedId->second, rect());
        graphicsItem.paint(&painter, &styleOptions, this);
    }
    else {
        // Show no next match available
        painter.setRenderHint(QPainter::Antialiasing, false);
        int padding = 5;
        int lineWidth = 1;
        QFont font("Noto Sans");
        // Draw bounding rect

        QPen pen;
        pen.setWidth(1);
        pen.setStyle(Qt::SolidLine);
        pen.setColor(palette().color(QPalette::Shadow).darker());
        painter.setPen(pen);
        painter.setBrush(palette().color(QPalette::Base));
        QRect rect(padding, padding, width()-padding*2, height()-padding*2);
        painter.drawRect(rect);

        pen.setWidth(lineWidth);
        pen.setColor(palette().color(QPalette::Text));
        painter.setPen(pen);
        painter.setBrush(palette().color(QPalette::Base));
        painter.drawText(rect, Qt::AlignCenter, tr("There are no more matches on this tatami"));
    }
}

QSize MatchWidget::sizeHint() const {
    return QSize(MatchGraphicsItem::WIDTH_HINT, MatchGraphicsItem::HEIGHT_HINT);
}

void MatchWidget::setMatch(std::optional<std::pair<CategoryId, MatchId>> combinedId) {
    mCombinedId = combinedId;

    update(0, 0, width(), height());
}

void MatchWidget::beginResetTournament() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }
}

void MatchWidget::endResetTournament() {
    auto &tournament = mStoreManager.getTournament();
    mConnections.push(connect(&tournament, &QTournamentStore::matchesChanged, this, &MatchWidget::changeMatches));
    mConnections.push(connect(&tournament, &QTournamentStore::playersChanged, this, &MatchWidget::changePlayers));
    mConnections.push(connect(&tournament, &QTournamentStore::matchesReset, this, &MatchWidget::resetMatches));
    mConnections.push(connect(&tournament, &QTournamentStore::categoriesChanged, this, &MatchWidget::changeCategories));

    mCombinedId = std::nullopt;
    update(0, 0, width(), height());
}

void MatchWidget::changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds) {
    if (!mCombinedId)
        return;

    if (mCombinedId->first == categoryId && std::find(matchIds.begin(), matchIds.end(), mCombinedId->second) != matchIds.end())
        update(0, 0, width(), height());
}

void MatchWidget::resetMatches(const std::vector<CategoryId> &categoryIds) {
    if (!mCombinedId)
        return;

    for (auto categoryId : categoryIds) {
        if (mCombinedId->first == categoryId) {
            mCombinedId = std::nullopt;
            update(0, 0, width(), height());
            return;
        }
    }
}

void MatchWidget::changeCategories(std::vector<CategoryId> categoryIds) {
    if (!mCombinedId)
        return;

    auto categoryId = mCombinedId->first;

    if (std::find(categoryIds.begin(), categoryIds.end(), categoryId) != categoryIds.end())
        update(0, 0, width(), height());
}

void MatchWidget::changePlayers(std::vector<PlayerId> playerIds) {
    if (!mCombinedId)
        return;

    const auto &tournament = mStoreManager.getTournament();
    const auto &category = tournament.getCategory(mCombinedId->first);
    const auto &match = category.getMatch(mCombinedId->second);

    bool changed = false;
    for (auto playerId : playerIds) {
        if (match.getWhitePlayer() == playerId || match.getBluePlayer() == playerId) {
            changed = true;
            break;
        }
    }

    if (changed)
        update(0, 0, width(), height());
}

