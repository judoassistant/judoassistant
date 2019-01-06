#include <QStyleFactory>
#include <QCommandLineParser>

#include "applications/hub_application.hpp"
#include "core.hpp"
#include "version.hpp"
#include "widgets/hub_window.hpp"

HubApplication::HubApplication(int &argc, char *argv[]) : QApplication(argc, argv) {
    setApplicationName("JudoAssistant");
    setApplicationVersion(QString::fromStdString(ApplicationVersion::current().toString()));

    QCommandLineParser parser;
    parser.setApplicationDescription("JudoAssistant");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("tournament", "Tournament file to open");

    parser.process(*this);

    mArgs = parser.positionalArguments();
}

int HubApplication::exec() {
    HubWindow window;

    if (mArgs.size() > 0) {
        window.readTournament(mArgs.at(0));
    }

    window.show();
    window.startServer();

    return QApplication::exec();
}

int main(int argc, char *argv[]) {
    HubApplication app(argc, argv);
    return app.exec();
}

