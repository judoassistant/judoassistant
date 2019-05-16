#pragma once

#include <QDialog>

#include "core/core.hpp"

class StoreManager;
class CSVReader;

class TournamentPreferencesDialog : public QDialog {
    Q_OBJECT
public:
    TournamentPreferencesDialog(StoreManager & storeManager, QWidget *parent = nullptr);

private:
    void closeClick();

    StoreManager &mStoreManager;
};

