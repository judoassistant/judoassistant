#include <sstream>

#include <QColor>
#include <QBrush>
#include <QTimer>

#include "store_managers/store_manager.hpp"
#include "stores/qtournament_store.hpp"
#include "stores/category_store.hpp"
#include "widgets/models/match_card.hpp"
#include "widgets/models/tatami_matches_model.hpp"

TatamiMatchesModel::TatamiMatchesModel(StoreManager &storeManager, size_t tatami, size_t rowCap, QObject *parent)
    : QAbstractTableModel(parent)
    , mStoreManager(storeManager)
    , mTatami(tatami)
    , mRowCap(rowCap)
    , mResettingMatches(false)
{
    beginResetTournament();
    endResetTournament();

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &TatamiMatchesModel::beginResetTournament);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &TatamiMatchesModel::endResetTournament);

    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TatamiMatchesModel::timerHit);
    timer->start(TIMER_INTERVAL);
}

void TatamiMatchesModel::beginResetMatches() {
    if (mResettingMatches)
        return;
    mResettingMatches = true;
    beginResetModel();

    mLoadedMatches.clear();
    mLoadedBlocks.clear();

    mUnfinishedMatches.clear();
    mUnfinishedMatchesSet.clear();
    mUnfinishedMatchesPlayers.clear();
    mUnfinishedMatchesPlayersInv.clear();
    mUnpausedMatches.clear();
}

void TatamiMatchesModel::endResetMatches() {
    loadBlocks(false);
    mResettingMatches = false;
    endResetModel();
}

void TatamiMatchesModel::beginResetTournament() {
    beginResetMatches();

    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }
}

void TatamiMatchesModel::loadBlocks(bool shouldSignal) {
    auto &tournament = mStoreManager.getTournament();
    auto &tatamis = tournament.getTatamis();
    if (tatamis.tatamiCount() <= mTatami)
        return;
    auto &tatami = tatamis[mTatami];

    std::vector<std::tuple<MatchId, bool, bool, std::optional<PlayerId>, std::optional<PlayerId>>> newMatchIds;
    size_t newUnfinishedMatches = 0;

    while (mUnfinishedMatches.size() + newMatchIds.size() < mRowCap) {
        if (mLoadedBlocks.size() == tatami.groupCount())
            break;

        auto handle = tatami.getHandle(mLoadedBlocks.size());
        mLoadedBlocks.insert(handle.id);
        auto &block = tatami.getGroup(handle);

        for (const auto &p : block.getMatches()) {
            auto &category = tournament.getCategory(p.first);
            auto &ruleset = category.getRuleset();
            auto &match = category.getMatch(p.second);

            bool isFinished = ruleset.isFinished(match);
            if (!isFinished)
                ++newUnfinishedMatches;

            newMatchIds.push_back(std::make_tuple(p.second, isFinished, match.isStopped(), match.getWhitePlayer(), match.getBluePlayer()));
        }
    }

    if (!newMatchIds.empty()) {
        if (shouldSignal)
            beginInsertRows(QModelIndex(), mUnfinishedMatches.size(), mUnfinishedMatches.size() + newUnfinishedMatches - 1);
        for (auto t : newMatchIds) {
            const auto &matchId = std::get<0>(t);
            const auto &isFinished = std::get<1>(t);
            const auto &isStopped = std::get<2>(t);
            const auto &whitePlayer = std::get<3>(t);
            const auto &bluePlayer = std::get<4>(t);
            auto loadingTime = mLoadedMatches.size();

            mLoadedMatches[matchId] = loadingTime;

            if (!isFinished) {
                mUnfinishedMatches.push_back({matchId, isFinished});
                mUnfinishedMatchesSet.insert(matchId);

                if (!isStopped)
                    mUnpausedMatches.insert(matchId);

                mUnfinishedMatchesPlayersInv[matchId] = {whitePlayer, bluePlayer};
                if (whitePlayer)
                    mUnfinishedMatchesPlayers[*whitePlayer].insert(matchId);
                if (bluePlayer)
                    mUnfinishedMatchesPlayers[*bluePlayer].insert(matchId);
            }
        }
        if (shouldSignal)
            endInsertRows();
    }
}

