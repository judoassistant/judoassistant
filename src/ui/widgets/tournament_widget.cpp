#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include "core/actions/change_tournament_date_action.hpp"
#include "core/actions/change_tournament_location_action.hpp"
#include "core/actions/change_tournament_name_action.hpp"
#include "core/actions/set_tatami_count_action.hpp"
#include "ui/store_managers/master_store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/tournament_widget.hpp"
#include "ui/widgets/web_client_widget.hpp"

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

    mLocationContent = new QLineEdit;
    mLocationContent->setText(QString::fromStdString(mStoreManager.getTournament().getLocation()));
    connect(mLocationContent, &QLineEdit::editingFinished, this, &TournamentWidget::updateTournamentLocation);
    layout->addWidget(new QLabel(tr("Tournament location")), 1, 0);
    layout->addWidget(mLocationContent, 1, 1);

    // TODO: connect to backend
    mDateContent = new QDateEdit;
    mDateContent->setDisplayFormat("dd/MM/yyyy");
    mDateContent->setDate(mStoreManager.getTournament().getQDate());
    mDateContent->setMinimumDate(QDate::currentDate());
    connect(mDateContent, &QDateEdit::dateChanged, this, &TournamentWidget::updateTournamentDate);
    layout->addWidget(new QLabel(tr("Tournament date")), 2, 0);
    layout->addWidget(mDateContent, 2, 1);

    mTatamiCountContent = new QSpinBox;
    mTatamiCountContent->setMinimum(1);
    mTatamiCountContent->setValue(mStoreManager.getTournament().getTatamis().tatamiCount());
    connect(mTatamiCountContent, QOverload<int>::of(&QSpinBox::valueChanged), this, &TournamentWidget::updateTatamiCount);
    layout->addWidget(new QLabel(tr("Tatami Count")), 3, 0);
    layout->addWidget(mTatamiCountContent, 3, 1);

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
    mLocationContent->setText(QString::fromStdString(mStoreManager.getTournament().getLocation()));
    mDateContent->setDate(mStoreManager.getTournament().getQDate());
}

void TournamentWidget::tatamiCountChanged() {
    mTatamiCountContent->setValue(mStoreManager.getTournament().getTatamis().tatamiCount());
}

void TournamentWidget::updateTournamentLocation() {
    std::string location = mLocationContent->text().toStdString();

    if (location == mStoreManager.getTournament().getLocation())
        return;

    mStoreManager.dispatch(std::make_unique<ChangeTournamentLocationAction>(location));
}

void TournamentWidget::updateTournamentName() {
    std::string name = mNameContent->text().toStdString();

    if (name == mStoreManager.getTournament().getName())
        return;

    mStoreManager.dispatch(std::make_unique<ChangeTournamentNameAction>(name));
}

void TournamentWidget::updateTournamentDate(const QDate &date) {
    if (date == mStoreManager.getTournament().getQDate())
        return;

    QString dateString = date.toString("yyyy-MM-dd");
    mStoreManager.dispatch(std::make_unique<ChangeTournamentDateAction>(dateString.toStdString()));
}

void TournamentWidget::updateTatamiCount(int count) {
    if (static_cast<size_t>(count) == mStoreManager.getTournament().getTatamis().tatamiCount())
        return;

    mStoreManager.dispatch(std::make_unique<SetTatamiCountAction>(mStoreManager.getTournament(), static_cast<size_t>(count)));
}

