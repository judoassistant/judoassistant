#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFormLayout>

#include "actions/player_actions.hpp"
#include "actions/category_actions.hpp"
#include "widgets/validators/optional_validator.hpp"
#include "widgets/edit_player_widget.hpp"
#include "stores/category_store.hpp"
#include "stores/qtournament_store.hpp"
#include "store_managers/store_manager.hpp"

EditPlayerWidget::EditPlayerWidget(StoreManager & storeManager, QWidget *parent)
    : QWidget(parent)
    , mStoreManager(storeManager)
{
    mFirstNameContent = new QLineEdit;
    connect(mFirstNameContent, &QLineEdit::editingFinished, this, &EditPlayerWidget::firstNameEdited);
    mLastNameContent = new QLineEdit;
    connect(mLastNameContent, &QLineEdit::editingFinished, this, &EditPlayerWidget::lastNameEdited);

    mAgeContent = new QLineEdit;
    mAgeContent->setValidator(new OptionalValidator(new QIntValidator(PlayerAge::min(), PlayerAge::max()), this));
    connect(mAgeContent, &QLineEdit::editingFinished, this, &EditPlayerWidget::ageEdited);

    mRankContent = new QComboBox;
    mRankContent->addItem("");
    for (PlayerRank rank : PlayerRank::values())
        mRankContent->addItem(QString::fromStdString(rank.toString()));
    connect(mRankContent, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {rankEdited();});

    mClubContent = new QLineEdit;
    connect(mClubContent, &QLineEdit::editingFinished, this, &EditPlayerWidget::clubEdited);

    mWeightContent = new QLineEdit;
    mWeightContent->setValidator(new OptionalValidator(new QDoubleValidator(PlayerWeight::min(), PlayerWeight::max(), 2), this));
    connect(mWeightContent, &QLineEdit::editingFinished, this, &EditPlayerWidget::weightEdited);

    mCountryContent = new QComboBox;
    mCountryContent->addItem("");
    for (PlayerCountry country : PlayerCountry::values())
        mCountryContent->addItem(QString::fromStdString(country.toString()));
    connect(mCountryContent, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {countryEdited();});

    mSexContent = new QComboBox;
    mSexContent->addItem("");
    for (PlayerSex sex : PlayerSex::values())
        mSexContent->addItem(QString::fromStdString(sex.toString()));
    connect(mSexContent, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {sexEdited();});

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("First name"), mFirstNameContent);
    formLayout->addRow(tr("Last name"), mLastNameContent);
    formLayout->addRow(tr("Sex"), mSexContent);
    formLayout->addRow(tr("Age"), mAgeContent);
    formLayout->addRow(tr("Weight"), mWeightContent);
    formLayout->addRow(tr("Rank"), mRankContent);
    formLayout->addRow(tr("Club"), mClubContent);
    formLayout->addRow(tr("Country"), mCountryContent);

    setPlayer(std::nullopt);
    setLayout(formLayout);

    connect(&(mStoreManager.getTournament()), &QTournamentStore::playersChanged, this, &EditPlayerWidget::playersChanged);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &EditPlayerWidget::tournamentReset);
}

void EditPlayerWidget::tournamentAboutToBeReset() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }

    setPlayer(std::nullopt);
}

void EditPlayerWidget::tournamentReset() {
    mConnections.push(connect(&(mStoreManager.getTournament()), &QTournamentStore::playersChanged, this, &EditPlayerWidget::playersChanged));
}