void TatamiMatchesModel::endResetTournament() {
    auto &tournament = mStoreManager.getTournament();
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisChanged, this, &TatamiMatchesModel::changeTatamis));
    mConnections.push(connect(&tournament, &QTournamentStore::matchesChanged, this, &TatamiMatchesModel::changeMatches));
    mConnections.push(connect(&tournament, &QTournamentStore::playersChanged, this, &TatamiMatchesModel::changePlayers));
    mConnections.push(connect(&tournament, &QTournamentStore::matchesAboutToBeReset, this, &TatamiMatchesModel::beginResetCategory));

    endResetMatches();
}


int TatamiMatchesModel::rowCount(const QModelIndex &parent) const {
    return mUnfinishedMatches.size();
}

int TatamiMatchesModel::columnCount(const QModelIndex &parent) const {
    return COLUMN_COUNT;
}

QVariant TatamiMatchesModel::data(const QModelIndex &index, int role) const {
    auto matchId = getMatch(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:
                return QVariant::fromValue(MatchCard(std::nullopt, std::nullopt));
        }
    }

    if (role == Qt::UserRole) { // Used for sorting
        return data(index, Qt::DisplayRole);
    }

    return QVariant();
}

QVariant TatamiMatchesModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

std::vector<MatchId> TatamiMatchesModel::getMatches(const QItemSelection &selection) const {
    std::unordered_set<int> rows;
    for (auto index : selection.indexes())
        rows.insert(index.row());

    std::vector<MatchId> matchIds;
    for (auto row : rows)
        matchIds.push_back(getMatch(row));

    return std::move(matchIds);
}

MatchId TatamiMatchesModel::getMatch(int row) const {
    return mUnfinishedMatches[row].first;
}

int TatamiMatchesModel::getRow(MatchId id) const {
    size_t row = 0;
    for (const auto & p : mUnfinishedMatches) {
        if (p.first == id)
            break;
        ++row;
    }

    return row;
}

void TatamiMatchesModel::changePlayers(std::vector<PlayerId> playerIds) {
    std::unordered_set<int> changedRows;

    for (auto playerId : playerIds) {
        auto it = mUnfinishedMatchesPlayers.find(playerId);
        if (it != mUnfinishedMatchesPlayers.end()) {
            for (auto matchId : it->second)
                changedRows.insert(getRow(matchId));
        }
    }

    for (auto row : changedRows)
        emit dataChanged(createIndex(row, 0), createIndex(row,0));
}

