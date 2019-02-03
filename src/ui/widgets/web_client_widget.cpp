#include <QGridLayout>
#include <QLabel>

#include "ui/store_managers/master_store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/configure_dialog.hpp"
#include "ui/widgets/login_dialog.hpp"
#include "ui/widgets/web_client_widget.hpp"

WebClientWidget::WebClientWidget(MasterStoreManager &storeManager, QWidget *parent)
    : QGroupBox(tr("Live Web Results"), parent)
    , mStoreManager(storeManager)
    , mWebStatus(WebClient::Status::NOT_CONNECTED)
{
    addWidgets();

    endResetTournament();
    connect(&mStoreManager, &MasterStoreManager::tournamentAboutToBeReset, this, &WebClientWidget::beginResetTournament);
    connect(&mStoreManager, &MasterStoreManager::tournamentReset, this, &WebClientWidget::endResetTournament);
    connect(&mStoreManager.getWebClient(), &WebClient::statusChanged, this, &WebClientWidget::changeWebStatus);
    connect(&mStoreManager.getWebClient(), &WebClient::loginSucceeded, this, &WebClientWidget::succeedLogin);
}

void WebClientWidget::addWidgets() {
    QGridLayout *layout = new QGridLayout(this);

    auto webNameLabel = new QLabel;
    webNameLabel->setText("Address");
    layout->addWidget(webNameLabel, 0, 0);

    mWebNameContent = new QLineEdit;
    mWebNameContent->setText("live.judoassistant.com/bjergkobing");
    mWebNameContent->setEnabled(false);
    layout->addWidget(mWebNameContent, 0, 1);

    // auto statusLabel = new QLabel;
    // statusLabel->setText("Status");
    // layout->addWidget(statusLabel, 1, 0);

    // auto statusContent = new QLineEdit;
    // statusContent->setText("not configured");
    // statusContent->setEnabled(false);
    // layout->addWidget(statusContent, 1, 1);

    mSetupButton = new QPushButton("Configure");
    connect(mSetupButton, &QPushButton::clicked, this, &WebClientWidget::buttonClick);
    layout->addWidget(mSetupButton, 2, 1);

    // auto mNameButton = new QPushButton("Connect");
    // mNameButton->setEnabled(false);
    // layout->addWidget(mNameButton, 2, 2);
}

void WebClientWidget::changeTournament() {
    const auto &tournament = mStoreManager.getTournament();
    mWebNameContent->setText(QString::fromStdString(tournament.getWebName()));
    updateButton();
}

void WebClientWidget::beginResetTournament() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }
}

void WebClientWidget::endResetTournament() {
    const auto &tournament = mStoreManager.getTournament();

    changeTournament();
    mConnections.push(connect(&tournament, &QTournamentStore::tournamentChanged, this, &WebClientWidget::changeTournament));
}

void WebClientWidget::buttonClick() {
    if (mWebStatus == WebClient::Status::CONNECTING || mWebStatus == WebClient::Status::CONFIGURING || mWebStatus == WebClient::Status::DISCONNECTING) {
        log_warning().msg("Can not press configure button when Web client is already working");
        return;
    }

    const auto webName = mStoreManager.getTournament().getWebName();

    // Login if not already
    if (mWebStatus == WebClient::Status::NOT_CONNECTED) {
        if (!mToken.has_value()) {
            LoginDialog dialog(mStoreManager);
            if (dialog.exec() != QDialog::Accepted)
                return;

            mToken = dialog.getToken();
        }
        else {
            log_debug().msg("Validate token");
        }
    }

    // Register name or disconnect
    if (mWebStatus == WebClient::Status::CONNECTED) {
        if (!webName.empty()) {
            log_debug().msg("Check webName");
        }
        else {
            log_debug().msg("Show configure dialog");
            ConfigureDialog dialog(mStoreManager);
            if (dialog.exec() != QDialog::Accepted)
                return;
        }
    }
    else if (mWebStatus == WebClient::Status::CONFIGURED) {
        log_debug().msg("Disconnect");
    }
}

void WebClientWidget::changeWebStatus(WebClient::Status status) {
    mWebStatus = status;
    updateButton();
}

void WebClientWidget::updateButton() {
    const auto webName = mStoreManager.getTournament().getWebName();

    if (mWebStatus == WebClient::Status::NOT_CONNECTED) {
        mSetupButton->setEnabled(true);

        if (!mToken.has_value())
            mSetupButton->setText("Login");
        else if (webName.empty())
            mSetupButton->setText("Configure");
        else
            mSetupButton->setText("Connect");
        return;
    }

    if (mWebStatus == WebClient::Status::CONNECTING) {
        mSetupButton->setEnabled(false);
        mSetupButton->setText("Connecting...");
        return;
    }

    if (mWebStatus == WebClient::Status::CONNECTED) {
        mSetupButton->setEnabled(true);
        if (webName.empty())
            mSetupButton->setText("Configure");
        else
            mSetupButton->setText("Connect");
        return;
    }

    if (mWebStatus == WebClient::Status::CONFIGURING) {
        mSetupButton->setEnabled(false);
        mSetupButton->setText("Configuring...");
        return;
    }

    if (mWebStatus == WebClient::Status::CONFIGURED) {
        mSetupButton->setEnabled(true);
        mSetupButton->setText("Disconnect");
        return;
    }
}

void WebClientWidget::succeedLogin(const WebToken &token) {
    mToken = token;
}

