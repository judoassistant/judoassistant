#pragma once

#include <QWidget>
#include <QFont>

#include "ui/models/match_card.hpp"
#include "ui/store_managers/store_manager.hpp"

class QPainter;

class MatchCardWidget : public QWidget {
    Q_OBJECT
public:
    MatchCardWidget(const StoreManager &storeManager, QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);

    void setMatch(std::optional<std::pair<CategoryId, MatchId>> combinedId);
    QSize sizeHint() const override;

    void beginResetTournament();
    void endResetTournament();
    void changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds);
    void changePlayers(std::vector<PlayerId> playerIds);
    void resetMatches(CategoryId categoryId);
    void changeCategories(std::vector<CategoryId> categoryIds);
private:
    const StoreManager &mStoreManager;
    std::stack<QMetaObject::Connection> mConnections;

    std::optional<std::pair<CategoryId, MatchId>> mCombinedId;
    MatchCard mMatchCard;
};

