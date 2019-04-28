#include <sstream>

#include <QColor>
#include <QBrush>
#include <QTimer>

#include "core/log.hpp"
#include "core/stores/category_store.hpp"
#include "ui/models/match_card.hpp"
#include "ui/models/tatami_matches_model.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"

TatamiMatchesModel::TatamiMatchesModel(StoreManager &storeManager, TatamiLocation tatami, size_t rowCap, QObject *parent)
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
    mLoadedGroups.clear();

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
    const auto &tournament = mStoreManager.getTournament();
    const auto &tatamis = tournament.getTatamis();
    if (!tatamis.containsTatami(mTatami))
        return;
    const auto &tatami = tatamis.at(mTatami);

    struct MatchInfo {
        CategoryId categoryId;
        MatchId matchId;
        MatchStatus status;
        std::optional<PlayerId> whitePlayer;
        std::optional<PlayerId> bluePlayer;
        bool osaekomi;
        bool bye;
    };

    std::vector<MatchInfo> newMatches;
    size_t newUnfinishedMatches = 0;

    while (mUnfinishedMatches.size() + newMatches.size() < mRowCap) {
        if (mLoadedGroups.size() == tatami.groupCount())
            break;

        auto handle = tatami.getHandle(mLoadedGroups.size());
        mLoadedGroups.insert(handle.id);
        const auto &group = tatami.at(handle);

        for (const auto &p : group.getMatches()) {
            MatchInfo matchInfo;
            auto &category = tournament.getCategory(p.first);
            auto &match = category.getMatch(p.second);

            matchInfo.categoryId = p.first;
            matchInfo.matchId = p.second;
            matchInfo.status = match.getStatus();
            matchInfo.whitePlayer = match.getWhitePlayer();
            matchInfo.bluePlayer = match.getBluePlayer();
            matchInfo.bye = match.isBye();
            matchInfo.osaekomi = match.getOsaekomi().has_value();

            if (!matchInfo.bye && matchInfo.status != MatchStatus::FINISHED)
                ++newUnfinishedMatches;

            newMatches.push_back(std::move(matchInfo));
        }
    }

    if (!newMatches.empty()) {
        if (shouldSignal)
            beginInsertRows(QModelIndex(), mUnfinishedMatches.size(), mUnfinishedMatches.size() + newUnfinishedMatches - 1);
        for (const MatchInfo &matchInfo : newMatches) {
            auto loadingTime = mLoadedMatches.size();
            auto combinedId = std::make_pair(matchInfo.categoryId, matchInfo.matchId);
            mLoadedMatches[combinedId] = loadingTime;

            if (!matchInfo.bye && matchInfo.status != MatchStatus::FINISHED) {
                mUnfinishedMatches.push_back(std::make_tuple(matchInfo.categoryId, matchInfo.matchId, loadingTime));
                mUnfinishedMatchesSet.insert(combinedId);

                if (matchInfo.status == MatchStatus::UNPAUSED || matchInfo.osaekomi)
                    mUnpausedMatches.insert(combinedId);

                mUnfinishedMatchesPlayersInv[combinedId] = {matchInfo.whitePlayer, matchInfo.bluePlayer};
                if (matchInfo.whitePlayer)
                    mUnfinishedMatchesPlayers[*matchInfo.whitePlayer].insert(combinedId);
                if (matchInfo.bluePlayer)
                    mUnfinishedMatchesPlayers[*matchInfo.bluePlayer].insert(combinedId);
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
    CategoryId categoryId;
    MatchId matchId;
    std::tie(categoryId, matchId) = getMatch(index.row());

    const auto &tournament = mStoreManager.getTournament();
    const auto &category = tournament.getCategory(categoryId);
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

std::vector<std::pair<CategoryId, MatchId>> TatamiMatchesModel::getMatches(const QItemSelection &selection) const {
    std::unordered_set<int> rows;
    for (auto index : selection.indexes())
        rows.insert(index.row());

    std::vector<std::pair<CategoryId, MatchId>> matchIds;
    for (auto row : rows)
        matchIds.push_back(getMatch(row));

    return std::move(matchIds);
}

std::pair<CategoryId, MatchId> TatamiMatchesModel::getMatch(int row) const {
    return {std::get<0>(mUnfinishedMatches[row]), std::get<1>(mUnfinishedMatches[row])};
}

int TatamiMatchesModel::getRow(CategoryId categoryId, MatchId matchId) const {
    size_t row = 0;
    for (const auto & p : mUnfinishedMatches) {
        if (std::get<0>(p) == categoryId && std::get<1>(p) == matchId)
            break;
        ++row;
    }

    return row;
}

int TatamiMatchesModel::getRow(std::pair<CategoryId, MatchId> combinedId) const {
    return getRow(std::get<0>(combinedId), std::get<1>(combinedId));
}

void TatamiMatchesModel::changePlayers(const std::vector<PlayerId> &playerIds) {
    std::unordered_set<int> changedRows;

    for (auto playerId : playerIds) {
        auto it = mUnfinishedMatchesPlayers.find(playerId);
        if (it != mUnfinishedMatchesPlayers.end()) {
            for (auto combinedId : it->second)
                changedRows.insert(getRow(combinedId));
        }
    }

    for (auto row : changedRows)
        emit dataChanged(createIndex(row, 0), createIndex(row,0));
}

void TatamiMatchesModel::changeMatches(CategoryId categoryId, const std::vector<MatchId> &matchIds) {
    bool didRemoveRows = false;
    for (auto matchId : matchIds) {
        auto combinedId = std::make_pair(categoryId, matchId);
        auto it = mLoadedMatches.find(combinedId);
        if (it == mLoadedMatches.end())
            continue;

        const auto &category = mStoreManager.getTournament().getCategory(categoryId);
        const auto &match = category.getMatch(matchId);

        bool wasFinished = (mUnfinishedMatchesSet.find(combinedId) == mUnfinishedMatchesSet.end());
        bool isFinished = (match.isBye() || match.getStatus() == MatchStatus::FINISHED);

        if (match.getStatus() == MatchStatus::UNPAUSED || match.getOsaekomi().has_value())
            mUnpausedMatches.insert(combinedId);
        else
            mUnpausedMatches.erase(combinedId);

        if (!isFinished && !wasFinished) {
            // May need to update players
            auto it = mUnfinishedMatchesPlayersInv.find(combinedId);
            assert(it != mUnfinishedMatchesPlayersInv.end());
            auto prevWhitePlayer = it->second.first;
            auto prevBluePlayer = it->second.second;

            if (match.getWhitePlayer() != prevWhitePlayer) {
                if (prevWhitePlayer) {
                    auto it1 = mUnfinishedMatchesPlayers.find(*prevWhitePlayer);
                    assert(it1 != mUnfinishedMatchesPlayers.end());
                    auto &set = it1->second;
                    set.erase(combinedId);

                    if (set.empty())
                        mUnfinishedMatchesPlayers.erase(it1);
                }

                if (match.getWhitePlayer()) {
                    mUnfinishedMatchesPlayers[*(match.getWhitePlayer())].insert(combinedId);
                }
            }

            if (match.getBluePlayer() != prevBluePlayer) {
                if (prevBluePlayer) {
                    auto it1 = mUnfinishedMatchesPlayers.find(*prevBluePlayer);
                    assert(it1 != mUnfinishedMatchesPlayers.end());
                    auto &set = it1->second;
                    set.erase(combinedId);

                    if (set.empty())
                        mUnfinishedMatchesPlayers.erase(it1);
                }

                if (match.getBluePlayer()) {
                    mUnfinishedMatchesPlayers[*(match.getBluePlayer())].insert(combinedId);
                }
            }

            if (match.getWhitePlayer() != prevWhitePlayer || match.getBluePlayer() != prevBluePlayer) {
                mUnfinishedMatchesPlayersInv[combinedId] = {match.getWhitePlayer(), match.getBluePlayer()};
            }

            auto row = getRow(combinedId);
            emit dataChanged(createIndex(row, 0), createIndex(row,0));
        }
        else if (isFinished && !wasFinished) {
            didRemoveRows = true;
            auto row = getRow(combinedId);
            beginRemoveRows(QModelIndex(), row, row);

            mUnfinishedMatches.erase(mUnfinishedMatches.begin() + row);
            mUnfinishedMatchesSet.erase(combinedId);

            auto it = mUnfinishedMatchesPlayersInv.find(combinedId);
            auto prevWhitePlayer = it->second.first;
            if (prevWhitePlayer) {
                auto it1 = mUnfinishedMatchesPlayers.find(*prevWhitePlayer);
                assert(it1 != mUnfinishedMatchesPlayers.end());
                auto & set = it1->second;
                set.erase(combinedId);

                if (set.empty())
                    mUnfinishedMatchesPlayers.erase(it1);
            }

            auto prevBluePlayer = it->second.second;
            if (prevBluePlayer) {
                auto it1 = mUnfinishedMatchesPlayers.find(*prevBluePlayer);
                assert(it1 != mUnfinishedMatchesPlayers.end());
                auto & set = it1->second;
                set.erase(combinedId);

                if (set.empty())
                    mUnfinishedMatchesPlayers.erase(it1);
            }
            mUnfinishedMatchesPlayersInv.erase(it);

            endRemoveRows();
        }
        else if (!isFinished && wasFinished) {
            auto loadingTime = it->second;

            // Find the first position with a higher loading time (lower bound)
            auto pos = mUnfinishedMatches.begin();
            int row = 0;
            while (pos != mUnfinishedMatches.end() && std::get<2>(*pos) < loadingTime) {
                ++pos;
                ++row;
            }

            beginInsertRows(QModelIndex(), row, row);
            mUnfinishedMatches.insert(pos, std::make_tuple(categoryId, matchId, loadingTime));
            mUnfinishedMatchesSet.insert(combinedId);

            if (match.getWhitePlayer())
                mUnfinishedMatchesPlayers[*(match.getWhitePlayer())].insert(combinedId);
            if (match.getBluePlayer())
                mUnfinishedMatchesPlayers[*(match.getBluePlayer())].insert(combinedId);
            mUnfinishedMatchesPlayersInv[combinedId] = {match.getWhitePlayer(), match.getBluePlayer()};

            endInsertRows();
        }
    }

    if (didRemoveRows)
        loadBlocks();
}

void TatamiMatchesModel::changeTatamis(const std::vector<BlockLocation> &locations, const std::vector<std::pair<CategoryId, MatchType>> &blocks) {
    const auto &tournament = mStoreManager.getTournament();
    const auto &tatamis = tournament.getTatamis();
    if (!tatamis.containsTatami(mTatami))
        return;
    const auto &tatami = tatamis.at(mTatami);

    bool shouldReset = false;
    bool shouldLoad = false;

    for (BlockLocation location : locations) {
        if (!mTatami.equiv(location.sequentialGroup.concurrentGroup.tatami)) continue;

        if (mResettingMatches) {
            shouldReset = true;
            break;
        }

        auto handle = location.sequentialGroup.concurrentGroup.handle;
        if (mLoadedGroups.find(handle.id) != mLoadedGroups.end()) {
            shouldReset = true;
            break;
        }

        if (tatami.containsGroup(handle)) {
            if (tatami.getIndex(handle) < mLoadedGroups.size()) {
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
        if (mLoadedMatches.find({categoryId, match->getId()}) != mLoadedMatches.end()) {
            beginResetMatches(); // Let the tatamiChanged call endResetMatches()
            return;
        }
    }
}

void TatamiMatchesModel::timerHit() {
    for (auto combinedId : mUnpausedMatches) {
        auto row = getRow(combinedId);
        emit dataChanged(createIndex(row, 0), createIndex(row,0));
    }
}

