#include "core/log.hpp"
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFormLayout>

#include "core/actions/change_players_age_action.hpp"
#include "core/actions/change_players_club_action.hpp"
#include "core/actions/change_players_country_action.hpp"
#include "core/actions/change_players_first_name_action.hpp"
#include "core/actions/change_players_last_name_action.hpp"
#include "core/actions/change_players_rank_action.hpp"
#include "core/actions/change_players_sex_action.hpp"
#include "core/actions/change_players_weight_action.hpp"
#include "core/stores/category_store.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/validators/optional_validator.hpp"
#include "ui/widgets/edit_player_widget.hpp"

int EditPlayerWidget::getSexIndex() {
    assert(!mPlayerIds.empty());
    int res = -1;

    for (auto playerId : mPlayerIds) {
        const auto & sex = mStoreManager.getTournament().getPlayer(playerId).getSex();
        auto index = (sex ? sex->toInt() + 1 : 0);
        if (res == -1)
            res = index;
        else if (res != index) // multiple
            return PlayerSex::SIZE + 1;
    }

    return res;
}

int EditPlayerWidget::getRankIndex() {
    assert(!mPlayerIds.empty());
    int res = -1;

    for (auto playerId : mPlayerIds) {
        const auto & rank = mStoreManager.getTournament().getPlayer(playerId).getRank();
        auto index = (rank ? rank->toInt() + 1 : 0);
        if (res == -1)
            res = index;
        else if (res != index) // multiple
            return PlayerRank::SIZE + 1;
    }

    return res;
}

int EditPlayerWidget::getCountryIndex() {
    assert(!mPlayerIds.empty());
    int res = -1;

    for (auto playerId : mPlayerIds) {
        const auto & country = mStoreManager.getTournament().getPlayer(playerId).getCountry();
        auto index = (country ? country->toInt() + 1 : 0);
        if (res == -1)
            res = index;
        else if (res != index) // multiple
            return PlayerCountry::SIZE + 1;
    }

    return res;
}

EditPlayerWidget::EditPlayerWidget(StoreManager & storeManager, QWidget *parent)
    : QWidget(parent)
    , mStoreManager(storeManager)
{
    mFirstNameContent = new QLineEdit;
    connect(mFirstNameContent, &QLineEdit::editingFinished, this, &EditPlayerWidget::editFirstName);
    mLastNameContent = new QLineEdit;
    connect(mLastNameContent, &QLineEdit::editingFinished, this, &EditPlayerWidget::editLastName);

    mAgeContent = new QLineEdit;
    mAgeContent->setValidator(new OptionalValidator(new QIntValidator(PlayerAge::min(), PlayerAge::max()), this));
    connect(mAgeContent, &QLineEdit::editingFinished, this, &EditPlayerWidget::editAge);

    mRankContent = new QComboBox;
    mRankContent->addItem(EMPTY_TEXT);
    mRankContent->setItemData(0, QBrush(Qt::gray), Qt::ForegroundRole);
    for (PlayerRank rank : PlayerRank::values())
        mRankContent->addItem(QString::fromStdString(rank.toString()));
    connect(mRankContent, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {editRank();});

    mClubContent = new QLineEdit;
    connect(mClubContent, &QLineEdit::editingFinished, this, &EditPlayerWidget::editClub);

    mWeightContent = new QLineEdit;
    mWeightContent->setValidator(new OptionalValidator(new QDoubleValidator(PlayerWeight::min(), PlayerWeight::max(), 2), this));
    connect(mWeightContent, &QLineEdit::editingFinished, this, &EditPlayerWidget::editWeight);

    mCountryContent = new QComboBox;
    mCountryContent->addItem(EMPTY_TEXT);
    mCountryContent->setItemData(0, QBrush(Qt::gray), Qt::ForegroundRole);
    for (PlayerCountry country : PlayerCountry::values())
        mCountryContent->addItem(QString::fromStdString(country.toString()));
    connect(mCountryContent, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {editCountry();});

    mSexContent = new QComboBox;
    mSexContent->addItem(EMPTY_TEXT);
    mSexContent->setItemData(0, QBrush(Qt::gray), Qt::ForegroundRole);
    for (PlayerSex sex : PlayerSex::values())
        mSexContent->addItem(QString::fromStdString(sex.toString()));
    connect(mSexContent, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {editSex();});

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("First name"), mFirstNameContent);
    formLayout->addRow(tr("Last name"), mLastNameContent);
    formLayout->addRow(tr("Sex"), mSexContent);
    formLayout->addRow(tr("Age"), mAgeContent);
    formLayout->addRow(tr("Weight"), mWeightContent);
    formLayout->addRow(tr("Rank"), mRankContent);
    formLayout->addRow(tr("Club"), mClubContent);
    formLayout->addRow(tr("Country"), mCountryContent);

    setPlayers({});
    setLayout(formLayout);

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &EditPlayerWidget::beginResetTournament);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &EditPlayerWidget::endResetTournament);

    endResetTournament();
}