void EditPlayerWidget::playersChanged(std::vector<PlayerId> ids) {
    if (!mPlayerId || std::find(ids.begin(), ids.end(), *mPlayerId) == ids.end())
        return;

    TournamentStore &tournament = mStoreManager.getTournament();
    PlayerStore &player = tournament.getPlayer(*mPlayerId);

    QString firstName = QString::fromStdString(player.getFirstName());
    if (firstName != mFirstNameContent->text())
        mFirstNameContent->setText(firstName);

    QString lastName = QString::fromStdString(player.getLastName());
    if (lastName != mLastNameContent->text())
        mLastNameContent->setText(lastName);

    QString age = (player.getAge() ? QString::number(player.getAge()->toInt()) : "");
    if (age != mAgeContent->text())
        mAgeContent->setText(age);

    int rankIndex = (player.getRank() ? player.getRank()->toInt() + 1 : 0);
    if (rankIndex != mRankContent->currentIndex())
        mRankContent->setCurrentIndex(rankIndex);

    QString club = QString::fromStdString(player.getClub());
    if (club != mClubContent->text())
        mClubContent->setText(club);

    QString weight = (player.getWeight() ? QString::number(player.getWeight()->toFloat()) : "");
    if (weight != mWeightContent->text())
        mWeightContent->setText(weight);

    int countryIndex = (player.getCountry() ? player.getCountry()->toInt() + 1 : 0);
    if (countryIndex != mCountryContent->currentIndex())
        mCountryContent->setCurrentIndex(countryIndex);

    int sexIndex = (player.getSex() ? player.getSex()->toInt() + 1 : 0);
    if (sexIndex != mSexContent->currentIndex())
        mSexContent->setCurrentIndex(sexIndex);
}

void EditPlayerWidget::setPlayer(std::optional<PlayerId> id) {
    mPlayerId = id;

    if (!id) {
        mFirstNameContent->clear();
        mFirstNameContent->setEnabled(false);
        mLastNameContent->clear();
        mLastNameContent->setEnabled(false);
        mAgeContent->clear();
        mAgeContent->setEnabled(false);
        mRankContent->setCurrentIndex(0);
        mRankContent->setEnabled(false);
        mClubContent->clear();
        mClubContent->setEnabled(false);
        mWeightContent->clear();
        mWeightContent->setEnabled(false);
        mCountryContent->setCurrentIndex(0);
        mCountryContent->setEnabled(false);
        mSexContent->setCurrentIndex(0);
        mSexContent->setEnabled(false);
    }
    else {
        const PlayerStore & player = mStoreManager.getTournament().getPlayer(*id);

        mFirstNameContent->setText(QString::fromStdString(player.getFirstName()));
        mFirstNameContent->setEnabled(true);

        mLastNameContent->setText(QString::fromStdString(player.getLastName()));
        mLastNameContent->setEnabled(true);

        if (player.getAge())
            mAgeContent->setText(QString::number(player.getAge()->toInt()));
        else
            mAgeContent->setText("");

        mAgeContent->setEnabled(true);

        mRankContent->setCurrentIndex(player.getRank() ? player.getRank()->toInt() + 1 : 0);
        mRankContent->setEnabled(true);

        mClubContent->setText(QString::fromStdString(player.getClub()));
        mClubContent->setEnabled(true);

        if (player.getWeight())
            mWeightContent->setText(QString::number(player.getWeight()->toFloat()));
        else
            mWeightContent->setText("");
        mWeightContent->setEnabled(true);

        mCountryContent->setCurrentIndex(player.getCountry() ? player.getCountry()->toInt() + 1 : 0);
        mCountryContent->setEnabled(true);

        mSexContent->setCurrentIndex(player.getSex() ? player.getSex()->toInt() + 1 : 0);
        mSexContent->setEnabled(true);
    }
}

void EditPlayerWidget::firstNameEdited() {
    if (!mPlayerId)
        return;

    TournamentStore &tournament = mStoreManager.getTournament();
    PlayerStore &player = tournament.getPlayer(*mPlayerId);

    std::string newValue = mFirstNameContent->text().toStdString();
    std::string oldValue = player.getFirstName();
    if (newValue == oldValue) return;

    mStoreManager.dispatch(std::make_unique<ChangePlayerFirstNameAction>(*mPlayerId, newValue));
}

