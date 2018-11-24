#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFormLayout>

#include "actions/category_actions.hpp"
#include "draw_systems/draw_systems.hpp"
#include "rulesets/rulesets.hpp"
#include "stores/category_store.hpp"
#include "stores/qtournament_store.hpp"
#include "store_managers/store_manager.hpp"
#include "widgets/create_category_dialog.hpp"

CreateCategoryDialog::CreateCategoryDialog(StoreManager & storeManager, QWidget *parent)
    : QDialog(parent)
    , mStoreManager(storeManager)
{
    mNameContent = new QLineEdit;

    mRulesetContent = new QComboBox;
    for (const auto & ruleset : Rulesets::getRulesets())
        mRulesetContent->addItem(QString::fromStdString(ruleset->getName()));

    mDrawSystemContent = new QComboBox;
    for (const auto & system : DrawSystems::getDrawSystems())
        mDrawSystemContent->addItem(QString::fromStdString(system->getName()));

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("Name"), mNameContent);
    formLayout->addRow(tr("Ruleset"), mRulesetContent);
    formLayout->addRow(tr("Draw System"), mDrawSystemContent);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    setWindowTitle(tr("Create new category"));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &CreateCategoryDialog::acceptClick);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CreateCategoryDialog::cancelClick);
}

void CreateCategoryDialog::acceptClick() {
    mStoreManager.dispatch(std::make_unique<AddCategoryAction>(mStoreManager.getTournament(), mNameContent->text().toStdString(), mRulesetContent->currentIndex(), mDrawSystemContent->currentIndex()));
    accept();
}

void CreateCategoryDialog::cancelClick() {
    reject();
}

