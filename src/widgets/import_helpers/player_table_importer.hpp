#pragma once

#include <optional>
#include "widgets/import_helpers/csv_reader.hpp"
#include "store_managers/store_manager.hpp"

class PlayerTableImporter {
public:
    PlayerTableImporter(CSVReader *reader);

    bool hasHeaderRow();
    void setHasHeaderRow(bool val);

    void setFirstNameColumn(std::optional<size_t> val);
    void setLastNameColumn(std::optional<size_t> val);
    void setAgeColumn(std::optional<size_t> val);
    void setRankColumn(std::optional<size_t> val);
    void setClubColumn(std::optional<size_t> val);
    void setWeightColumn(std::optional<size_t> val);
    void setCountryColumn(std::optional<size_t> val);
    void setSexColumn(std::optional<size_t> val);

    std::optional<size_t> getFirstNameColumn() const;
    std::optional<size_t> getLastNameColumn() const;
    std::optional<size_t> getAgeColumn() const;
    std::optional<size_t> getRankColumn() const;
    std::optional<size_t> getClubColumn() const;
    std::optional<size_t> getWeightColumn() const;
    std::optional<size_t> getCountryColumn() const;
    std::optional<size_t> getSexColumn() const;

    bool isValid(size_t row, size_t column) const;
    std::string getHeader(size_t column) const;

    void guessHasHeaderRow();
    void guessColumns();

    void import(StoreManager & storeManager);

private:
    template<typename T>
    std::optional<T> parseValue(size_t row, std::optional<size_t> column) const {
        std::optional<T> res;

        if (column) {
            std::string str = mReader->get(row, *column);
            try {
                T val(str);
                res = val;
            }
            catch (const std::exception &e) {
            }
        }

        return res;
    }

    CSVReader *mReader;
    bool mHasHeaderRow;

    std::optional<size_t> mFirstNameColumn;
    std::optional<size_t> mLastNameColumn;
    std::optional<size_t> mAgeColumn;
    std::optional<size_t> mRankColumn;
    std::optional<size_t> mClubColumn;
    std::optional<size_t> mWeightColumn;
    std::optional<size_t> mCountryColumn;
    std::optional<size_t> mSexColumn;
};

