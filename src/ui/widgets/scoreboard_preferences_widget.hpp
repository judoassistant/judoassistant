#pragma once

#include <stack>
#include <QMetaObject>
#include <QWidget>

#include "core/core.hpp"
#include "core/stores/preferences_store.hpp"

class StoreManager;
class QComboBox;

class ScoreboardPreferencesWidget : public QWidget {
    Q_OBJECT
public:
    ScoreboardPreferencesWidget(StoreManager &storeManager, QWidget *parent = nullptr);

private:
    QComboBox *mLanguageContent;

    void changeStyle();
    StoreManager &mStoreManager;
};

Q_DECLARE_METATYPE(ScoreboardStylePreference)
