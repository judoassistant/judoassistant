#pragma once

#include <stack>
#include <QMetaObject>
#include <QWidget>

#include "core/core.hpp"
#include "core/stores/preferences_store.hpp"

class StoreManager;
class QComboBox;

class DisplayPreferencesWidget : public QWidget {
    Q_OBJECT
public:
    DisplayPreferencesWidget(StoreManager &storeManager, QWidget *parent = nullptr);

private:
    QComboBox *mScoreboardContent;
    QComboBox *mMatchCardContent;

    void changeScoreboardStyle();
    void changeMatchCardStyle();
    StoreManager &mStoreManager;
};

Q_DECLARE_METATYPE(ScoreboardStylePreference)
Q_DECLARE_METATYPE(MatchCardStylePreference)
