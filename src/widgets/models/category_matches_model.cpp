#include <sstream>

#include <QColor>
#include <QBrush>
#include <QTimer>

#include "store_managers/store_manager.hpp"
#include "stores/qtournament_store.hpp"
#include "widgets/models/match_card.hpp"
#include "widgets/models/category_matches_model.hpp"

CategoryMatchesModel::CategoryMatchesModel(StoreManager &storeManager, QObject *parent)
    : QAbstractTableModel(parent)
    , mStoreManager(storeManager)
    , mResettingMatches(false)
{
    beginResetTournament();
    endResetTournament();

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &CategoryMatchesModel::beginResetTournament);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &CategoryMatchesModel::endResetTournament);

    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &CategoryMatchesModel::timerHit);
    timer->start(TIMER_INTERVAL);
}

void CategoryMatchesModel::beginResetMatches() {
    if (mResettingMatches)
        return;
    mResettingMatches = true;
    beginResetModel();

    mMatches.clear();
    mMatchesMap.clear();
    mPlayers.clear();
    mMatchPlayerMap.clear();
    mUnpausedMatches.clear();
}

void CategoryMatchesModel::endResetMatches() {
    const auto &tournament = mStoreManager.getTournament();
    if (mCategoryId && tournament.containsCategory(*mCategoryId)) {
        const auto &category = tournament.getCategory(*mCategoryId);

        for (auto & matchPtr : category.getMatches()) {
            const auto &match = *matchPtr;
            auto matchId = match.getId();

            mMatchesMap[matchId] = mMatches.size();
            mMatches.push_back(matchId);

            if (match.getStatus() == MatchStatus::UNPAUSED)
                mUnpausedMatches.insert(matchId);

            auto whitePlayer = match.getWhitePlayer();
            auto bluePlayer = match.getBluePlayer();
            if (whitePlayer)
                mPlayers[*whitePlayer].insert(matchId);
            if (bluePlayer)
                mPlayers[*bluePlayer].insert(matchId);

            mMatchPlayerMap[matchId] = std::make_pair(whitePlayer, bluePlayer);
        }
    }

    log_debug().field("count", mMatches.size()).msg("Done resetting matches");

    mResettingMatches = false;
    endResetModel();
}

void CategoryMatchesModel::beginResetTournament() {
    beginResetMatches();

    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }
}

void CategoryMatchesModel::endResetTournament() {
    endResetMatches();

    auto &tournament = mStoreManager.getTournament();
    mConnections.push(connect(&tournament, &QTournamentStore::matchesChanged, this, &CategoryMatchesModel::changeMatches));
    mConnections.push(connect(&tournament, &QTournamentStore::playersChanged, this, &CategoryMatchesModel::changePlayers));
    mConnections.push(connect(&tournament, &QTournamentStore::matchesAboutToBeReset, this, &CategoryMatchesModel::beginResetMatches));
    mConnections.push(connect(&tournament, &QTournamentStore::matchesReset, this, &CategoryMatchesModel::endResetMatches));
}


int CategoryMatchesModel::rowCount(const QModelIndex &parent) const {
    return mMatches.size();
}

int CategoryMatchesModel::columnCount(const QModelIndex &parent) const {
    return COLUMN_COUNT;
}

QVariant CategoryMatchesModel::data(const QModelIndex &index, int role) const {
    assert(mCategoryId.has_value());

    MatchId matchId = getMatch(index.row());

    const auto &tournament = mStoreManager.getTournament();
    const auto &category = tournament.getCategory(*mCategoryId);
    const auto &match = category.getMatch(matchId);

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:
                return QVariant::fromValue(MatchCard(tournament, category, match, mStoreManager.masterTime()));
        }
    }

    if (role == Qt::UserRole) { // Used for sorting
        return data(index, Qt::DisplayRole);
    }

    return QVariant();
}

QVariant CategoryMatchesModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:
                    return QString(tr("Match Card"));
            }
        }
    }
    return QVariant();
}

std::vector<MatchId> CategoryMatchesModel::getMatches(const QItemSelection &selection) const {
    std::unordered_set<int> rows;
    for (auto index : selection.indexes())
        rows.insert(index.row());

    std::vector<MatchId> matchIds;
    for (auto row : rows)
        matchIds.push_back(getMatch(row));

    return std::move(matchIds);
}

MatchId CategoryMatchesModel::getMatch(int row) const {
    return mMatches[row];
}

int CategoryMatchesModel::getRow(MatchId matchId) const {
    auto it = mMatchesMap.find(matchId);
    assert(it != mMatchesMap.end());
    return static_cast<int>(it->second);
}

void CategoryMatchesModel::changePlayers(std::vector<PlayerId> playerIds) {
    std::unordered_set<int> changedRows;

    for (auto playerId : playerIds) {
        auto it = mPlayers.find(playerId);
        if (it != mPlayers.end()) {
            for (auto matchId : it->second)
                changedRows.insert(getRow(matchId));
        }
    }

    for (auto row : changedRows)
        emit dataChanged(createIndex(row, 0), createIndex(row,0));
}

void CategoryMatchesModel::changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds) {
    if (mCategoryId != categoryId)
        return;

    const auto &tournament = mStoreManager.getTournament();
    const auto &category = tournament.getCategory(*mCategoryId);

    for (MatchId matchId : matchIds) {
        const auto &match = category.getMatch(matchId);

        // update unpaused matches set
        if (match.getStatus() == MatchStatus::UNPAUSED)
            mUnpausedMatches.insert(matchId);
        else
            mUnpausedMatches.erase(matchId);

        // update player <-> matches maps
        assert(mMatchPlayerMap.find(matchId) != mMatchPlayerMap.end());

        auto mapIt = mMatchPlayerMap[matchId];

        auto whitePlayer = match.getWhitePlayer();
        std::optional<PlayerId> &storedWhitePlayer = mapIt.first;
        if (whitePlayer != storedWhitePlayer) {
            if (storedWhitePlayer)
                mPlayers[*storedWhitePlayer].erase(matchId);
            if (whitePlayer)
                mPlayers[*whitePlayer].insert(matchId);
            storedWhitePlayer = whitePlayer;
        }

        auto bluePlayer = match.getBluePlayer();
        std::optional<PlayerId> &storedBluePlayer = mapIt.second;
        if (bluePlayer != storedBluePlayer) {
            if (storedBluePlayer)
                mPlayers[*storedBluePlayer].erase(matchId);
            if (bluePlayer)
                mPlayers[*bluePlayer].insert(matchId);
            storedBluePlayer = bluePlayer;
        }

        auto row = getRow(matchId);
        emit dataChanged(createIndex(row, 0), createIndex(row,0));
    }
}

void CategoryMatchesModel::timerHit() {
    for (MatchId matchId : mUnpausedMatches) {
        auto row = getRow(matchId);
        emit dataChanged(createIndex(row, 0), createIndex(row,0));
    }
}

void CategoryMatchesModel::setCategory(std::optional<CategoryId> categoryId) {
    log_debug().field("categoryId", categoryId).msg("Setting category");
    beginResetMatches();
    mCategoryId = categoryId;
    endResetMatches();
}

std::optional<CategoryId> CategoryMatchesModel::getCategory() const {
    return mCategoryId;
}
