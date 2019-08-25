#include <QPainter>

#include "core/log.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/graphics_items/match_graphics_item.hpp"

constexpr int PADDING = 10;
constexpr int COLUMN_TWO_WIDTH = 100;
constexpr int HEADER_HEIGHT = 30;
constexpr int LARGE_FONT_SIZE = 18;

MatchGraphicsItem::MatchGraphicsItem(const StoreManager &storeManager, CategoryId categoryId, MatchId matchId, QRect rect, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , mStoreManager(storeManager)
    , mCategoryId(categoryId)
    , mMatchId(matchId)
    , mRect(rect)
{
}

QRectF MatchGraphicsItem::boundingRect() const {
    return mRect;
}

void MatchGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    const auto &tournament = mStoreManager.getTournament();
    if (!tournament.containsCategory(mCategoryId))
        return;

    const auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;

    const auto &match = category.getMatch(mMatchId);

    painter->save();
    painter->translate(mRect.x(), mRect.y());

    int playerHeight = (mRect.height() - HEADER_HEIGHT) / 2;
    int headerHeight = mRect.height() - 2 * playerHeight;

    QRect headerRect(0, 0, mRect.width(), headerHeight);
    QRect whitePlayerRect(0, headerHeight, mRect.width(), playerHeight);
    QRect bluePlayerRect(0, headerHeight + playerHeight, mRect.width(), playerHeight);

    paintHeader(*painter, headerRect, category, match);
    paintPlayer(*painter, whitePlayerRect, MatchStore::PlayerIndex::WHITE, tournament, category, match);
    paintPlayer(*painter, bluePlayerRect, MatchStore::PlayerIndex::BLUE, tournament, category, match);

    painter->setBrush(Qt::NoBrush);
    painter->setPen(QColor("#2e3440"));
    painter->drawRect(QRect(0, 0, mRect.width(), mRect.height()));

    painter->restore();
}

void MatchGraphicsItem::paintHeader(QPainter &painter, const QRect &rect, const CategoryStore &category, const MatchStore &match) {
    QRect textRect(PADDING, 0, rect.width() - PADDING, rect.height());
    QRect scoreRect(rect.width() - COLUMN_TWO_WIDTH, 0, COLUMN_TWO_WIDTH, rect.height());
    QRect scoreTextRect(rect.width() - COLUMN_TWO_WIDTH + PADDING, 0, COLUMN_TWO_WIDTH - 2 * PADDING, rect.height());

    // Draw background
    painter.setBrush(QColor("#2e3440"));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect);

    // Draw category name
    QFont font("Noto Sans");
    font.setPixelSize((80 * LARGE_FONT_SIZE) / 100);
    font.setBold(true);
    font.setCapitalization(QFont::AllUppercase);
    painter.setFont(font);

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QColor("#ffffff"));
    painter.drawText(textRect, QString::fromStdString(category.getName()), Qt::AlignVCenter | Qt::AlignLeft);

    // Draw Time
    if (match.getStatus() != MatchStatus::NOT_STARTED) {
        painter.setBrush(QColor("#3b4252"));
        painter.setPen(Qt::NoPen);
        painter.drawRect(scoreRect);

        painter.setPen(QColor("#ffffff"));
        font.setPixelSize(LARGE_FONT_SIZE);
        painter.setFont(font);

        if (match.getOsaekomi()) {
            auto osaekomi = std::chrono::floor<std::chrono::seconds>(match.currentOsaekomiTime(mStoreManager.masterTime())).count();
            painter.drawText(scoreTextRect, QString::number(osaekomi), Qt::AlignVCenter | Qt::AlignRight);

            font.setPixelSize((70 * LARGE_FONT_SIZE) / 100);
            painter.setFont(font);
            painter.drawText(scoreTextRect, QString("OSK"), Qt::AlignVCenter | Qt::AlignLeft);
        }
        else {
            std::chrono::seconds time = std::chrono::ceil<std::chrono::seconds>(std::chrono::abs(category.getRuleset().getNormalTime() - match.currentDuration(mStoreManager.masterTime())));
            QString seconds = QString::number((time % std::chrono::minutes(1)).count()).rightJustified(2, '0');
            QString minutes = QString::number(std::chrono::duration_cast<std::chrono::minutes>(time).count());

            painter.drawText(scoreTextRect, QString("%1:%2").arg(minutes,seconds), Qt::AlignVCenter | Qt::AlignRight);

            // Draw Golden Score Indicator
            if (match.isGoldenScore()) {
                font.setPixelSize((70 * LARGE_FONT_SIZE) / 100);
                painter.setFont(font);
                painter.setPen(QColor("#ebcb8b"));
                painter.drawText(scoreTextRect, QString("GS"), Qt::AlignVCenter | Qt::AlignLeft);
            }
        }
    }
}

