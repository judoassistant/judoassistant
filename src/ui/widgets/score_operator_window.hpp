#pragma once

#include <stack>

#include <QComboBox>
#include <QPushButton>

#include "core/core.hpp"
#include "core/stores/tatami/location.hpp"
#include "ui/constants/network.hpp"
#include "ui/widgets/client_window.hpp"
#include "ui/widgets/match_widget.hpp"
#include "ui/widgets/score_operator_widget.hpp"
#include "ui/widgets/score_display_window.hpp"

class WarningWidget;

class ScoreOperatorWindow : public ClientWindow {
    Q_OBJECT
public:
    ScoreOperatorWindow();
    void silentConnect(QString host, int port=Constants::DEFAULT_PORT);

    virtual void show();

private:
    void quit();
    void openHomePage();
    void openManual();
    void openReportIssue();
    void showAboutDialog();

    void clearTatamiSelect();
    void populateTatamiSelect();

    void beginResetTournament();
    void endResetTournament();

    void changeTatamis(std::vector<BlockLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks);
    void changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds);
    void beginResetMatches(const std::vector<CategoryId> &categoryIds);
    void endResetMatches(const std::vector<CategoryId> &categoryIds);

    void setTatami(int index);

    void findNextMatch();
    void updateNextButton();
    void goNextMatch();
    void disableControlButtons();
    void updateControlButtons();

    void resumeButtonClick();
    void resetButtonClick();
    void awardIppon(MatchStore::PlayerIndex playerIndex);
    void awardWazari(MatchStore::PlayerIndex playerIndex);
    void awardShido(MatchStore::PlayerIndex playerIndex);
    void awardHansokuMake(MatchStore::PlayerIndex playerIndex);
    void osaekomiButtonClick(MatchStore::PlayerIndex playerIndex);

    void createStatusBar();
    void createTournamentMenu();
    void createEditMenu();
    void createPreferencesMenu();
    void createHelpMenu();

    QWidget* createScoreboardSection();
    QWidget* createLowerSection();

    void changeNetworkClientState(NetworkClientState state);

    MatchWidget *mNextMatchWidget;
    ScoreOperatorWidget *mScoreOperatorWidget;

    QAction *mConnectAction;
    QAction *mDisconnectAction;

    std::optional<TatamiLocation> mTatami;
    std::optional<CombinedId> mCurrentMatch;
    std::optional<CombinedId> mNextMatch;
    std::stack<QMetaObject::Connection> mConnections;

    QPushButton *mNextButton;
    QPushButton *mResetButton;
    QComboBox *mTatamiSelect;
    WarningWidget *mWarningWidget;

    ScoreDisplayWindow mDisplayWindow;
};

