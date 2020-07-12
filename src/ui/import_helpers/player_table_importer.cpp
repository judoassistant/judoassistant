#include <sstream>
#include <QTextStream>

#include "core/actions/add_players_action.hpp"
#include "core/stores/category_store.hpp"
#include "ui/import_helpers/csv_reader.hpp"
#include "ui/import_helpers/player_table_importer.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qplayer_age.hpp"
#include "ui/stores/qplayer_country.hpp"
#include "ui/stores/qplayer_rank.hpp"
#include "ui/stores/qplayer_sex.hpp"
#include "ui/stores/qplayer_weight.hpp"
#include "ui/stores/qtournament_store.hpp"

PlayerTableImporter::PlayerTableImporter(CSVReader *reader)
    : mReader(reader)
    , mColumnsManuallySet(false)
{
    if (!reader->isOpen()) return;

    guessHasHeaderRow();
    guessColumns();
}

bool PlayerTableImporter::hasHeaderRow() {
    return mHasHeaderRow;
}

void PlayerTableImporter::setHasHeaderRow(bool val) {
    if (mHasHeaderRow == val)
        return;

    mHasHeaderRow = val;

    if (!mColumnsManuallySet)
        guessColumns();
}

void PlayerTableImporter::guessHasHeaderRow() {
    if (mReader->rowCount() == 0 || mReader->columnCount() == 0) {
        mHasHeaderRow = false;
        return;
    }

    size_t cellMatches = 0;

    for (size_t column = 0; column < mReader->columnCount(); ++column) {
        const QString cell = mReader->get(0, column);

        if (isFirstNameHeader(cell)
            || isLastNameHeader(cell)
            || isSexHeader(cell)
            || isAgeHeader(cell)
            || isWeightHeader(cell)
            || isCountryHeader(cell)
            || isClubHeader(cell)
           ) {
            ++cellMatches;
        }
    }

    mHasHeaderRow = (cellMatches >= HEADER_MATCH_ACTION);
}

void PlayerTableImporter::guessColumns() {
    mFirstNameColumn = std::nullopt;
    mLastNameColumn = std::nullopt;
    mAgeColumn = std::nullopt;
    mRankColumn = std::nullopt;
    mClubColumn = std::nullopt;
    mWeightColumn = std::nullopt;
    mCountryColumn = std::nullopt;
    mSexColumn = std::nullopt;

    if (!mHasHeaderRow)
        return;
    if (mReader->columnCount() == 0)
        return;
    if (mReader->rowCount() == 0)
        return;

    for (size_t column = 0; column < mReader->columnCount(); ++column) {
        const QString cell = mReader->get(0, column);

        if (!mFirstNameColumn.has_value() && isFirstNameHeader(cell)) {
            mFirstNameColumn = column;
            continue;
        }

        if (!mLastNameColumn.has_value() && isLastNameHeader(cell)) {
            mLastNameColumn = column;
            continue;
        }

        if (!mAgeColumn.has_value() && isAgeHeader(cell)) {
            mAgeColumn = column;
            continue;
        }

        if (!mRankColumn.has_value() && isRankHeader(cell)) {
            mRankColumn = column;
            continue;
        }

        if (!mClubColumn.has_value() && isClubHeader(cell)) {
            mClubColumn = column;
            continue;
        }

        if (!mWeightColumn.has_value() && isWeightHeader(cell)) {
            mWeightColumn = column;
            continue;
        }

        if (!mCountryColumn.has_value() && isCountryHeader(cell)) {
            mCountryColumn = column;
            continue;
        }

        if (!mSexColumn.has_value() && isSexHeader(cell)) {
            mSexColumn = column;
            continue;
        }
    }
}

void PlayerTableImporter::setFirstNameColumn(std::optional<size_t> val) {
    mFirstNameColumn = val;
    mColumnsManuallySet = true;
}

void PlayerTableImporter::setLastNameColumn(std::optional<size_t> val) {
    mLastNameColumn = val;
    mColumnsManuallySet = true;
}

void PlayerTableImporter::setAgeColumn(std::optional<size_t> val) {
    mAgeColumn = val;
    mColumnsManuallySet = true;
}

void PlayerTableImporter::setRankColumn(std::optional<size_t> val) {
    mRankColumn = val;
    mColumnsManuallySet = true;
}

void PlayerTableImporter::setClubColumn(std::optional<size_t> val) {
    mClubColumn = val;
    mColumnsManuallySet = true;
}

void PlayerTableImporter::setWeightColumn(std::optional<size_t> val) {
    mWeightColumn = val;
    mColumnsManuallySet = true;
}

void PlayerTableImporter::setCountryColumn(std::optional<size_t> val) {
    mCountryColumn = val;
    mColumnsManuallySet = true;
}

std::optional<size_t> PlayerTableImporter::getFirstNameColumn() const {
    return mFirstNameColumn;
}

std::optional<size_t> PlayerTableImporter::getLastNameColumn() const {
    return mLastNameColumn;
}

std::optional<size_t> PlayerTableImporter::getAgeColumn() const {
    return mAgeColumn;
}

std::optional<size_t> PlayerTableImporter::getRankColumn() const {
    return mRankColumn;
}

std::optional<size_t> PlayerTableImporter::getClubColumn() const {
    return mClubColumn;
}

std::optional<size_t> PlayerTableImporter::getWeightColumn() const {
    return mWeightColumn;
}

std::optional<size_t> PlayerTableImporter::getCountryColumn() const {
    return mCountryColumn;
}

