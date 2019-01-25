#include <QStyleFactory>
#include <QCommandLineParser>

#include "ui/applications/score_application.hpp"
#include "core/core.hpp"
#include "core/version.hpp"
#include "ui/widgets/score_operator_window.hpp"
#include "ui/widgets/score_display_window.hpp"

ScoreApplication::ScoreApplication(int &argc, char *argv[]) : QApplication(argc, argv) {
    setApplicationName("JudoAssistant Score");
    setApplicationVersion(QString::fromStdString(ApplicationVersion::current().toString()));

    QCommandLineParser parser;
    parser.setApplicationDescription("JudoAssistant Score");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("host", "Connect to the server with address <host>");
    parser.addPositionalArgument("port", "Connect to the server on port <port>");
    // parser.addPositionalArgument("tournament", "Tournament file to open");

    parser.process(*this);

    mArgs = parser.positionalArguments();
}

int ScoreApplication::exec() {
    ScoreOperatorWindow operatorWindow;
    // ScoreDisplayWindow displayWindow;

    if (mArgs.size() > 2) {
        bool ok = true;
        int port = mArgs.at(1).toInt(&ok);

        if (!ok) {
            std::cerr << "Failed converting port argument to int";
            return 1;
        }

        operatorWindow.silentConnect(mArgs.at(0), port);
    }
    else if (mArgs.size() > 1) {
        operatorWindow.silentConnect(mArgs.at(0));
    }

    operatorWindow.show();
    // displayWindow.show();

    return QApplication::exec();
}

int main(int argc, char *argv[]) {
    ScoreApplication app(argc, argv);
    return app.exec();
}

