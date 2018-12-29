#pragma once

#include <QWidget>
#include <QFont>
#include <QTimer>
#include "store_managers/store_manager.hpp"
#include "stores/match_store.hpp"

class CategoryStore;
class PlayerStore;
class QPainter;

enum class ScoreDisplayState {
    INTRODUCTION, NORMAL, WINNER
};

class ScoreDisplayWidget : public QWidget {
    Q_OBJECT
public:
    ScoreDisplayWidget(const StoreManager &mStoreManager, QWidget *parent = nullptr);

    void setMatch(std::optional<std::pair<CategoryId, MatchId>> combinedId, bool showIntro = true);
    void setState(ScoreDisplayState state);
    void paintEvent(QPaintEvent *event);
private:
    static const int PADDING = 5;
    static const int INTRO_INTERVAL = 4000;
    static const int WINNER_INTERVAL = 4000;
    static const int DURATION_INTERVAL = 1000;

    void durationTimerHit();

    void paintNullMatch(QPainter &painter);

    void paintPlayerIntroduction(QRect rect, MatchStore::PlayerIndex playerIndex, QPainter &painter, const MatchStore &match, const PlayerStore &player);
    void paintLowerIntroduction(QRect rect, QPainter &painter, const CategoryStore &category, const MatchStore &match);

    void paintPlayerNormal(QRect rect, MatchStore::PlayerIndex playerIndex, QPainter &painter, const MatchStore &match, const PlayerStore &player);
    void paintLowerNormal(QRect rect, QPainter &painter, const CategoryStore &category, const MatchStore &match);

    void beginResetTournament();
    void endResetTournament();
    void changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds);
    void changePlayers(std::vector<PlayerId> playerIds);
    void resetMatches(CategoryId categoryId);
    void changeCategories(std::vector<CategoryId> categoryIds);

    const StoreManager &mStoreManager;
    std::stack<QMetaObject::Connection> mConnections;
    std::optional<std::pair<CategoryId, MatchId>> mCombinedId;
    ScoreDisplayState mState;
    QFont mFont;
    QTimer mIntroTimer;
    QTimer mWinnerTimer;
    QTimer mDurationTimer;
};

