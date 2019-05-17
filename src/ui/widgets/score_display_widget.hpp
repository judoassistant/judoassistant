#pragma once

#include <QFont>
#include <QTimer>
#include <QWidget>

#include "core/stores/match_store.hpp"
#include "core/stores/player_store.hpp"
#include "core/stores/preferences_store.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/widgets/scoreboard_painters/scoreboard_painter.hpp"

class CategoryStore;
class QPainter;
class ScoreboardPainter;

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
    static constexpr auto INTRO_INTERVAL = std::chrono::milliseconds(4000);
    static constexpr auto WINNER_INTERVAL = std::chrono::milliseconds(4000);
    static constexpr auto DURATION_INTERVAL = std::chrono::milliseconds(200);

    void durationTimerHit();

    void beginResetTournament();
    void endResetTournament();
    void changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds);
    void changePlayers(std::vector<PlayerId> playerIds);
    void resetMatches(const std::vector<CategoryId> &categoryIds);
    void changeCategories(std::vector<CategoryId> categoryIds);

    const StoreManager &mStoreManager;
    std::stack<QMetaObject::Connection> mConnections;
    std::optional<std::pair<CategoryId, MatchId>> mCombinedId;
    ScoreDisplayState mState;
    QTimer mIntroTimer;
    QTimer mWinnerTimer;
    QTimer mDurationTimer;

    std::unique_ptr<ScoreboardPainter> mScoreboardPainter;
    ScoreboardStylePreference mScoreboardStyle;
};