void EditPlayerWidget::beginResetTournament() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }

    setPlayers({});
}

void EditPlayerWidget::endResetTournament() {
    mConnections.push(connect(&(mStoreManager.getTournament()), &QTournamentStore::playersChanged, this, &EditPlayerWidget::changePlayers));
}

void EditPlayerWidget::changePlayers(std::vector<PlayerId> ids) {
    bool intersect = false;
    for (auto playerId : ids) {
        if (mPlayerIds.find(playerId) != mPlayerIds.end()) {
            intersect = true;
            break;
        }
    }

    if (!intersect)
        return;

    updateFirstName();
    updateLastName();
    updateAge();
    updateRank();
    updateClub();
    updateWeight();
    updateCountry();
    updateSex();
}

void EditPlayerWidget::setPlayers(const std::vector<PlayerId> &playerIds) {
    mPlayerIds.clear();
    mPlayerIds.insert(playerIds.begin(), playerIds.end());

    updateFirstName();
    updateLastName();
    updateAge();
    updateRank();
    updateClub();
    updateWeight();
    updateCountry();
    updateSex();
}

void EditPlayerWidget::editFirstName() {
    if (mPlayerIds.size() != 1)
        return;

    const TournamentStore &tournament = mStoreManager.getTournament();
    const PlayerStore &player = tournament.getPlayer(*(mPlayerIds.begin()));

    std::string newValue = mFirstNameContent->text().toStdString();
    std::string oldValue = player.getFirstName();
    if (newValue == oldValue) return;

    mStoreManager.dispatch(std::make_unique<ChangePlayersFirstNameAction>(std::vector<PlayerId>(mPlayerIds.begin(), mPlayerIds.end()), newValue));
}

void EditPlayerWidget::editLastName() {
    if (mPlayerIds.size() != 1)
        return;

    const TournamentStore &tournament = mStoreManager.getTournament();
    const PlayerStore &player = tournament.getPlayer(*(mPlayerIds.begin()));

    std::string newValue = mLastNameContent->text().toStdString();
    std::string oldValue = player.getLastName();
    if (newValue == oldValue) return;

    mStoreManager.dispatch(std::make_unique<ChangePlayersLastNameAction>(std::vector<PlayerId>(mPlayerIds.begin(), mPlayerIds.end()), newValue));
}

void EditPlayerWidget::editAge() {
    if (mPlayerIds.size() != 1)
        return;

    const TournamentStore &tournament = mStoreManager.getTournament();
    const PlayerStore &player = tournament.getPlayer(*(mPlayerIds.begin()));

    std::optional<PlayerAge> newValue;
    if (!mAgeContent->text().isEmpty())
        newValue = PlayerAge(mAgeContent->text().toInt());

    std::optional<PlayerAge> oldValue = player.getAge();
    if (newValue == oldValue) return;

    mStoreManager.dispatch(std::make_unique<ChangePlayersAgeAction>(std::vector<PlayerId>(mPlayerIds.begin(), mPlayerIds.end()), newValue));
}