void TatamiMatchesModel::changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds) {
    bool didRemoveRows = false;
    for (auto matchId : matchIds) {
        auto it = mLoadedMatches.find(matchId);
        if (it == mLoadedMatches.end())
            continue;

        const auto &category = mStoreManager.getTournament().getCategory(categoryId);
        const auto &match = category.getMatch(matchId);
        const auto &ruleset = category.getRuleset();

        auto wasFinished = (mUnfinishedMatchesSet.find(matchId) == mUnfinishedMatchesSet.end());
        auto isFinished = ruleset.isFinished(match);

        if (match.isStopped())
            mUnpausedMatches.erase(matchId);
        else
            mUnpausedMatches.insert(matchId);

        if (!isFinished && !wasFinished) {
            // May need to update players
            auto it = mUnfinishedMatchesPlayersInv.find(matchId);
            auto prevWhitePlayer = it->second.first;
            auto prevBluePlayer = it->second.second;

            if (match.getWhitePlayer() != prevWhitePlayer) {
                if (prevWhitePlayer) {
                    auto it1 = mUnfinishedMatchesPlayers.find(*prevWhitePlayer);
                    auto & set = it1->second;
                    set.erase(matchId);

                    if (set.empty())
                        mUnfinishedMatchesPlayers.erase(it1);
                }

                if (match.getWhitePlayer()) {
                    mUnfinishedMatchesPlayers[*(match.getWhitePlayer())].insert(matchId);
                }
            }

            if (match.getBluePlayer() != prevBluePlayer) {
                if (prevBluePlayer) {
                    auto it1 = mUnfinishedMatchesPlayers.find(*prevBluePlayer);
                    auto & set = it1->second;
                    set.erase(matchId);

                    if (set.empty())
                        mUnfinishedMatchesPlayers.erase(it1);
                }

                if (match.getBluePlayer()) {
                    mUnfinishedMatchesPlayers[*(match.getBluePlayer())].insert(matchId);
                }
            }

            if (match.getWhitePlayer() != prevWhitePlayer || match.getBluePlayer() != prevBluePlayer) {
                mUnfinishedMatchesPlayersInv[matchId] = {match.getWhitePlayer(), match.getBluePlayer()};
            }

            auto row = getRow(matchId);
            emit dataChanged(createIndex(row, 0), createIndex(row,0));
        }
        else if (isFinished) {
            didRemoveRows = true;
            auto row = getRow(matchId);
            beginRemoveRows(QModelIndex(), row, row);

            mUnfinishedMatches.erase(mUnfinishedMatches.begin() + row);
            mUnfinishedMatchesSet.erase(matchId);

            auto it = mUnfinishedMatchesPlayersInv.find(matchId);
            auto prevWhitePlayer = it->second.first;
            if (prevWhitePlayer) {
                auto it1 = mUnfinishedMatchesPlayers.find(*prevWhitePlayer);
                auto & set = it1->second;
                set.erase(matchId);

                if (set.empty())
                    mUnfinishedMatchesPlayers.erase(it1);
            }

            auto prevBluePlayer = it->second.first;
            if (prevBluePlayer) {
                auto it1 = mUnfinishedMatchesPlayers.find(*prevWhitePlayer);
                auto & set = it1->second;
                set.erase(matchId);

                if (set.empty())
                    mUnfinishedMatchesPlayers.erase(it1);
            }
            mUnfinishedMatchesPlayersInv.erase(it);

            endRemoveRows();
        }
        else { // was finished
            auto loadingTime = it->second;

            // Find the first position with a higher loading time (lower bound)
            auto pos = mUnfinishedMatches.begin();
            int row = 0;
            while (pos != mUnfinishedMatches.end() && pos->second < loadingTime) {
                ++pos;
                ++row;
            }

            beginInsertRows(QModelIndex(), row, row);
            mUnfinishedMatches.insert(pos, *it);
            mUnfinishedMatchesSet.insert(matchId);

            if (match.getWhitePlayer())
                mUnfinishedMatchesPlayers[*(match.getWhitePlayer())].insert(matchId);
            if (match.getBluePlayer())
                mUnfinishedMatchesPlayers[*(match.getBluePlayer())].insert(matchId);
            mUnfinishedMatchesPlayersInv[matchId] = {match.getWhitePlayer(), match.getBluePlayer()};

            endInsertRows();
        }
    }

    if (didRemoveRows)
        loadBlocks();
}

void TatamiMatchesModel::changeTatamis(std::vector<TatamiLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks) {
    auto &tournament = mStoreManager.getTournament();
    auto &tatamis = tournament.getTatamis();
    if (tatamis.tatamiCount() <= mTatami)
        return;
    auto &tatami = tatamis[mTatami];

    bool shouldReset = false;
    bool shouldLoad = false;

    for (TatamiLocation location : locations) {
        if (location.tatamiIndex != mTatami) continue;

        if (mLoadedBlocks.find(location.concurrentGroup.id) != mLoadedBlocks.end()) {
            shouldReset = true;
            break;
        }

        if (tatami.containsGroup(location.concurrentGroup)) {
            if (tatami.getIndex(location.concurrentGroup) > mLoadedBlocks.size()) {
                shouldReset = true;
                break;
            }

            if (mUnfinishedMatches.size() < mRowCap) {
                shouldLoad = true;
            }
        }
    }

    if (shouldReset) {
        log_debug().field("tatami", mTatami).msg("Resetting tatami matches");
        beginResetMatches();
        endResetMatches();
    }
    else if (shouldLoad) {
        log_debug().field("tatami", mTatami).msg("Loading tatami matches");
        loadBlocks();
    }

}

void TatamiMatchesModel::beginResetCategory(CategoryId categoryId) {
    const auto &tournament = mStoreManager.getTournament();
    const auto &category = tournament.getCategory(categoryId);
    for (const auto &match : category.getMatches()) {
        if (mLoadedMatches.find(match->getId()) != mLoadedMatches.end()) {
            beginResetMatches(); // Let the tatamiChanged call endResetMatches()
            return;
        }
    }
}

void TatamiMatchesModel::timerHit() {
    if (!mUnpausedMatches.empty())
        log_debug().field("matches", mUnpausedMatches).msg("Timer hit");
    for (auto matchId : mUnpausedMatches) {
        auto row = getRow(matchId);
        emit dataChanged(createIndex(row, 0), createIndex(row,0));
    }
}
