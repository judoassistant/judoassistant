#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include "core/actions/set_tatami_count_action.hpp"
#include "core/actions/change_tournament_name_action.hpp"
#include "ui/store_managers/master_store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/tournament_widget.hpp"
#include "ui/widgets/web_client_widget.hpp"

constexpr size_t FIRST_COLUMN_WIDTH = 200;

TournamentWidget::TournamentWidget(MasterStoreManager &storeManager)
    : mStoreManager(storeManager)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(basicInformationSection());
    mainLayout->addWidget(new WebClientWidget(mStoreManager, this));
    mainLayout->setAlignment(Qt::AlignTop);

    setLayout(mainLayout);

    tournamentAboutToBeReset();
    tournamentReset();

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &TournamentWidget::tournamentAboutToBeReset);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &TournamentWidget::tournamentReset);
}

QWidget* TournamentWidget::basicInformationSection() {
    QGroupBox *box = new QGroupBox(tr("Basic tournament information"));
    QGridLayout *layout = new QGridLayout(box);

    mNameContent = new QLineEdit;
    mNameContent->setText(QString::fromStdString(mStoreManager.getTournament().getName()));
    connect(mNameContent, &QLineEdit::editingFinished, this, &TournamentWidget::updateTournamentName);
    layout->addWidget(new QLabel(tr("Tournament name")), 0, 0);
    layout->addWidget(mNameContent, 0, 1);

    mTatamiCountContent = new QSpinBox;
    mTatamiCountContent->setMinimum(1);
    mTatamiCountContent->setValue(mStoreManager.getTournament().getTatamis().tatamiCount());
    connect(mTatamiCountContent, QOverload<int>::of(&QSpinBox::valueChanged), this, &TournamentWidget::updateTatamiCount);
    layout->addWidget(new QLabel(tr("Tatami Count")), 1, 0);
    layout->addWidget(mTatamiCountContent, 1, 1);

    return box;
}

void TournamentWidget::tournamentAboutToBeReset() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }
}

void TournamentWidget::tournamentReset() {
    mConnections.push(connect(&mStoreManager.getTournament(), &QTournamentStore::tournamentChanged, this, &TournamentWidget::tournamentChanged));
    mConnections.push(connect(&mStoreManager.getTournament(), &QTournamentStore::tatamisAdded, this, [this](std::vector<TatamiLocation> id) { tatamiCountChanged(); }));
    mConnections.push(connect(&mStoreManager.getTournament(), &QTournamentStore::tatamisErased, this, [this](std::vector<TatamiLocation> id) { tatamiCountChanged(); }));
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

    mStoreManager.dispatch(std::make_unique<SetTatamiCountAction>(mStoreManager.getTournament(), static_cast<size_t>(count)));
}

