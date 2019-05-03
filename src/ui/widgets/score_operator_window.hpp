#pragma once

#include <stack>

#include <QActionGroup>
#include <QMetaObject>
#include <QPushButton>
#include <QTableView>

#include "core/core.hpp"
#include "core/stores/tatami/location.hpp"
#include "ui/constants/network.hpp"
#include "ui/models/actions_model.hpp"
#include "ui/widgets/client_window.hpp"
#include "ui/widgets/match_card_widget.hpp"
#include "ui/widgets/score_display_widget.hpp"
#include "ui/widgets/score_display_window.hpp"

class ScoreOperatorWindow : public ClientWindow {
    Q_OBJECT

public:
    ScoreOperatorWindow();
    void silentConnect(QString host, int port=Constants::DEFAULT_PORT);

    virtual void show();

private:
    static constexpr auto PAUSING_TIMER_INTERVAL = std::chrono::milliseconds(100);

    void quit();
    void openHomePage();
    void openManual();
    void openReportIssue();
    void showAboutDialog();

    void clearTatamiMenu();
    void populateTatamiMenu();

    void beginResetTournament();
    void endResetTournament();

    void pausingTimerHit();

    void changeTatamis(std::vector<BlockLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks);
    void changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds);
    void beginResetMatches(const std::vector<CategoryId> &categoryIds);
    void endResetMatches(const std::vector<CategoryId> &categoryIds);

    void setTatami(TatamiLocation tatami);

    void findNextMatch();
    void updateNextButton();
    void goNextMatch();
    void disableControlButtons();
    void updateControlButtons();

    void updateUndoButton();
    void undoSelectedAction();

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
    void createViewMenu();
    void createPreferencesMenu();
    void createHelpMenu();

    QWidget* createMainArea();
    QWidget* createSideArea();

    void changeNetworkClientState(NetworkClientState state);

    QMenu *mTatamiMenu;
    QActionGroup *mTatamiActionGroup;
    MatchCardWidget *mNextMatchWidget;
    ScoreDisplayWidget *mScoreDisplayWidget;

    ActionsProxyModel *mActionsModel;
    QTableView *mActionsTable;
    QPushButton *mUndoButton;

    QAction *mConnectAction;
    QAction *mDisconnectAction;

    std::optional<TatamiLocation> mTatami;
    std::optional<std::pair<CategoryId, MatchId>> mCurrentMatch;
    std::optional<std::pair<CategoryId, MatchId>> mNextMatch;
    std::stack<QMetaObject::Connection> mConnections;

    QPushButton *mNextButton;
    QPushButton *mResumeButton;
    QPushButton *mResetButton;

    QPushButton *mWhiteIpponButton;
    QPushButton *mWhiteWazariButton;
    QPushButton *mWhiteShidoButton;
    QPushButton *mWhiteHansokuMakeButton;
    QPushButton *mWhiteOsaekomiButton;

    QPushButton *mBlueIpponButton;
    QPushButton *mBlueWazariButton;
    QPushButton *mBlueShidoButton;
    QPushButton *mBlueHansokuMakeButton;
    QPushButton *mBlueOsaekomiButton;

    QTimer mPausingTimer;

    ScoreDisplayWindow mDisplayWindow;
};

