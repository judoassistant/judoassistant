#pragma once

#include <stack>
#include <unordered_map>
#include <QMetaObject>
#include <QAbstractTableModel>

#include "id.hpp"

enum class MatchStatus;
class StoreManager;
class PlayerStore;

class ResultsModel : public QAbstractTableModel {
    Q_OBJECT
public:
    ResultsModel(StoreManager &storeManager, QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // std::vector<PlayerId> getPlayers(const QItemSelection &selection) const;
    // PlayerId getPlayer(int row) const;

    // int getRow(PlayerId id) const;
    // std::vector<int> getRows(std::vector<PlayerId> id) const;

    void setCategory(std::optional<CategoryId> categoryId);
    std::optional<CategoryId> getCategory() const;
private slots:
    void changePlayers(std::vector<PlayerId> ids);
    void beginErasePlayers(std::vector<PlayerId> ids);
    void beginResetPlayers();

    void beginResetTournament();
    void endResetTournament();

    void beginResetMatches(CategoryId categoryId);
    void endResetMatches(CategoryId categoryId);

    void changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds);
private:
    void beginResetResults();
    void endResetResults();
    const int COLUMN_COUNT = 3;

    StoreManager & mStoreManager;
    std::stack<QMetaObject::Connection> mConnections;
    std::optional<CategoryId> mCategory;

    bool mResetting;
    std::unordered_map<PlayerId, size_t> mPlayers;
    std::unordered_map<MatchId, MatchStatus> mMatches;
    std::vector<std::pair<std::optional<unsigned int>, PlayerId>> mResults;
};

