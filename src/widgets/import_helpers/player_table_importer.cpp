#include <sstream>

#include "actions/player_actions.hpp"
#include "actions/category_actions.hpp"
#include "store_managers/store_manager.hpp"
#include "stores/category_store.hpp"
#include "stores/qtournament_store.hpp"
#include "widgets/import_helpers/csv_reader.hpp"
#include "widgets/import_helpers/player_table_importer.hpp"

PlayerTableImporter::PlayerTableImporter(CSVReader *reader)
    : mReader(reader)
{
    if (!reader->isOpen()) return;

    guessHasHeaderRow();
    guessColumns();
}

bool PlayerTableImporter::hasHeaderRow() {
    return mHasHeaderRow;
}

void PlayerTableImporter::setHasHeaderRow(bool val) {
    mHasHeaderRow = val;
}

void PlayerTableImporter::guessHasHeaderRow() {
    mHasHeaderRow = false;
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
}

void PlayerTableImporter::setFirstNameColumn(std::optional<size_t> val) {
    mFirstNameColumn = val;
}

void PlayerTableImporter::setLastNameColumn(std::optional<size_t> val) {
    mLastNameColumn = val;
}

void PlayerTableImporter::setAgeColumn(std::optional<size_t> val) {
    mAgeColumn = val;
}

void PlayerTableImporter::setRankColumn(std::optional<size_t> val) {
    mRankColumn = val;
}

void PlayerTableImporter::setClubColumn(std::optional<size_t> val) {
    mClubColumn = val;
}

void PlayerTableImporter::setWeightColumn(std::optional<size_t> val) {
    mWeightColumn = val;
}

void PlayerTableImporter::setCountryColumn(std::optional<size_t> val) {
    mCountryColumn = val;
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
}

bool PlayerTableImporter::isValid(size_t row, size_t column) const {
    // String columns are always valid
    std::string val = mReader->get(row, column);

    if (val.empty())
        return true;

    if (mAgeColumn && *mAgeColumn == column) {
        try {
            PlayerAge age(val);
        }
        catch (const std::exception &e) {
            return false;
        }
    }

    if (mRankColumn && *mRankColumn == column) {
        try {
            PlayerRank rank(val);
        }
        catch (const std::exception &e) {
            return false;
        }
    }

    if (mWeightColumn && *mWeightColumn == column) {
        try {
            PlayerWeight weight(val);
        }
        catch (const std::exception &e) {
            return false;
        }
    }

    if (mCountryColumn && *mCountryColumn == column) {
        try {
            PlayerCountry country(val);
        }
        catch (const std::exception &e) {
            return false;
        }
    }

    if (mSexColumn && *mSexColumn == column) {
        try {
            PlayerSex sex(val);
        }
        catch (const std::exception &e) {
            return false;
        }
    }

    return true;
}

std::string PlayerTableImporter::getHeader(size_t column) const {
    std::stringstream ss;
    if (mFirstNameColumn && *mFirstNameColumn == column)
        ss << "First Name, ";

    if (mLastNameColumn && *mLastNameColumn == column)
        ss << "Last Name, ";

    if (mAgeColumn && *mAgeColumn == column)
        ss << "Age, ";

    if (mRankColumn && *mRankColumn == column)
        ss << "Rank, ";

    if (mClubColumn && *mClubColumn == column)
        ss << "Club, ";

    if (mWeightColumn && *mWeightColumn == column)
        ss << "Weight, ";

    if (mCountryColumn && *mCountryColumn == column)
        ss << "Country, ";

    if (mSexColumn && *mSexColumn == column)
        ss << "Sex, ";

    std::string res = ss.str();

    if (res.empty())
        return std::to_string(column+1);
    return res.substr(0, res.size() - 2); // remove the last comma
}

void PlayerTableImporter::import(StoreManager & storeManager) {
    std::vector<PlayerFields> fieldsList;

    size_t offset = (hasHeaderRow() ? 1 : 0);
    for (size_t row = offset; row < mReader->rowCount(); ++row) {
        PlayerFields fields;

        if (mFirstNameColumn)
            fields.firstName = mReader->get(row, *mFirstNameColumn);

        if (mLastNameColumn)
            fields.lastName = mReader->get(row, *mLastNameColumn);

        fields.age = parseValue<PlayerAge>(row, mAgeColumn);
        fields.rank = parseValue<PlayerRank>(row, mRankColumn);

        if (mClubColumn)
            fields.club = mReader->get(row, *mClubColumn);

        fields.weight = parseValue<PlayerWeight>(row, mWeightColumn);
        fields.country = parseValue<PlayerCountry>(row, mCountryColumn);
        fields.sex = parseValue<PlayerSex>(row, mSexColumn);

        fieldsList.push_back(std::move(fields));
    }

    storeManager.dispatch(std::make_unique<AddPlayersAction>(storeManager.getTournament(), std::move(fieldsList)));
}

