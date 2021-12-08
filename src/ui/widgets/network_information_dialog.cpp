#include <QDialogButtonBox>
#include <QListWidget>
#include <QVBoxLayout>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/host_name.hpp>

#include "ui/widgets/network_information_dialog.hpp"
#include "core/log.hpp"

NetworkInformationDialog::NetworkInformationDialog(QWidget *parent)
    : QDialog(parent)
{
    QListWidget *listWidget = new QListWidget;

    listWidget->addItem(tr("General Preferences"));
    listWidget->setCurrentRow(0);
    // connect(listWidget, &QListWidget::currentRowChanged, stackedWidget, &QStackedWidget::setCurrentIndex);

    // Setup layouts
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(listWidget);

    // Setup button box
    {
        QDialogButtonBox *buttonBox = new QDialogButtonBox;
        buttonBox->addButton(tr("Close"), QDialogButtonBox::AcceptRole);
        mainLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, this, &NetworkInformationDialog::accept);
    }

    setLayout(mainLayout);
    setWindowTitle(tr("Network Information"));
    setGeometry(geometry().x(), geometry().y(), 800, 400);

    listAddresses();
}

void NetworkInformationDialog::listAddresses() {
    boost::asio::io_service io_service;

    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(boost::asio::ip::host_name() + ".local","");
    boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);

    while(it != boost::asio::ip::tcp::resolver::iterator()) {
        boost::asio::ip::address addr = (it++)->endpoint().address();
        log_debug().field("is_ipv6", addr.is_v6()).field("addr", addr.to_string()).msg("Got address");
    }
}