std::optional<size_t> PlayerTableImporter::getSexColumn() const {
    return mSexColumn;
}

void PlayerTableImporter::setSexColumn(std::optional<size_t> val) {
    mSexColumn = val;
    mColumnsManuallySet = true;
}

bool PlayerTableImporter::isValid(size_t row, size_t column) const {
    // String columns are always valid
    const QString val = mReader->get(row, column);

    if (val.isEmpty())
        return true;

    if (mAgeColumn && *mAgeColumn == column) {
        try {
            QPlayerAge::fromHumanString(val);
        }
        catch (const std::exception &e) {
            return false;
        }
    }

    if (mRankColumn && *mRankColumn == column) {
        try {
            QPlayerRank::fromHumanString(val);
        }
        catch (const std::exception &e) {
            return false;
        }
    }

    if (mWeightColumn && *mWeightColumn == column) {
        try {
            QPlayerWeight::fromHumanString(val);
        }
        catch (const std::exception &e) {
            return false;
        }
    }

    if (mCountryColumn && *mCountryColumn == column) {
        try {
            QPlayerCountry::fromHumanString(val);
        }
        catch (const std::exception &e) {
            return false;
        }
    }

    if (mSexColumn && *mSexColumn == column) {
        try {
            QPlayerSex::fromHumanString(val);
        }
        catch (const std::exception &e) {
            return false;
        }
    }

    return true;
}

QString PlayerTableImporter::getHeader(size_t column) const {
    QTextStream stream;
    if (mFirstNameColumn && *mFirstNameColumn == column)
        stream << tr("First Name") << ", ";

    if (mLastNameColumn && *mLastNameColumn == column)
        stream << tr("Last Name") << ", ";

    if (mAgeColumn && *mAgeColumn == column)
        stream << tr("Age") << ", ";

    if (mRankColumn && *mRankColumn == column)
        stream << tr("Rank") << ", ";

    if (mClubColumn && *mClubColumn == column)
        stream << tr("Club") << ", ";

    if (mWeightColumn && *mWeightColumn == column)
        stream << tr("Weight") << ", ";

    if (mCountryColumn && *mCountryColumn == column)
        stream << tr("Country") << ", ";

    if (mSexColumn && *mSexColumn == column)
        stream << tr("Sex") << ", ";

    const QString &res = *(stream.string());

    if (res.isEmpty())
        return QString::number(column+1);
    return res.left(res.size() - 2); // remove the last comma
}

void PlayerTableImporter::import(StoreManager & storeManager) {
    std::vector<PlayerFields> fieldsList;

    size_t offset = hasHeaderRow();
    for (size_t row = offset; row < mReader->rowCount(); ++row) {
        PlayerFields fields;

        if (mFirstNameColumn)
            fields.firstName = mReader->get(row, *mFirstNameColumn).toStdString();

        if (mLastNameColumn)
            fields.lastName = mReader->get(row, *mLastNameColumn).toStdString();

        fields.age = parseValue<PlayerAge, QPlayerAge>(row, mAgeColumn);
        fields.rank = parseValue<PlayerRank, QPlayerRank>(row, mRankColumn);

        if (mClubColumn)
            fields.club = mReader->get(row, *mClubColumn).toStdString();

        fields.weight = parseValue<PlayerWeight, QPlayerWeight>(row, mWeightColumn);
        fields.country = parseValue<PlayerCountry, QPlayerCountry>(row, mCountryColumn);
        fields.sex = parseValue<PlayerSex, QPlayerSex>(row, mSexColumn);

        fieldsList.push_back(std::move(fields));
    }

    storeManager.dispatch(std::make_unique<AddPlayersAction>(storeManager.getTournament(), std::move(fieldsList)));
}

void PlayerTableImporter::setDelimiter(QChar del) {
    mReader->setDelimiter(del);

    if (!mColumnsManuallySet) {
        guessHasHeaderRow();
        guessColumns();
    }
}

bool PlayerTableImporter::isFirstNameHeader(const QString &cell) const {
    const QString lower = cell.toLower();

    if (lower == QString("first name"))
        return true;
    if (lower == QString("firstname"))
        return true;
    return false;
}

bool PlayerTableImporter::isLastNameHeader(const QString &cell) const {
    const QString lower = cell.toLower();

    if (lower == QString("last name"))
        return true;
    if (lower == QString("lastname"))
        return true;
    return false;
}

bool PlayerTableImporter::isAgeHeader(const QString &cell) const {
    const QString lower = cell.toLower();

    if (lower == QString("age"))
        return true;
    return false;
}

bool PlayerTableImporter::isRankHeader(const QString &cell) const {
    const QString lower = cell.toLower();

    if (lower == QString("rank"))
        return true;
    if (lower == QString("belt"))
        return true;
    return false;
}

bool PlayerTableImporter::isClubHeader(const QString &cell) const {
    const QString lower = cell.toLower();

    if (lower == QString("club"))
        return true;
    return false;
}

bool PlayerTableImporter::isWeightHeader(const QString &cell) const {
    const QString lower = cell.toLower();

    if (lower == QString("weight"))
        return true;
    return false;
}

bool PlayerTableImporter::isCountryHeader(const QString &cell) const {
    const QString lower = cell.toLower();

    if (lower == QString("country"))
        return true;
    if (lower == QString("nation"))
        return true;
    return false;
}

bool PlayerTableImporter::isSexHeader(const QString &cell) const {
    const QString lower = cell.toLower();

    if (lower == QString("sex"))
        return true;
    if (lower == QString("gender"))
        return true;
    return false;
}

