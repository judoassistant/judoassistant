#pragma once

#include <QFont>

#include "core/stores/match_store.hpp"
#include "core/stores/player_store.hpp"
#include "ui/widgets/scoreboard_painters/scoreboard_painter.hpp"

class NationalScoreboardPainter : public ScoreboardPainter {
public:
    NationalScoreboardPainter();

    void paintEmpty(QPainter &painter, const QRect &rect) override;
    void paintIntroduction(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) override;
    void paintNormal(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) override;
    void paintWinner(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) override;

    void resizeEvent(const QRect &rect) override;

private:
    static constexpr int PADDING = 5;

    void paintIntroductionPlayer(QPainter &painter, const ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex);
    void paintIntroductionLower(QPainter &painter, const ScoreboardPainterParams &params);

    void paintNormalPlayer(QPainter &painter, const ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex);
    void paintNormalLower(QPainter &painter, const ScoreboardPainterParams &params);

    QFont mFont;
    // int mColumnOne;
    // int mColumnTwo;
    // int mColumnThree;
    QRect mWhiteRect;
    QRect mBlueRect;
    QRect mLowerRect;

    // Introduction Mode
    QRect mIntroductionTitleRect;
    QRect mIntroductionCategoryRect;

    QRect mIntroductionWhiteNameRect;
    QRect mIntroductionBlueNameRect;

    QRect mIntroductionWhiteClubRect;
    QRect mIntroductionBlueClubRect;

    int mIntroductionTitleFontSize;
    int mIntroductionCategoryFontSize;
    int mIntroductionClubFontSize;
    int mIntroductionNameFontSize;

    // Normal Mode
    QRect mWhiteScoreRect;
    QRect mBlueScoreRect;
    QRect mOsaekomiRect;

    QRect mNormalTitleRect;
    QRect mNormalCategoryRect;
    QRect mGoldenScoreRect;

    int mNormalTitleFontSize;
    int mNormalCategoryFontSize;
    int mDurationFontSize;
    int mOsaekomiFontSize;
    int mGoldenScoreFontSize;
    int mNormalClubFontSize;
    int mNormalNameFontSize;
    int mScoreFontSize;

    QRect mNormalWhiteNameRect;
    QRect mNormalBlueNameRect;

    QRect mNormalWhiteClubRect;
    QRect mNormalBlueClubRect;
};

