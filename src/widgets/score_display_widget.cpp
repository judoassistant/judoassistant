#include <QPainter>
#include "widgets/score_display_widget.hpp"

ScoreDisplayWidget::ScoreDisplayWidget(QWidget *parent)
    : QWidget(parent)
    , mFont("Helvetica")
{
    mFont.setBold(true);
    mFont.setCapitalization(QFont::AllUppercase);
}

void ScoreDisplayWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setFont(mFont);

    paintFirstCompetitor(painter);
    paintSecondCompetitor(painter);
    paintLowerSection(painter);
}

void ScoreDisplayWidget::paintFirstCompetitor(QPainter &painter) {
    // Draw Background
    {
        painter.setBrush(Qt::white);
        painter.setPen(Qt::NoPen);
        QRectF coveringRectangle(0.0, 0.0, width(), height()/3);
        painter.drawRect(coveringRectangle);
    }

    int margin = 5;
    int insideHeight = height()/3-margin*2;
    int flagHeight = (insideHeight-margin) * 3/5;
    int flagWidth = (flagHeight * 4)/3;

    // Draw flag
    {
        painter.setBrush(Qt::green);
        QRectF flagRect(margin, margin, flagWidth, flagHeight);
        painter.drawRect(flagRect);

    }

    // Draw country text
    {
        int countrySize = (insideHeight-margin) * 2/5;
        painter.setPen(Qt::black);
        auto font = mFont;
        font.setPixelSize(countrySize);
        painter.setFont(font);
        painter.drawText(margin, flagHeight + margin*2 + countrySize, QString::fromStdString("DEN"));
    }

    // Draw Name
    {
        painter.setPen(Qt::black);
        int nameSize = (insideHeight-margin) / 5;
        auto font = mFont;
        font.setPixelSize(nameSize);
        painter.setFont(font);
        painter.drawText(margin + flagWidth + 20, margin + nameSize, QString::fromStdString("Svendsen S."));
    }

    // Draw Score
    {
        // TODO: Size and position text accurately
        painter.setPen(Qt::black);
        int scoreSize = insideHeight * 4/5;
        auto font = mFont;
        font.setPixelSize(scoreSize);
        painter.setFont(font);

        painter.drawText(width() - scoreSize * 2 + 10, insideHeight-margin, QString::number(1));
        painter.drawText(width() - scoreSize - 10, insideHeight - margin, QString::number(0));
        // painter.drawText(margin + flagWidth + 20, margin + nameSize, QString::fromNumber(1));
    }
}


void ScoreDisplayWidget::paintSecondCompetitor(QPainter &painter) {
    int offset = height()/3;

    // Draw Background
    {
        painter.setBrush(Qt::blue);
        painter.setPen(Qt::NoPen);
        QRectF coveringRectangle(0.0, offset, width(), height()/3);
        painter.drawRect(coveringRectangle);
    }

    int margin = 5;
    int insideHeight = height()/3-margin*2;
    int flagHeight = (insideHeight-margin) * 3/5;
    int flagWidth = (flagHeight * 4)/3;

    // Draw flag
    {
        painter.setBrush(Qt::green);
        QRectF flagRect(margin, offset + height()/3 - flagHeight - margin, flagWidth, flagHeight);
        painter.drawRect(flagRect);
    }

    // Draw country text
    {
        int countrySize = (insideHeight-margin) * 2/5;
        painter.setPen(Qt::white);
        auto font = mFont;
        font.setPixelSize(countrySize);
        painter.setFont(font);
        painter.drawText(margin, offset + margin + countrySize, QString::fromStdString("DEN"));
    }

    // Draw Name
    {
        painter.setPen(Qt::white);
        int nameSize = (insideHeight-margin) / 5;
        auto font = mFont;
        font.setPixelSize(nameSize);
        painter.setFont(font);
        painter.drawText(margin + flagWidth + 20, offset + height()/3 - margin, QString::fromStdString("Svendsen S."));
    }

    // Draw Score
    {
        // TODO: Size and position text accurately
        painter.setPen(Qt::white);
        int scoreSize = insideHeight * 4/5;
        auto font = mFont;
        font.setPixelSize(scoreSize);
        painter.setFont(font);

        // painter.drawText(width() - scoreSize * 2 + 10, offset + margin + scoreSize, QString::number(1));
        painter.drawText(width() - scoreSize - 10, offset + margin + scoreSize, QString::number(0));
        // painter.drawText(margin + flagWidth + 20, margin + nameSize, QString::fromNumber(1));
    }
}

void ScoreDisplayWidget::paintLowerSection(QPainter &painter) {
    int offset = 2 * (height()/3);

    // Draw Background
    {
        painter.setBrush(Qt::black);
        painter.setPen(Qt::NoPen);
        QRectF coveringRectangle(0.0, offset, width(), height()/3);
        painter.drawRect(coveringRectangle);
    }

    int margin = 5;
    int insideHeight = height()/3-margin*2;

    // Draw time
    {
        int timeSize = (insideHeight-margin*2);
        painter.setPen(Qt::green);
        auto font = mFont;
        font.setPixelSize(timeSize);
        painter.setFont(font);
        painter.drawText(margin, height() - margin, QString::fromStdString("2:33"));
    }
}
