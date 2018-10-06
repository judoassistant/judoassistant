#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFormLayout>

#include "widgets/edit_player_widget.hpp"
#include "actions/player_actions.hpp"

EditPlayerWidget::EditPlayerWidget(QStoreHandler & storeHandler, QWidget *parent)
    : QWidget(parent)
    , mStoreHandler(storeHandler)
{
    mFirstNameContent = new QLineEdit;
    mLastNameContent = new QLineEdit;
    mAgeContent = new QSpinBox;

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("First name"), mFirstNameContent);
    formLayout->addRow(tr("Last name"), mLastNameContent);
    formLayout->addRow(tr("Age"), mAgeContent);

    setLayout(formLayout);
}

void EditPlayerWidget::setPlayer(Id id) {
}
