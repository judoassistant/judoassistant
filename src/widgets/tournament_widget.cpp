#include <QFormLayout>
#include <QGroupBox>
#include <QString>

#include "widgets/tournament_widget.hpp"
#include "actions/change_tournament_name_action.hpp"

TournamentWidget::TournamentWidget(QStoreHandler &storeHandler)
    : mStoreHandler(storeHandler)
{
    // Basic group fields
    QGroupBox *basicGroupBox = new QGroupBox(tr("Basic tournament information"));
    mNameContent = new QLineEdit;
    mNameContent->setText(QString::fromStdString(mStoreHandler.getTournament().getName()));
    connect(mNameContent, &QLineEdit::editingFinished, this, &TournamentWidget::updateTournamentName);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("Tournament name"), mNameContent);
    basicGroupBox->setLayout(formLayout);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(basicGroupBox);

    setLayout(mainLayout);

    connect(&mStoreHandler, &QStoreHandler::tournamentReset, this, &TournamentWidget::tournamentChanged);
    connect(&mStoreHandler.getTournament(), &QTournamentStore::tournamentChanged, this, &TournamentWidget::tournamentChanged);
}

void TournamentWidget::tournamentChanged() {
    mNameContent->setText(QString::fromStdString(mStoreHandler.getTournament().getName()));
}

void TournamentWidget::updateTournamentName() {
    std::string name = mNameContent->text().toStdString();

    if (name == mStoreHandler.getTournament().getName())
        return;

    mStoreHandler.dispatch(std::make_unique<ChangeTournamentNameAction>(mStoreHandler.getTournament(), name));
}
