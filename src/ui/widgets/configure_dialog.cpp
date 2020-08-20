#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "core/log.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/store_managers/master_store_manager.hpp"
#include "ui/widgets/configure_dialog.hpp"

ConfigureDialog::ConfigureDialog(MasterStoreManager &storeManager, QWidget *parent)
    : QDialog(parent)
    , mStoreManager(storeManager)
{
    setWindowTitle(tr("Configure Web Name"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    {
        QLabel *helpLabel = new QLabel(tr("Select the web name for your tournament. This will be displayed in the address bar on live.judoassistant.com."), this);
        helpLabel->setWordWrap(true);
        mainLayout->addWidget(helpLabel);
    }

    {
        QWidget *group = new QWidget(this);
        QFormLayout *formLayout = new QFormLayout(group);

        mWebNameContent = new QLineEdit;
        // mWebNameContent->setText("bjergkobing");

        formLayout->addRow(tr("Web Name"), mWebNameContent);

        mainLayout->addWidget(group);
    }

    {
        QDialogButtonBox *buttonBox = new QDialogButtonBox;
        mRegisterButton = new QPushButton(tr("Register"));
        mRegisterButton->setEnabled(true);
        buttonBox->addButton(mRegisterButton, QDialogButtonBox::AcceptRole);
        buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

        connect(buttonBox, &QDialogButtonBox::accepted, this, &ConfigureDialog::registerClick);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &ConfigureDialog::cancelClick);

        mainLayout->addWidget(buttonBox);
    }

    auto &webClient = mStoreManager.getWebClient();
    connect(&webClient, &WebClient::registrationSucceeded, this, &ConfigureDialog::succeedRegistration);
    connect(&webClient, &WebClient::registrationFailed, this, &ConfigureDialog::failRegistration);
    connect(&webClient, &WebClient::webNameChecked, this, &ConfigureDialog::showCheckResult);
}

void ConfigureDialog::registerClick() {
    mRegisterButton->setText(tr("Registrering.."));
    mRegisterButton->setEnabled(false);

    auto &webClient = mStoreManager.getWebClient();
    auto webName = mWebNameContent->text();
    webClient.registerWebName(mStoreManager.getTournament().getId(), webName);
}

void ConfigureDialog::cancelClick() {
    reject();
}

void ConfigureDialog::succeedRegistration(const QString &webName) {
    accept();
}

void ConfigureDialog::failRegistration(WebNameRegistrationResponse response) {
    mRegisterButton->setText(tr("Register"));
    mRegisterButton->setEnabled(true);

    if (response == WebNameRegistrationResponse::SERVER_ERROR)
        QMessageBox::warning(this, tr("Registration failed"), tr("The connection to live web failed. Try again later."), QMessageBox::Ok, QMessageBox::Ok);
    else if (response == WebNameRegistrationResponse::OCCUPIED_OTHER_USER)
        QMessageBox::warning(this, tr("Registration failed"), tr("The chosen web name is already in use by another used "), QMessageBox::Ok, QMessageBox::Ok);
}

void ConfigureDialog::checkWebName() {

}

void ConfigureDialog::showCheckResult(const QString &webName, WebNameCheckResponse status) {

}

