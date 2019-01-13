#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include "widgets/tournament_widget.hpp"
#include "actions/tournament_actions.hpp"
#include "actions/tatami_actions.hpp"
#include "stores/qtournament_store.hpp"
#include "store_managers/store_manager.hpp"

constexpr size_t FIRST_COLUMN_WIDTH = 200;

TournamentWidget::TournamentWidget(StoreManager &storeManager)
    : mStoreManager(storeManager)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(basicInformationSection());
    mainLayout->addWidget(webSection());
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

    mLanguageContent = new QComboBox;
    mLanguageContent->addItem("English");
    // connect(mNameContent, &QLineEdit::editingFinished, this, &TournamentWidget::updateTournamentName);
    layout->addWidget(new QLabel(tr("Tournament Language")), 2, 0);
    layout->addWidget(mLanguageContent, 2, 1);

    return box;
}

QWidget* TournamentWidget::webSection() {
    QGroupBox *box = new QGroupBox(tr("Live Web Results"));
    QGridLayout *layout = new QGridLayout(box);

    auto mUrlLabel = new QLabel;
    mUrlLabel->setText("Address");
    layout->addWidget(mUrlLabel, 0, 0);

    auto mUrlContent = new QLineEdit;
    mUrlContent->setText("live.judoassistant.com/bjergkobing");
    mUrlContent->setEnabled(false);
    layout->addWidget(mUrlContent, 0, 1, 1, 2);

    auto mStatusLabel = new QLabel;
    mStatusLabel->setText("Status");
    layout->addWidget(mStatusLabel, 1, 0);

    auto mStatusContent = new QLineEdit;
    mStatusContent->setText("not configured");
    mStatusContent->setEnabled(false);
    layout->addWidget(mStatusContent, 1, 1, 1, 2);

    auto mSetupButton = new QPushButton("Configure");
    layout->addWidget(mSetupButton, 2, 1);

    auto mNameButton = new QPushButton("Connect");
    mNameButton->setEnabled(false);
    layout->addWidget(mNameButton, 2, 2);
    // mNameContent = new QLineEdit;
    // mNameContent->setText(QString::fromStdString(mStoreManager.getTournament().getName()));
    // // connect(mNameContent, &QLineEdit::editingFinished, this, &TournamentWidget::updateTournamentName);
    // formLayout->addRow(tr("Status"), mNameContent);

    // mNameContent = new QLabel;
    // mTatamiCountContent->setMinimum(1);
    // // mTatamiCountContent->setValue(mStoreManager.getTournament().getTatamis().tatamiCount());
    // // connect(mTatamiCountContent, QOverload<int>::of(&QSpinBox::valueChanged), this, &TournamentWidget::updateTatamiCount);
    // formLayout->addRow(tr(""), mTatamiCountContent);

    // mLanguageContent = new QComboBox;
    // mLanguageContent->addItem("English");
    // // connect(mNameContent, &QLineEdit::editingFinished, this, &TournamentWidget::updateTournamentName);
    // formLayout->addRow(tr("Tournament language"), mLanguageContent);

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

