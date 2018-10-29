#pragma once

#include <QDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QTableWidget>

#include "core.hpp"
#include "store_handlers/qstore_handler.hpp"
#include "widgets/import_helpers/csv_reader.hpp"
#include "widgets/import_helpers/player_table_importer.hpp"

class QStoreHandler;

class ImportPlayersCSVDialog : public QDialog {
    Q_OBJECT
public:
    ImportPlayersCSVDialog(QStoreHandler & storeHandler, CSVReader *reader, QWidget *parent = nullptr);

protected slots:
    void acceptClick();
    void cancelClick();

    void setHasHeaderRow(int state);
    void setDelimiter(const QString &text);
    void setFirstNameColumn(int index);
    void setLastNameColumn(int index);
    void setAgeColumn(int index);
    void setRankColumn(int index);
    void setClubColumn(int index);
    void setWeightColumn(int index);
    void setCountryColumn(int index);
    void setSexColumn(int index);
    void refillColumnBoxes();

private:
    void refillColumnBox(QComboBox* box, std::optional<size_t> currentColumn);
    void resetPreview();
    void updatePreviewColumn(size_t column);

    QStoreHandler & mStoreHandler;
    QComboBox *mDelimiterContent;
    QCheckBox *mHeaderContent;
    CSVReader *mReader;
    PlayerTableImporter mImporter;

    QComboBox *mFirstNameContent;
    QComboBox *mLastNameContent;
    QComboBox *mAgeContent;
    QComboBox *mRankContent;
    QComboBox *mClubContent;
    QComboBox *mWeightContent;
    QComboBox *mCountryContent;
    QComboBox *mSexContent;
    QTableWidget *mPreviewWidget;
    std::vector<bool> mIsColumnValid;
};

