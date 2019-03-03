#pragma once

#include <chrono>

#include <QMetaType>
#include <QString>

#include "core/stores/player_store.hpp"
#include "core/stores/match_store.hpp"

struct MatchCardPlayerFields {
    QString firstName;
    QString lastName;
    QString club;
    std::optional<PlayerCountry> country;

    MatchStore::Score score;
};

class MatchCard {
public:
    MatchCard() = default;
    MatchCard(const TournamentStore & tournament, const CategoryStore &category, const MatchStore &match, std::chrono::milliseconds masterTime);

    void paint(QPainter *painter, const QRect &rect, const QPalette &palette) const;
    QSize sizeHint() const;

private:
    void setCategory(const TournamentStore &tournament, const CategoryStore &category, const MatchStore &match);
    void setWhitePlayer(const TournamentStore & tournament, const MatchStore &match);
    void setBluePlayer(const TournamentStore & tournament, const MatchStore &match);
    void setMatch(const CategoryStore &category, const MatchStore &match, std::chrono::milliseconds masterTime);

private:
    static const int WIDTH_HINT = 250;
    static const int HEIGHT_HINT = 120;

    void paintPlayer(MatchCardPlayerFields playerFields, QPainter *painter, QFont &font, int insideWidth, int insideHeight, int columnTwoOffset, int columnThreeOffset, int padding) const;

    std::optional<MatchCardPlayerFields> mWhitePlayer;
    std::optional<MatchCardPlayerFields> mBluePlayer;
    std::optional<size_t> mTatami;
    QString mCategory;
    MatchStatus mStatus;
    bool mIsStopped;
    bool mBye;
    bool mGoldenScore;
    std::chrono::seconds mTime;
};

Q_DECLARE_METATYPE(MatchCard)
