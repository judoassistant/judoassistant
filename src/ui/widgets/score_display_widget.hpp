#pragma once

#include <QFont>
#include <QTimer>
#include <QWidget>

#include "core/stores/match_store.hpp"
#include "core/stores/player_store.hpp"
#include "ui/store_managers/store_manager.hpp"

class CategoryStore;
class QPainter;

enum class ScoreDisplayState {
    INTRODUCTION, NORMAL, WINNER
};

struct FlagImage {
    void update(std::optional<PlayerCountry> country);

    std::optional<QImage> image;
    std::optional<PlayerCountry> country;
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
    static constexpr auto INTRO_INTERVAL = std::chrono::milliseconds(4000);
    static constexpr auto WINNER_INTERVAL = std::chrono::milliseconds(4000);
    static constexpr auto DURATION_INTERVAL = std::chrono::milliseconds(1000);

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
    std::array<FlagImage,2> mFlags;
};

