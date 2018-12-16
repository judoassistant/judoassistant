#pragma once

#include <QWidget>
#include <QFont>
#include "store_managers/store_manager.hpp"
#include "stores/match_store.hpp"
#include "stores/match_event.hpp"

class CategoryStore;
class PlayerStore;
class QPainter;

enum class ScoreDisplayState {
    INTRODUCTION, NORMAL, FINISHED
};

class ScoreDisplayWidget : public QWidget {
    Q_OBJECT
public:
    ScoreDisplayWidget(const StoreManager &mStoreManager, QWidget *parent = nullptr);

    void setMatch(std::optional<std::pair<CategoryId, MatchId>> combinedId);
    void paintEvent(QPaintEvent *event);
private:
    static const int PADDING = 5;

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
    std::optional<std::pair<CategoryId, MatchId>> mCombinedId;
    ScoreDisplayState mState;
    QFont mFont;
};

