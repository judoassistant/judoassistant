#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFormLayout>

#include "widgets/create_category_dialog.hpp"
#include "actions/category_actions.hpp"

#include "rulesets/rulesets.hpp"
#include "draw_strategies/draw_strategies.hpp"

CreateCategoryDialog::CreateCategoryDialog(QStoreHandler & storeHandler, QWidget *parent)
    : QDialog(parent)
    , mStoreHandler(storeHandler)
{
    mNameContent = new QLineEdit;

    mRulesetContent = new QComboBox;
    mRulesetContent->addItem(QString::fromStdString(TwentyEighteenRuleset::getStaticName()));

    mDrawStrategyContent = new QComboBox;
    mDrawStrategyContent->addItem(QString::fromStdString(PoolDrawStrategy::getStaticName()));

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("Name"), mNameContent);
    formLayout->addRow(tr("Ruleset"), mRulesetContent);
    formLayout->addRow(tr("Draw System"), mDrawStrategyContent);

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
    mStoreHandler.dispatch(std::make_unique<CreateCategoryAction>(mStoreHandler.getTournament(), mNameContent->text().toStdString(), std::make_unique<TwentyEighteenRuleset>(), std::make_unique<PoolDrawStrategy>()));
    accept();
}

void CreateCategoryDialog::cancelClick() {
    reject();
}

