#include <QPainter>

#include "core/stores/category_store.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/colors.hpp"
#include "ui/widgets/match_card_widget.hpp"

// TODO: Setup minSize and maxSize for match card
MatchCardWidget::MatchCardWidget(const StoreManager &storeManager, QWidget *parent)
    : QWidget(parent)
    , mStoreManager(storeManager)
{
    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &MatchCardWidget::beginResetTournament);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &MatchCardWidget::endResetTournament);

    beginResetTournament();
    endResetTournament();
}

void MatchCardWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QRect boundingRect(0,0,width(),height());
    if (mCombinedId) {
        mMatchCard.paint(&painter, boundingRect, palette());
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

QSize MatchCardWidget::sizeHint() const {
    return mMatchCard.sizeHint();

}

void MatchCardWidget::setMatch(std::optional<std::pair<CategoryId, MatchId>> combinedId) {
    mCombinedId = combinedId;

    if (mCombinedId) {
        const auto &tournament = mStoreManager.getTournament();
        const auto &category = tournament.getCategory(mCombinedId->first);
        const auto &match = category.getMatch(mCombinedId->second);

        mMatchCard = MatchCard(tournament, category, match, mStoreManager.masterTime());
    }

    update(0, 0, width(), height());
}

void MatchCardWidget::beginResetTournament() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }
}

void MatchCardWidget::endResetTournament() {
    auto &tournament = mStoreManager.getTournament();
    mConnections.push(connect(&tournament, &QTournamentStore::matchesChanged, this, &MatchCardWidget::changeMatches));
    mConnections.push(connect(&tournament, &QTournamentStore::playersChanged, this, &MatchCardWidget::changePlayers));
    mConnections.push(connect(&tournament, &QTournamentStore::matchesReset, this, &MatchCardWidget::resetMatches));
    mConnections.push(connect(&tournament, &QTournamentStore::categoriesChanged, this, &MatchCardWidget::changeCategories));

    mCombinedId = std::nullopt;
    update(0, 0, width(), height());
}

void MatchCardWidget::changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds) {
    if (!mCombinedId)
        return;

    if (mCombinedId->first == categoryId && std::find(matchIds.begin(), matchIds.end(), mCombinedId->second) != matchIds.end()) {
        const auto &tournament = mStoreManager.getTournament();
        const auto &category = tournament.getCategory(mCombinedId->first);
        const auto &match = category.getMatch(mCombinedId->second);

        mMatchCard = MatchCard(tournament, category, match, mStoreManager.masterTime());
        update(0, 0, width(), height());
    }
}

void MatchCardWidget::resetMatches(CategoryId categoryId) {
    if (mCombinedId && mCombinedId->first == categoryId) {
        mCombinedId = std::nullopt;
        update(0, 0, width(), height());
    }
}

void MatchCardWidget::changeCategories(std::vector<CategoryId> categoryIds) {
    if (!mCombinedId)
        return;

    auto categoryId = mCombinedId->first;

    if (std::find(categoryIds.begin(), categoryIds.end(), categoryId) != categoryIds.end()) {
        const auto &tournament = mStoreManager.getTournament();
        const auto &category = tournament.getCategory(mCombinedId->first);
        const auto &match = category.getMatch(mCombinedId->second);

        mMatchCard = MatchCard(tournament, category, match, mStoreManager.masterTime());
        update(0, 0, width(), height());
    }
}

void MatchCardWidget::changePlayers(std::vector<PlayerId> playerIds) {
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

    if (changed) {
        const auto &tournament = mStoreManager.getTournament();
        const auto &category = tournament.getCategory(mCombinedId->first);
        const auto &match = category.getMatch(mCombinedId->second);

        mMatchCard = MatchCard(tournament, category, match, mStoreManager.masterTime());
        update(0, 0, width(), height());
    }
}

