#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFormLayout>

#include "widgets/create_player_dialog.hpp"
#include "actions/create_player_action.hpp"

CreatePlayerDialog::CreatePlayerDialog(QStoreHandler & storeHandler, QWidget *parent)
    : QDialog(parent)
    , mStoreHandler(storeHandler)
{
    mFirstNameContent = new QLineEdit;
    mLastNameContent = new QLineEdit;
    mAgeContent = new QSpinBox;

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("First name"), mFirstNameContent);
    formLayout->addRow(tr("Last name"), mLastNameContent);
    formLayout->addRow(tr("Age"), mAgeContent);

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
    mStoreHandler.dispatch(std::make_unique<CreatePlayerAction>(mStoreHandler.getTournament(), mFirstNameContent->text().toStdString(), mLastNameContent->text().toStdString(), mAgeContent->value()));
    accept();
}

void CreatePlayerDialog::cancelClick() {
    reject();
}
