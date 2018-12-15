#pragma once

#include <QMetaType>
#include <QString>
#include <QStyledItemDelegate>

#include "stores/player_store.hpp"

struct MatchCardPlayerFields {
    QString firstName;
    QString lastName;
    QString club;
    std::optional<PlayerCountry> country;
    int shidos;
    int ippons;
    int wazaris;
    bool hansokuMake;
};

class MatchCard {
public:
    MatchCard() = default;
    MatchCard(std::optional<MatchCardPlayerFields> whitePlayer, std::optional<MatchCardPlayerFields> bluePlayer);

    void paint(QPainter *painter, const QRect &rect, const QPalette &palette) const;
    QSize sizeHint();

private:
    static const int WIDTH_HINT = 250;
    static const int HEIGHT_HINT = 120;

    std::optional<MatchCardPlayerFields> mWhitePlayer;
    std::optional<MatchCardPlayerFields> mBluePlayer;
    size_t mTatami;
    bool mStarted;
    bool mPaused;
    bool mGoldenScore;
    int time;
};

Q_DECLARE_METATYPE(MatchCard)

class MatchCardDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    MatchCardDelegate(QWidget *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
};

