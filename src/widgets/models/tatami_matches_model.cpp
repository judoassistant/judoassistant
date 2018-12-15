#include <QColor>
#include <QBrush>
#include <sstream>

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

    std::vector<std::pair<MatchId, bool>> newMatchIds;
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

            newMatchIds.push_back({p.second, isFinished});
        }
    }

    if (!newMatchIds.empty()) {
        if (shouldSignal)
            beginInsertRows(QModelIndex(), mUnfinishedMatches.size(), mUnfinishedMatches.size() + newUnfinishedMatches - 1);
        for (auto p : newMatchIds) {
            const auto &matchId = p.first;
            const auto &isFinished = p.second;
            auto loadingTime = mLoadedMatches.size();

            mLoadedMatches[matchId] = loadingTime;

            if (!isFinished) {
                mUnfinishedMatches.push_back({matchId, isFinished});
                mUnfinishedMatchesSet.insert(matchId);
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
    mConnections.push(connect(&tournament, &QTournamentStore::matchesAboutToBeReset, this, &TatamiMatchesModel::beginResetCategory));

    endResetMatches();
}


int TatamiMatchesModel::rowCount(const QModelIndex &parent) const {
    return mUnfinishedMatches.size();
}

int TatamiMatchesModel::columnCount(const QModelIndex &parent) const {
    return static_cast<int>(COLUMN_COUNT);
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

        if (wasFinished == isFinished) continue; // no need to update

        if (isFinished) {
            didRemoveRows = true;
            auto row = getRow(matchId);
            beginRemoveRows(QModelIndex(), row, row);
            mUnfinishedMatches.erase(mUnfinishedMatches.begin() + row);
            mUnfinishedMatchesSet.erase(matchId);
            endRemoveRows();
        }
        else { // was finished
            auto loadingTime = it->second;

            // Find the first position with a higher loading time (lower bound)
            auto pos = mUnfinishedMatches.begin();
            while (pos != mUnfinishedMatches.end() && pos->second < loadingTime)
                ++pos;

            mUnfinishedMatches.insert(pos, *it);
            mUnfinishedMatchesSet.insert(matchId);
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

