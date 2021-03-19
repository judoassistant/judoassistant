#pragma once

#include <stack>
#include <QWidget>
#include <QFont>

#include "ui/store_managers/store_manager.hpp"

class QPainter;

class MatchWidget : public QWidget {
    Q_OBJECT
public:
    MatchWidget(const StoreManager &storeManager, QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override;

    void setMatch(std::optional<CombinedId> combinedId);
    QSize sizeHint() const override;

    void beginResetTournament();
    void endResetTournament();
    void changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds);
    void changePlayers(std::vector<PlayerId> playerIds);
    void resetMatches(const std::vector<CategoryId> &categoryIds);
    void changeCategories(std::vector<CategoryId> categoryIds);
private:
    const StoreManager &mStoreManager;
    std::stack<QMetaObject::Connection> mConnections;

    std::optional<CombinedId> mCombinedId;
};

