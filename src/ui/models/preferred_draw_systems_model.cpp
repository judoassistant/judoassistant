#include "core/actions/change_draw_system_preference_identifier_action.hpp"
#include "core/actions/change_draw_system_preference_limit_action.hpp"
#include "ui/models/preferred_draw_systems_model.hpp"
#include "ui/store_managers/store_manager.hpp"

PreferredDrawSystemsModel::PreferredDrawSystemsModel(StoreManager &storeManager, QObject *parent)
    : mStoreManager(storeManager)
{
    beginResetTournament();
    endResetTournament();

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &PreferredDrawSystemsModel::beginResetTournament);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &PreferredDrawSystemsModel::endResetTournament);
}

void PreferredDrawSystemsModel::beginResetTournament() {
    beginResetModel();

    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }
}

void PreferredDrawSystemsModel::endResetTournament() {
    auto &tournament = mStoreManager.getTournament();

    mConnections.push(connect(&tournament, &QTournamentStore::preferencesChanged, this, &PreferredDrawSystemsModel::changePreferences));

    mPreferredDrawSystems = tournament.getPreferences().getPreferredDrawSystems();
    endResetModel();

}

void PreferredDrawSystemsModel::changePreferences() {
    beginResetModel();
    auto &tournament = mStoreManager.getTournament();
    mPreferredDrawSystems = tournament.getPreferences().getPreferredDrawSystems();
    endResetModel();
}

int PreferredDrawSystemsModel::rowCount(const QModelIndex &parent) const {
    return mPreferredDrawSystems.size();
}

int PreferredDrawSystemsModel::columnCount(const QModelIndex &parent) const {
    return COLUMN_COUNT;
}

QVariant PreferredDrawSystemsModel::data(const QModelIndex &index, int role) const {
    auto preference = mPreferredDrawSystems[index.row()];

    if (role == Qt::DisplayRole) {
        if (index.column() == 0)
            return static_cast<int>(preference.playerLowerLimit);
        if (index.column() == 1) {
            const auto &drawSystem = DrawSystem::getDrawSystem(preference.drawSystem);
            return QString::fromStdString(drawSystem->getName());
        }
    }
    if (role == Qt::EditRole) {
        if (index.column() == 0)
            return static_cast<int>(preference.playerLowerLimit);
        if (index.column() == 1) {
            return QVariant::fromValue(preference.drawSystem);
        }
    }

    return QVariant();
}

QVariant PreferredDrawSystemsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:
                    return QString(tr("Lower Player Limit"));
                case 1:
                    return QString(tr("Draw System"));
            }
        }
    }
    return QVariant();
}

bool PreferredDrawSystemsModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::EditRole) {
        // if (!QAbstractItemModel::checkIndex(index))
        //     return false;

        if (index.column() == 0) {
            std::size_t limit = value.toInt();
            mStoreManager.dispatch(std::make_unique<ChangeDrawSystemPreferenceLimitAction>(index.row(), limit));
        }
        else if (index.column() == 1) {
            DrawSystemIdentifier identifier = value.value<DrawSystemIdentifier>();
            mStoreManager.dispatch(std::make_unique<ChangeDrawSystemPreferenceIdentifierAction>(index.row(), identifier));
        }

        return true;
    }

    return false;
}

Qt::ItemFlags PreferredDrawSystemsModel::flags(const QModelIndex &index) const {
    auto flags = QAbstractTableModel::flags(index);
    if (!(index.row() == 0 && index.column() == 0))
        flags |= Qt::ItemIsEditable;
    return flags;
}

