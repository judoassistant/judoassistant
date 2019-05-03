#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFormLayout>

#include "core/actions/category_actions.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "core/stores/category_store.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/auto_add_category_dialog.hpp"

AutoAddCategoryDialog::AutoAddCategoryDialog(StoreManager & storeManager, const std::vector<PlayerId> &playerIds, QWidget *parent)
    : QDialog(parent)
    , mStoreManager(storeManager)
    , mPlayerIds(playerIds)
{
    mBaseNameContent = new QLineEdit;

    mMaxDifferenceContent = new QDoubleSpinBox;
    mMaxDifferenceContent->setRange(0.0, 100.0);
    mMaxDifferenceContent->setSuffix("%");
    mMaxDifferenceContent->setValue(10.0);

    mMaxSizeContent = new QSpinBox;
    mMaxSizeContent->setMinimum(2);
    mMaxSizeContent->setMaximum(mPlayerIds.size());
    // mMaxSizeContent->setSpecialValueText(tr("Unlimited"));
    mMaxSizeContent->setValue(mPlayerIds.size());

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("Base Name"), mBaseNameContent);
    formLayout->addRow(tr("Maximum Weight Difference"), mMaxDifferenceContent);
    formLayout->addRow(tr("Maximum Category Size"), mMaxSizeContent);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    setWindowTitle(tr("Automatically add Categories"));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &AutoAddCategoryDialog::acceptClick);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AutoAddCategoryDialog::cancelClick);
}

void AutoAddCategoryDialog::acceptClick() {
    mStoreManager.dispatch(std::make_unique<AutoAddCategoriesAction>(mStoreManager.getTournament(), mPlayerIds, mBaseNameContent->text().toStdString(), mMaxDifferenceContent->value(), mMaxSizeContent->value()));
    accept();
}

void AutoAddCategoryDialog::cancelClick() {
    reject();
}

