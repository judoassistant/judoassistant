#include <QFormLayout>
#include <QGroupBox>
#include <QString>

#include "widgets/tournament_widget.hpp"
#include "actions/tournament_actions.hpp"
#include "actions/tatami_actions.hpp"
#include "stores/qtournament_store.hpp"
#include "store_managers/store_manager.hpp"

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

    mTatamiCountContent = new QSpinBox;
    mTatamiCountContent->setMinimum(1);
    mTatamiCountContent->setValue(mStoreManager.getTournament().getTatamis().tatamiCount());
    connect(mTatamiCountContent, QOverload<int>::of(&QSpinBox::valueChanged), this, &TournamentWidget::updateTatamiCount);
    formLayout->addRow(tr("Tatami Count"), mTatamiCountContent);

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
    mConnections.push(connect(&mStoreManager.getTournament(), &QTournamentStore::tatamisAdded, this, [this](std::vector<size_t> id) { tatamiCountChanged(); }));
    mConnections.push(connect(&mStoreManager.getTournament(), &QTournamentStore::tatamisErased, this, [this](std::vector<size_t> id) { tatamiCountChanged(); }));
    tournamentChanged();
    tatamiCountChanged();
}

void TournamentWidget::tournamentChanged() {
    mNameContent->setText(QString::fromStdString(mStoreManager.getTournament().getName()));
}

void TournamentWidget::tatamiCountChanged() {
    mTatamiCountContent->setValue(mStoreManager.getTournament().getTatamis().tatamiCount());
}

void TournamentWidget::updateTournamentName() {
    std::string name = mNameContent->text().toStdString();

    if (name == mStoreManager.getTournament().getName())
        return;

    mStoreManager.dispatch(std::make_unique<ChangeTournamentNameAction>(name));
}

void TournamentWidget::updateTatamiCount(int count) {
    if (static_cast<size_t>(count) == mStoreManager.getTournament().getTatamis().tatamiCount())
        return;

    mStoreManager.dispatch(std::make_unique<SetTatamiCountAction>(static_cast<size_t>(count)));
}