void EditPlayerWidget::lastNameEdited() {
    if (!mPlayerId)
        return;

    TournamentStore &tournament = mStoreManager.getTournament();
    PlayerStore &player = tournament.getPlayer(*mPlayerId);

    std::string newValue = mLastNameContent->text().toStdString();
    std::string oldValue = player.getLastName();
    if (newValue == oldValue) return;

    mStoreManager.dispatch(std::make_unique<ChangePlayerLastNameAction>(*mPlayerId, newValue));
}

void EditPlayerWidget::ageEdited() {
    if (!mPlayerId)
        return;

    TournamentStore &tournament = mStoreManager.getTournament();
    PlayerStore &player = tournament.getPlayer(*mPlayerId);

    std::optional<PlayerAge> newValue;
    if (!mAgeContent->text().isEmpty())
        newValue = PlayerAge(mAgeContent->text().toInt());

    std::optional<PlayerAge> oldValue = player.getAge();
    if (newValue == oldValue) return;

    mStoreManager.dispatch(std::make_unique<ChangePlayerAgeAction>(*mPlayerId, newValue));
}

void EditPlayerWidget::rankEdited() {
    if (!mPlayerId)
        return;

    TournamentStore &tournament = mStoreManager.getTournament();
    PlayerStore &player = tournament.getPlayer(*mPlayerId);

    std::optional<PlayerRank> newValue;
    if (mRankContent->currentIndex() > 0)
        newValue = PlayerRank(mRankContent->currentIndex() - 1);

    std::optional<PlayerRank> oldValue = player.getRank();
    if (newValue == oldValue) return;

    mStoreManager.dispatch(std::make_unique<ChangePlayerRankAction>(*mPlayerId, newValue));
}

void EditPlayerWidget::clubEdited() {
    if (!mPlayerId)
        return;

    TournamentStore &tournament = mStoreManager.getTournament();
    PlayerStore &player = tournament.getPlayer(*mPlayerId);

    std::string newValue = mClubContent->text().toStdString();
    std::string oldValue = player.getClub();
    if (newValue == oldValue) return;

    mStoreManager.dispatch(std::make_unique<ChangePlayerClubAction>(*mPlayerId, newValue));
}

void EditPlayerWidget::weightEdited() {
    if (!mPlayerId)
        return;

    TournamentStore &tournament = mStoreManager.getTournament();
    PlayerStore &player = tournament.getPlayer(*mPlayerId);

    std::optional<PlayerWeight> newValue;
    if (!mWeightContent->text().isEmpty())
        newValue = PlayerWeight(mWeightContent->text().toFloat());

    std::optional<PlayerWeight> oldValue = player.getWeight();
    if (newValue == oldValue) return;

    mStoreManager.dispatch(std::make_unique<ChangePlayerWeightAction>(*mPlayerId, newValue));
}

void EditPlayerWidget::countryEdited() {
    if (!mPlayerId)
        return;

    TournamentStore &tournament = mStoreManager.getTournament();
    PlayerStore &player = tournament.getPlayer(*mPlayerId);

    std::optional<PlayerCountry> newValue;
    if (mCountryContent->currentIndex() > 0)
        newValue = PlayerCountry(mCountryContent->currentIndex() - 1);

    std::optional<PlayerCountry> oldValue = player.getCountry();

    if (newValue == oldValue) return;

    mStoreManager.dispatch(std::make_unique<ChangePlayerCountryAction>(*mPlayerId, newValue));
}

void EditPlayerWidget::sexEdited() {
    if (!mPlayerId)
        return;

    TournamentStore &tournament = mStoreManager.getTournament();
    PlayerStore &player = tournament.getPlayer(*mPlayerId);

    std::optional<PlayerSex> newValue;
    if (mSexContent->currentIndex() > 0)
        newValue = PlayerSex(mSexContent->currentIndex() - 1);

    std::optional<PlayerSex> oldValue = player.getSex();

    if (newValue == oldValue) return;

    mStoreManager.dispatch(std::make_unique<ChangePlayerSexAction>(*mPlayerId, newValue));
}

