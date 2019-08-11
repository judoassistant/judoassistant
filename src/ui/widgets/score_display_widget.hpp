#pragma once

#include <stack>
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

enum class ScoreDisplayState {
    INTRODUCTION, NORMAL, WINNER
};

class ScoreDisplayWidget : public QWidget {
    Q_OBJECT
public:
    ScoreDisplayWidget(const StoreManager &storeManager, QWidget *parent = nullptr);

    void setMatch(std::optional<std::pair<CategoryId, MatchId>> combinedId, bool showIntro = true);
    void setState(ScoreDisplayState state);

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

protected:
    std::optional<std::pair<CategoryId, MatchId>> mCombinedId;
    ScoreDisplayState mState;
    std::unique_ptr<ScoreboardPainter> mScoreboardPainter;
    ScoreboardStylePreference mScoreboardStyle;

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
    void changePreferences();
    void loadPainter();

    const StoreManager &mStoreManager;
    std::stack<QMetaObject::Connection> mConnections;
    QTimer mIntroTimer;
    QTimer mWinnerTimer;
    QTimer mDurationTimer;
};