void MatchGraphicsItem::paintPlayer(QPainter &painter, const QRect &rect, MatchStore::PlayerIndex playerIndex, const TournamentStore &tournament, const CategoryStore &category, const MatchStore &match) {
    QRect nameRect(PADDING, 0, rect.width() - COLUMN_TWO_WIDTH, rect.height() / 2);
    QRect clubRect(PADDING, nameRect.height(), rect.width() - COLUMN_TWO_WIDTH, rect.height() - nameRect.height());
    QRect scoreRect(rect.width() - COLUMN_TWO_WIDTH, 0, COLUMN_TWO_WIDTH, rect.height());
    QRect scoreTextRect(rect.width() - COLUMN_TWO_WIDTH + PADDING, 0, COLUMN_TWO_WIDTH - 2 * PADDING, rect.height());

    int shidoHeight = rect.height() / 3;
    int shidoWidth = (2 * shidoHeight) / 3;
    int shidoOffset = rect.height() / 2 - shidoHeight / 2;
    QRect firstShidoRect(rect.width() - COLUMN_TWO_WIDTH + PADDING, shidoOffset, shidoWidth, shidoHeight);
    QRect secondShidoRect(rect.width() - COLUMN_TWO_WIDTH + PADDING + shidoWidth + 5, shidoOffset, shidoWidth, shidoHeight);

    QLine line(0, 0, rect.width(), 0);

    nameRect.translate(rect.topLeft());
    clubRect.translate(rect.topLeft());
    scoreRect.translate(rect.topLeft());
    scoreTextRect.translate(rect.topLeft());
    line.translate(rect.topLeft());
    firstShidoRect.translate(rect.topLeft());
    secondShidoRect.translate(rect.topLeft());

    // Draw background
    painter.setBrush(playerIndex == MatchStore::PlayerIndex::WHITE ? QColor("#ffffff") : QColor("#1f4fa6"));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect);

    if (match.getStatus() != MatchStatus::NOT_STARTED) {
        const auto &score = match.getScore(playerIndex);
        const auto &otherScore = match.getScore(playerIndex == MatchStore::PlayerIndex::WHITE ? MatchStore::PlayerIndex::BLUE : MatchStore::PlayerIndex::WHITE);
        // Draw Score background
        QFont font("Noto Sans");
        font.setPixelSize(LARGE_FONT_SIZE);
        font.setBold(true);
        painter.setFont(font);

        // Draw Ippon
        if (!score.hansokuMake && (score.ippon || otherScore.hansokuMake)) {
            painter.setBrush(QColor("#fcc949"));
            painter.drawRect(scoreRect);

            painter.setBrush(Qt::NoBrush);
            painter.setPen(QColor("#ffffff"));

            painter.drawText(scoreTextRect, QString("IPPON"), Qt::AlignHCenter | Qt::AlignVCenter);
        }
        else { // Draw Wazari and Shido
            painter.setBrush(QColor("#2e3440"));
            painter.drawRect(scoreRect);

            painter.setBrush(Qt::NoBrush);
            painter.setPen(QColor("#ffffff"));

            painter.drawText(scoreTextRect, QString::number(score.wazari), Qt::AlignRight | Qt::AlignVCenter);

            painter.setPen(Qt::NoPen);
            if (score.hansokuMake) {
                painter.setBrush(QColor("#af1911"));
                painter.drawRect(firstShidoRect);
            }
            else {
                painter.setBrush(QColor("#fced49"));
                if (score.shido >= 1)
                    painter.drawRect(firstShidoRect);
                if (score.shido >= 2)
                    painter.drawRect(secondShidoRect);
            }
        }
    }

    // Draw player
    if (match.getPlayer(playerIndex)) {
        const PlayerStore &player = tournament.getPlayer(*match.getPlayer(playerIndex));

        painter.setBrush(Qt::NoBrush);
        painter.setPen(playerIndex == MatchStore::PlayerIndex::WHITE ? QColor("#363538") : QColor("#ffffff"));

        QFont font("Noto Sans");
        const int fontSize = (3 * nameRect.height()) / 5;
        font.setBold(true);
        font.setPixelSize(fontSize);
        font.setCapitalization(QFont::AllUppercase);
        painter.setFont(font);

        QString name = QString("%1 %2").arg(QString::fromStdString(player.getFirstName()), QString::fromStdString(player.getLastName()));
        painter.drawText(nameRect, name, Qt::AlignLeft | Qt::AlignVCenter);

        // Draw Club
        font.setBold(false);
        font.setCapitalization(QFont::MixedCase);
        painter.setFont(font);
        painter.drawText(clubRect, QString::fromStdString(player.getClub()), Qt::AlignLeft | Qt::AlignVCenter);
    }

    // Draw line
    painter.setPen(playerIndex == MatchStore::PlayerIndex::WHITE ? QColor("#ebcb8b") : QColor("#504c4b"));
    painter.drawLine(line);
}

