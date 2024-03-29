#include "ui/constants/web.hpp"
#include "ui/store_managers/master_store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/save_preferences_widget.hpp"
#include "ui/constants/settings.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QSpinBox>

SavePreferencesWidget::SavePreferencesWidget(MasterStoreManager &storeManager, QWidget *parent)
    : QWidget(parent)
    , mStoreManager(storeManager)
{
    QFormLayout *layout = new QFormLayout;
    const QSettings& settings = storeManager.getSettings();

    bool autosave = settings.value(Constants::Settings::AUTOSAVE_ENABLED, true).toBool();
    {
        auto *autosaveContent = new QCheckBox;
        autosaveContent->setChecked(autosave);
        layout->addRow(new QLabel(tr("Autosave")), autosaveContent);
        connect(autosaveContent, &QCheckBox::toggled, this, &SavePreferencesWidget::setAutosave);
    }
    {
        mAutosaveFrequencyContent = new QSpinBox;
        mAutosaveFrequencyContent->setEnabled(autosave);
        mAutosaveFrequencyContent->setMinimum(1);
        mAutosaveFrequencyContent->setMaximum(60);
        mAutosaveFrequencyContent->setSuffix("min");
        mAutosaveFrequencyContent->setValue(settings.value(Constants::Settings::AUTOSAVE_FREQUENCY, 5).toInt());
        layout->addRow(new QLabel(tr("Autosave Frequency")), mAutosaveFrequencyContent);
        connect(mAutosaveFrequencyContent, QOverload<int>::of(&QSpinBox::valueChanged), this, &SavePreferencesWidget::setAutosaveFrequency);
    }

    bool backup = settings.value(Constants::Settings::BACKUP_ENABLED, false).toBool();
    {
        auto *backupContent = new QCheckBox;
        backupContent->setChecked(backup);
        layout->addRow(new QLabel(tr("Backup Previous Saves")), backupContent);
        connect(backupContent, &QCheckBox::toggled, this, &SavePreferencesWidget::setBackup);
    }
    {
        mBackupAmountContent = new QSpinBox;
        mBackupAmountContent->setEnabled(backup);
        mBackupAmountContent->setMinimum(1);
        mBackupAmountContent->setMaximum(60);
        mBackupAmountContent->setValue(settings.value(Constants::Settings::BACKUP_AMOUNT, 2).toInt());
        layout->addRow(new QLabel(tr("Number of saves to keep")), mBackupAmountContent);
        connect(mBackupAmountContent, QOverload<int>::of(&QSpinBox::valueChanged), this, &SavePreferencesWidget::setBackupAmount);
    }

    setLayout(layout);
    setMinimumWidth(300);
}

void SavePreferencesWidget::setAutosave(bool value) {
    QSettings& settings = mStoreManager.getSettings();
    settings.setValue(Constants::Settings::AUTOSAVE_ENABLED, value);

    mAutosaveFrequencyContent->setEnabled(value);
}

void SavePreferencesWidget::setBackup(bool value) {
    QSettings& settings = mStoreManager.getSettings();
    settings.setValue(Constants::Settings::BACKUP_ENABLED, value);

    mBackupAmountContent->setEnabled(value);
}

void SavePreferencesWidget::setAutosaveFrequency(int value) {
    QSettings& settings = mStoreManager.getSettings();
    settings.setValue(Constants::Settings::AUTOSAVE_FREQUENCY, value);
}

void SavePreferencesWidget::setBackupAmount(int value) {
    QSettings& settings = mStoreManager.getSettings();
    settings.setValue(Constants::Settings::BACKUP_AMOUNT, value);
}