void EditPlayerWidget::editRank() {
    if (mPlayerIds.empty())
        return;

    auto oldValue = getRankIndex();

    if (mRankContent->currentIndex() == oldValue)
        return;

    std::optional<PlayerRank> newValue;
    if (mRankContent->currentIndex() > 0)
        newValue = PlayerRank(mRankContent->currentIndex() - 1);

    mStoreManager.dispatch(std::make_unique<ChangePlayersRankAction>(std::vector<PlayerId>(mPlayerIds.begin(), mPlayerIds.end()), newValue));
}

void EditPlayerWidget::editClub() {
    if (mPlayerIds.size() != 1)
        return;

    const TournamentStore &tournament = mStoreManager.getTournament();
    const PlayerStore &player = tournament.getPlayer(*(mPlayerIds.begin()));

    std::string newValue = mClubContent->text().toStdString();
    std::string oldValue = player.getClub();
    if (newValue == oldValue) return;

    mStoreManager.dispatch(std::make_unique<ChangePlayersClubAction>(std::vector<PlayerId>(mPlayerIds.begin(), mPlayerIds.end()), newValue));
}

void EditPlayerWidget::editWeight() {
    if (mPlayerIds.size() != 1)
        return;

    const TournamentStore &tournament = mStoreManager.getTournament();
    const PlayerStore &player = tournament.getPlayer(*(mPlayerIds.begin()));

    std::optional<PlayerWeight> newValue;
    if (!mWeightContent->text().isEmpty())
        newValue = PlayerWeight(mWeightContent->text().toFloat());

    std::optional<PlayerWeight> oldValue = player.getWeight();
    if (newValue == oldValue) return;

    mStoreManager.dispatch(std::make_unique<ChangePlayersWeightAction>(std::vector<PlayerId>(mPlayerIds.begin(), mPlayerIds.end()), newValue));
}

void EditPlayerWidget::editCountry() {
    if (mPlayerIds.empty())
        return;

    auto oldValue = getCountryIndex();

    if (mCountryContent->currentIndex() == oldValue)
        return;

    std::optional<PlayerCountry> newValue;
    if (mCountryContent->currentIndex() > 0)
        newValue = PlayerCountry(mCountryContent->currentIndex() - 1);

    mStoreManager.dispatch(std::make_unique<ChangePlayersCountryAction>(std::vector<PlayerId>(mPlayerIds.begin(), mPlayerIds.end()), newValue));
}

void EditPlayerWidget::editSex() {
    if (mPlayerIds.empty())
        return;

    auto oldValue = getSexIndex();

    if (mSexContent->currentIndex() == oldValue)
        return;

    std::optional<PlayerSex> newValue;
    if (mSexContent->currentIndex() > 0)
        newValue = PlayerSex(mSexContent->currentIndex() - 1);

    mStoreManager.dispatch(std::make_unique<ChangePlayersSexAction>(std::vector<PlayerId>(mPlayerIds.begin(), mPlayerIds.end()), newValue));
}

void EditPlayerWidget::updateFirstName() {
    if (mPlayerIds.size() != 1) {
        mFirstNameContent->clear();
        mFirstNameContent->setEnabled(false);
        return;
    }

    const TournamentStore &tournament = mStoreManager.getTournament();
    const PlayerStore &player = tournament.getPlayer(*(mPlayerIds.begin()));
    mFirstNameContent->setText(QString::fromStdString(player.getFirstName()));
    mFirstNameContent->setEnabled(true);
}

void EditPlayerWidget::updateLastName() {
    if (mPlayerIds.size() != 1) {
        mLastNameContent->clear();
        mLastNameContent->setEnabled(false);
        return;
    }

    const TournamentStore &tournament = mStoreManager.getTournament();
    const PlayerStore &player = tournament.getPlayer(*(mPlayerIds.begin()));
    mLastNameContent->setText(QString::fromStdString(player.getLastName()));
    mLastNameContent->setEnabled(true);
}

