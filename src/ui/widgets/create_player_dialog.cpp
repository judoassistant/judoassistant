#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFormLayout>

#include "core/actions/add_players_action.hpp"
#include "core/stores/category_store.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/validators/optional_validator.hpp"
#include "ui/widgets/create_player_dialog.hpp"
#include "ui/stores/qplayer_country.hpp"
#include "ui/stores/qplayer_rank.hpp"
#include "ui/stores/qplayer_sex.hpp"
#include "ui/stores/qplayer_weight.hpp"

CreatePlayerDialog::CreatePlayerDialog(StoreManager & storeManager, QWidget *parent)
    : QDialog(parent)
    , mStoreManager(storeManager)
{
    mFirstNameContent = new QLineEdit;
    mFirstNameContent->setMinimumWidth(300);

    mLastNameContent = new QLineEdit;

    mAgeContent = new QLineEdit;
    mAgeContent->setValidator(new OptionalValidator(new QIntValidator(PlayerAge::min(), PlayerAge::max()), this));

    mRankContent = new QComboBox;
    mRankContent->addItem("");
    for (QPlayerRank rank : PlayerRank::values())
        mRankContent->addItem(rank.toHumanString());

    mClubContent = new QLineEdit;

    mWeightContent = new QLineEdit;
    mWeightContent->setValidator(new OptionalValidator(new QDoubleValidator(PlayerWeight::min(), PlayerWeight::max(), 2), this));

    mCountryContent = new QComboBox;
    mCountryContent->addItem("");
    for (QPlayerCountry country : PlayerCountry::values())
        mCountryContent->addItem(country.toHumanString());

    mSexContent = new QComboBox;
    mSexContent->addItem("");
    for (QPlayerSex sex : PlayerSex::values())
        mSexContent->addItem(sex.toHumanString());

    mBlueJudogiHintContent = new QCheckBox;

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("First name"), mFirstNameContent);
    formLayout->addRow(tr("Last name"), mLastNameContent);
    formLayout->addRow(tr("Sex"), mSexContent);
    formLayout->addRow(tr("Age"), mAgeContent);
    formLayout->addRow(tr("Weight"), mWeightContent);
    formLayout->addRow(tr("Rank"), mRankContent);
    formLayout->addRow(tr("Club"), mClubContent);
    formLayout->addRow(tr("Country"), mCountryContent);
    formLayout->addRow(tr("No White Judogi"), mBlueJudogiHintContent);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    setWindowTitle(tr("Create new player"));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &CreatePlayerDialog::acceptClick);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CreatePlayerDialog::cancelClick);
}

void CreatePlayerDialog::acceptClick() {
    PlayerFields fields;
    fields.firstName = mFirstNameContent->text().toStdString();
    fields.lastName = mLastNameContent->text().toStdString();

    if (!mAgeContent->text().isEmpty())
        fields.age = PlayerAge(mAgeContent->text().toInt());

    if (!mWeightContent->text().isEmpty())
        fields.weight = QPlayerWeight::fromHumanString(mWeightContent->text());

    if (mRankContent->currentIndex() > 0) // account for the first index being nullopt
        fields.rank = PlayerRank(mRankContent->currentIndex() - 1);

    fields.club = mClubContent->text().toStdString();

    if (mCountryContent->currentIndex() > 0) // account for the first index being nullopt
        fields.country = PlayerCountry(mCountryContent->currentIndex() - 1);

    if (mSexContent->currentIndex() > 0) // account for the first index being nullopt
        fields.sex = PlayerSex(mSexContent->currentIndex() - 1);

    fields.blueJudogiHint = mBlueJudogiHintContent->checkState() == Qt::Checked;

    mStoreManager.dispatch(std::make_unique<AddPlayersAction>(mStoreManager.getTournament(), std::vector<PlayerFields>{std::move(fields)}));
    accept();
}

void CreatePlayerDialog::cancelClick() {
    reject();
}
