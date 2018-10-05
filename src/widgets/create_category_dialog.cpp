#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFormLayout>

#include "widgets/create_category_dialog.hpp"
#include "actions/category_actions.hpp"

CreateCategoryDialog::CreateCategoryDialog(QStoreHandler & storeHandler, QWidget *parent)
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

    connect(buttonBox, &QDialogButtonBox::accepted, this, &CreateCategoryDialog::acceptClick);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CreateCategoryDialog::cancelClick);
}

void CreateCategoryDialog::acceptClick() {
    // mStoreHandler.dispatch(std::make_unique<CreateCategoryAction>(mStoreHandler.getTournament(), mFirstNameContent->text().toStdString(), mLastNameContent->text().toStdString(), mAgeContent->value()));
    accept();
}

void CreateCategoryDialog::cancelClick() {
    reject();
}