void EditPlayerWidget::updateAge() {
    if (mPlayerIds.size() != 1) {
        mAgeContent->clear();
        mAgeContent->setEnabled(false);
        return;
    }

    const TournamentStore &tournament = mStoreManager.getTournament();
    const PlayerStore &player = tournament.getPlayer(*(mPlayerIds.begin()));
    auto age = player.getAge();

    mAgeContent->setText(age ? QString::number(age->toInt()) : "");
    mAgeContent->setEnabled(true);
}

void EditPlayerWidget::updateRank() {
    if (mPlayerIds.empty()) {
        mRankContent->setCurrentIndex(0);
        mRankContent->setEnabled(false);
        return;
    }

    int index = getRankIndex();
    if (index == PlayerRank::SIZE + 1) { // multiple
        if (mRankContent->count() == PlayerRank::SIZE + 1) {
            mRankContent->addItem(MULTIPLE_TEXT);
            mRankContent->setItemData(PlayerRank::SIZE + 1, QBrush(Qt::gray), Qt::ForegroundRole);
        }
    }
    else {
        if (mRankContent->count() != PlayerRank::SIZE + 1)
            mRankContent->removeItem(mRankContent->count() - 1);
    }

    mRankContent->setCurrentIndex(index);
    mRankContent->setEnabled(true);
}

void EditPlayerWidget::updateClub() {
    if (mPlayerIds.size() != 1) {
        mClubContent->clear();
        mClubContent->setEnabled(false);
        return;
    }

    const TournamentStore &tournament = mStoreManager.getTournament();
    const PlayerStore &player = tournament.getPlayer(*(mPlayerIds.begin()));

    mClubContent->setText(QString::fromStdString(player.getClub()));
    mClubContent->setEnabled(true);
}

void EditPlayerWidget::updateWeight() {
    if (mPlayerIds.size() != 1) {
        mWeightContent->clear();
        mWeightContent->setEnabled(false);
        return;
    }

    const TournamentStore &tournament = mStoreManager.getTournament();
    const PlayerStore &player = tournament.getPlayer(*(mPlayerIds.begin()));
    auto weight = player.getWeight();

    mWeightContent->setText(weight ? QString::number(weight->toFloat()) : "");
    mWeightContent->setEnabled(true);
}

void EditPlayerWidget::updateCountry() {
    if (mPlayerIds.empty()) {
        mCountryContent->setCurrentIndex(0);
        mCountryContent->setEnabled(false);
        return;
    }

    int index = getCountryIndex();
    if (index == PlayerCountry::SIZE + 1) { // multiple
        if (mCountryContent->count() == PlayerCountry::SIZE + 1) {
            mCountryContent->addItem(MULTIPLE_TEXT);
            mCountryContent->setItemData(PlayerCountry::SIZE + 1, QBrush(Qt::gray), Qt::ForegroundRole);
        }
    }
    else {
        if (mCountryContent->count() != PlayerCountry::SIZE + 1)
            mCountryContent->removeItem(mCountryContent->count() - 1);
    }

    mCountryContent->setCurrentIndex(index);
    mCountryContent->setEnabled(true);
}

void EditPlayerWidget::updateSex() {
    if (mPlayerIds.empty()) {
        mSexContent->setCurrentIndex(0);
        mSexContent->setEnabled(false);
        return;
    }

    int index = getSexIndex();
    if (index == PlayerSex::SIZE + 1) { // multiple
        if (mSexContent->count() == PlayerSex::SIZE + 1) {
            mSexContent->addItem(MULTIPLE_TEXT);
            mSexContent->setItemData(PlayerSex::SIZE + 1, QBrush(Qt::gray), Qt::ForegroundRole);
        }
    }
    else {
        if (mSexContent->count() != PlayerSex::SIZE + 1)
            mSexContent->removeItem(mSexContent->count() - 1);
    }

    mSexContent->setCurrentIndex(index);
    mSexContent->setEnabled(true);
}

