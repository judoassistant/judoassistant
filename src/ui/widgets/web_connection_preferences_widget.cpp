#include "ui/constants/web.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/web_connection_preferences_widget.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

WebConnectionPreferencesWidget::WebConnectionPreferencesWidget(StoreManager &storeManager, QWidget *parent)
    : QWidget(parent)
    , mStoreManager(storeManager)
{
    QFormLayout *layout = new QFormLayout;

    {
        auto *customServerContent = new QCheckBox;
        layout->addRow(new QLabel(tr("Use Custom Server")), customServerContent);
        connect(customServerContent, &QCheckBox::toggled, this, &WebConnectionPreferencesWidget::useCustomServer);
    }
    {
        mHostnameContent = new QLineEdit;
        mHostnameContent->setEnabled(false);
        mHostnameContent->setText(QString::fromStdString(Constants::WEB_HOST));
        layout->addRow(new QLabel(tr("Hostname")), mHostnameContent);
    }
    {
        mPortContent = new QSpinBox;
        mPortContent->setEnabled(false);
        mPortContent->setMinimum(1);
        mPortContent->setMaximum(65535);
        mPortContent->setValue(Constants::WEB_PORT);
        layout->addRow(new QLabel(tr("Port")), mPortContent);
    }
    {
        mSSLContent = new QCheckBox;
        mSSLContent->setEnabled(false);
        mSSLContent->setChecked(true);
        layout->addRow(new QLabel(tr("Use SSL")), mSSLContent);
    }

    setLayout(layout);
    setMinimumWidth(300);
}

void WebConnectionPreferencesWidget::useCustomServer(bool checked) {
    mHostnameContent->setEnabled(checked);
    mPortContent->setEnabled(checked);
    mSSLContent->setEnabled(checked);
}

