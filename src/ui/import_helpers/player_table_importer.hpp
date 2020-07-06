#pragma once

#include <optional>

#include "core/core.hpp"
#include "ui/import_helpers/csv_reader.hpp"

class StoreManager;

class PlayerTableImporter {
public:
    static constexpr size_t HEADER_MATCH_ACTION = 2; // Guess that the first row is the header row if at least HEADER_MATCH_ACTION titles match

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
    void setDelimiter(char del);

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

    bool isFirstNameHeader(const QString &cell) const;
    bool isLastNameHeader(const QString &cell) const;
    bool isAgeHeader(const QString &cell) const;
    bool isRankHeader(const QString &cell) const;
    bool isClubHeader(const QString &cell) const;
    bool isWeightHeader(const QString &cell) const;
    bool isCountryHeader(const QString &cell) const;
    bool isSexHeader(const QString &cell) const;

    CSVReader *mReader;
    bool mHasHeaderRow;
    bool mColumnsManuallySet;

    std::optional<size_t> mFirstNameColumn;
    std::optional<size_t> mLastNameColumn;
    std::optional<size_t> mAgeColumn;
    std::optional<size_t> mRankColumn;
    std::optional<size_t> mClubColumn;
    std::optional<size_t> mWeightColumn;
    std::optional<size_t> mCountryColumn;
    std::optional<size_t> mSexColumn;
};

