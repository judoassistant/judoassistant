#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "store_managers/master_store_manager.hpp"
#include "widgets/web_client_widget.hpp"

WebClientWidget::WebClientWidget(MasterStoreManager &storeManager, QWidget *parent)
    : QGroupBox(tr("Live Web Results"), parent)
    , mStoreManager(storeManager)
{
    QGridLayout *layout = new QGridLayout(this);

    auto mUrlLabel = new QLabel;
    mUrlLabel->setText("Address");
    layout->addWidget(mUrlLabel, 0, 0);

    auto mUrlContent = new QLineEdit;
    mUrlContent->setText("live.judoassistant.com/bjergkobing");
    mUrlContent->setEnabled(false);
    layout->addWidget(mUrlContent, 0, 1, 1, 2);

    auto mStatusLabel = new QLabel;
    mStatusLabel->setText("Status");
    layout->addWidget(mStatusLabel, 1, 0);

    auto mStatusContent = new QLineEdit;
    mStatusContent->setText("not configured");
    mStatusContent->setEnabled(false);
    layout->addWidget(mStatusContent, 1, 1, 1, 2);

    auto mSetupButton = new QPushButton("Configure");
    layout->addWidget(mSetupButton, 2, 1);

    auto mNameButton = new QPushButton("Connect");
    mNameButton->setEnabled(false);
    layout->addWidget(mNameButton, 2, 2);
    // mNameContent = new QLineEdit;
    // mNameContent->setText(QString::fromStdString(mStoreManager.getTournament().getName()));
    // // connect(mNameContent, &QLineEdit::editingFinished, this, &TournamentWidget::updateTournamentName);
    // formLayout->addRow(tr("Status"), mNameContent);

    // mNameContent = new QLabel;
    // mTatamiCountContent->setMinimum(1);
    // // mTatamiCountContent->setValue(mStoreManager.getTournament().getTatamis().tatamiCount());
    // // connect(mTatamiCountContent, QOverload<int>::of(&QSpinBox::valueChanged), this, &TournamentWidget::updateTatamiCount);
    // formLayout->addRow(tr(""), mTatamiCountContent);

    // mLanguageContent = new QComboBox;
    // mLanguageContent->addItem("English");
    // // connect(mNameContent, &QLineEdit::editingFinished, this, &TournamentWidget::updateTournamentName);
    // formLayout->addRow(tr("Tournament language"), mLanguageContent);
}
