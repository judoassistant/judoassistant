#pragma once

#include <QDialog>

#include "core/core.hpp"

class MasterStoreManager;
class CSVReader;

class JudoassistantPreferencesDialog : public QDialog {
    Q_OBJECT
public:
    JudoassistantPreferencesDialog(MasterStoreManager & storeManager, QWidget *parent = nullptr);

private:
    void closeClick();

    MasterStoreManager &mStoreManager;
};

