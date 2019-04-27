#include <QStyleFactory>
#include <QCommandLineParser>

#include "ui/applications/kiosk_application.hpp"
#include "core/core.hpp"
#include "core/version.hpp"
#include "ui/widgets/kiosk_window.hpp"

KioskApplication::KioskApplication(int &argc, char *argv[]) : QApplication(argc, argv) {
    setApplicationName("JudoAssistant Kiosk");
    setApplicationVersion(QString::fromStdString(ApplicationVersion::current().toString()));

    QCommandLineParser parser;
    parser.setApplicationDescription("JudoAssistant Kiosk");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("host", "Connect to the server with address <host>");
    parser.addPositionalArgument("port", "Connect to the server on port <port>");
    // parser.addPositionalArgument("tournament", "Tournament file to open");

    parser.process(*this);

    mArgs = parser.positionalArguments();
}

int KioskApplication::exec() {
    KioskWindow kioskWindow;

    if (mArgs.size() > 2) {
        bool ok = true;
        int port = mArgs.at(1).toInt(&ok);

        if (!ok) {
            std::cerr << "Failed converting port argument to int";
            return 1;
        }

        kioskWindow.silentConnect(mArgs.at(0), port);
    }
    else if (mArgs.size() > 1) {
        kioskWindow.silentConnect(mArgs.at(0));
    }

    kioskWindow.show();

    return QApplication::exec();
}

int main(int argc, char *argv[]) {
    KioskApplication app(argc, argv);
    return app.exec();
}

