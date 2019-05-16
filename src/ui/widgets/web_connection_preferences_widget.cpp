#include "ui/constants/web.hpp"
#include "ui/store_managers/master_store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/web_connection_preferences_widget.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QSpinBox>

WebConnectionPreferencesWidget::WebConnectionPreferencesWidget(MasterStoreManager &storeManager, QWidget *parent)
    : QWidget(parent)
    , mStoreManager(storeManager)
{
    QFormLayout *layout = new QFormLayout;
    const QSettings& settings = storeManager.getSettings();

    bool useCustomServer = settings.value("web/customServer", false).toBool();
    {
        auto *customServerContent = new QCheckBox;
        customServerContent->setChecked(useCustomServer);
        layout->addRow(new QLabel(tr("Use Custom Server")), customServerContent);
        connect(customServerContent, &QCheckBox::toggled, this, &WebConnectionPreferencesWidget::setCustomServer);
    }
    {
        mHostnameContent = new QLineEdit;
        mHostnameContent->setEnabled(useCustomServer);
        mHostnameContent->setText(settings.value("web/hostname", Constants::WEB_HOST).toString());
        layout->addRow(new QLabel(tr("Hostname")), mHostnameContent);
        connect(mHostnameContent, &QLineEdit::textEdited, this, &WebConnectionPreferencesWidget::setHostname);
    }
    {
        mPortContent = new QSpinBox;
        mPortContent->setEnabled(useCustomServer);
        mPortContent->setMinimum(1);
        mPortContent->setMaximum(65535);
        mPortContent->setValue(settings.value("web/port", Constants::WEB_PORT).toInt());
        layout->addRow(new QLabel(tr("Port")), mPortContent);
        connect(mPortContent, QOverload<int>::of(&QSpinBox::valueChanged), this, &WebConnectionPreferencesWidget::setPort);
    }
    {
        mSSLContent = new QCheckBox;
        mSSLContent->setEnabled(useCustomServer);
        mSSLContent->setChecked(settings.value("web/ssl", true).toBool());
        layout->addRow(new QLabel(tr("Use SSL")), mSSLContent);
        connect(mSSLContent, &QCheckBox::toggled, this, &WebConnectionPreferencesWidget::setSSL);
    }

    setLayout(layout);
    setMinimumWidth(300);
}

void WebConnectionPreferencesWidget::setCustomServer(bool value) {
    QSettings& settings = mStoreManager.getSettings();
    settings.setValue("web/customServer", value);

    mHostnameContent->setEnabled(value);
    mPortContent->setEnabled(value);
    mSSLContent->setEnabled(value);
}

void WebConnectionPreferencesWidget::setHostname(const QString &value) {
    QSettings& settings = mStoreManager.getSettings();
    settings.setValue("web/hostname", value);
}

void WebConnectionPreferencesWidget::setPort(int value) {
    QSettings& settings = mStoreManager.getSettings();
    settings.setValue("web/port", value);
}

void WebConnectionPreferencesWidget::setSSL(bool value) {
    QSettings& settings = mStoreManager.getSettings();
    settings.setValue("web/ssl", value);
}

