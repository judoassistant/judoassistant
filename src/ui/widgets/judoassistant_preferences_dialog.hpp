#pragma once

#include <QDialog>

#include "core/core.hpp"

class StoreManager;
class CSVReader;

class JudoassistantPreferencesDialog : public QDialog {
    Q_OBJECT
public:
    JudoassistantPreferencesDialog(StoreManager & storeManager, QWidget *parent = nullptr);

private:
    void closeClick();

    StoreManager &mStoreManager;
};

