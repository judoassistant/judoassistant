#include <QFormLayout>
#include <QGroupBox>
#include <QString>

#include "widgets/tournament_widget.hpp"
#include "actions/tournament_actions.hpp"

TournamentWidget::TournamentWidget(StoreManager &storeManager)
    : mStoreManager(storeManager)
{
    // Basic group fields

    QGroupBox *basicGroupBox = new QGroupBox(tr("Basic tournament information"));
    QFormLayout *formLayout = new QFormLayout;
    basicGroupBox->setLayout(formLayout);

    mNameContent = new QLineEdit;
    mNameContent->setText(QString::fromStdString(mStoreManager.getTournament().getName()));
    connect(mNameContent, &QLineEdit::editingFinished, this, &TournamentWidget::updateTournamentName);
    formLayout->addRow(tr("Tournament name"), mNameContent);

    mLanguageContent = new QComboBox;
    mLanguageContent->addItem("English");
    mLanguageContent->addItem("Danish (Dansk)");
    // connect(mNameContent, &QLineEdit::editingFinished, this, &TournamentWidget::updateTournamentName);
    formLayout->addRow(tr("Tournament language"), mLanguageContent);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(basicGroupBox);

    setLayout(mainLayout);

    tournamentAboutToBeReset();
    tournamentReset();

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &TournamentWidget::tournamentAboutToBeReset);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &TournamentWidget::tournamentReset);
}

void TournamentWidget::tournamentAboutToBeReset() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }
}

void TournamentWidget::tournamentReset() {
    mConnections.push(connect(&mStoreManager.getTournament(), &QTournamentStore::tournamentChanged, this, &TournamentWidget::tournamentChanged));
    tournamentChanged();
}

void TournamentWidget::tournamentChanged() {
    mNameContent->setText(QString::fromStdString(mStoreManager.getTournament().getName()));
}

void TournamentWidget::updateTournamentName() {
    std::string name = mNameContent->text().toStdString();

    if (name == mStoreManager.getTournament().getName())
        return;

    mStoreManager.dispatch(std::make_unique<ChangeTournamentNameAction>(name));
}
