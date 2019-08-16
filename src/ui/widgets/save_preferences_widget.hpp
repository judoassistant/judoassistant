#pragma once

#include <stack>
#include <QMetaObject>
#include <QWidget>

#include "core/core.hpp"

class MasterStoreManager;
class QLineEdit;
class QSpinBox;
class QCheckBox;

class SavePreferencesWidget : public QWidget {
    Q_OBJECT
public:
    SavePreferencesWidget(MasterStoreManager &storeManager, QWidget *parent = nullptr);

private:
    void setAutosave(bool value);
    void setAutosaveFrequency(int value);
    void setBackup(bool value);
    void setBackupAmount(int value);
    MasterStoreManager &mStoreManager;

    QSpinBox *mAutosaveFrequencyContent;
    QSpinBox *mBackupAmountContent;
};

