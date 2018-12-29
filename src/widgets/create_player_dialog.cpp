#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFormLayout>

#include "actions/player_actions.hpp"
#include "actions/category_actions.hpp"
#include "stores/category_store.hpp"
#include "stores/qtournament_store.hpp"
#include "store_managers/store_manager.hpp"
#include "widgets/validators/optional_validator.hpp"
#include "widgets/create_player_dialog.hpp"

CreatePlayerDialog::CreatePlayerDialog(StoreManager & storeManager, QWidget *parent)
    : QDialog(parent)
    , mStoreManager(storeManager)
{
    mFirstNameContent = new QLineEdit;
    mLastNameContent = new QLineEdit;

    mAgeContent = new QLineEdit;
    mAgeContent->setValidator(new OptionalValidator(new QIntValidator(PlayerAge::min(), PlayerAge::max()), this));

    mRankContent = new QComboBox;
    mRankContent->addItem("");
    for (PlayerRank rank : PlayerRank::values())
        mRankContent->addItem(QString::fromStdString(rank.toString()));

    mClubContent = new QLineEdit;

    mWeightContent = new QLineEdit;
    mWeightContent->setValidator(new OptionalValidator(new QDoubleValidator(PlayerWeight::min(), PlayerWeight::max(), 2), this));

    mCountryContent = new QComboBox;
    mCountryContent->addItem("");
    for (PlayerCountry country : PlayerCountry::values())
        mCountryContent->addItem(QString::fromStdString(country.toString()));

    mSexContent = new QComboBox;
    mSexContent->addItem("");
    for (PlayerSex country : PlayerSex::values())
        mSexContent->addItem(QString::fromStdString(country.toString()));

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("First name"), mFirstNameContent);
    formLayout->addRow(tr("Last name"), mLastNameContent);
    formLayout->addRow(tr("Sex"), mSexContent);
    formLayout->addRow(tr("Age"), mAgeContent);
    formLayout->addRow(tr("Weight"), mWeightContent);
    formLayout->addRow(tr("Rank"), mRankContent);
    formLayout->addRow(tr("Club"), mClubContent);
    formLayout->addRow(tr("Country"), mCountryContent);

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
        fields.weight = PlayerWeight(mWeightContent->text().toFloat());

    if (mRankContent->currentIndex() > 0) // account for the first index being nullopt
        fields.rank = PlayerRank(mRankContent->currentIndex() - 1);

    fields.club = mClubContent->text().toStdString();

    if (mCountryContent->currentIndex() > 0) // account for the first index being nullopt
        fields.country = PlayerCountry(mCountryContent->currentIndex() - 1);

    if (mSexContent->currentIndex() > 0) // account for the first index being nullopt
        fields.sex = PlayerSex(mSexContent->currentIndex() - 1);

    mStoreManager.dispatch(std::make_unique<AddPlayersAction>(mStoreManager.getTournament(), std::vector<PlayerFields>{std::move(fields)}));
    accept();
}

void CreatePlayerDialog::cancelClick() {
    reject();
}
