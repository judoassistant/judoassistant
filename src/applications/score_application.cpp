#include <QStyleFactory>
#include <QCommandLineParser>

#include "applications/score_application.hpp"
#include "core.hpp"
#include "version.hpp"
#include "widgets/score_operator_window.hpp"
#include "widgets/score_display_window.hpp"

ScoreApplication::ScoreApplication(int &argc, char *argv[]) : QApplication(argc, argv) {
    setApplicationName("JudoAssistant Score");
    setApplicationVersion(QString::fromStdString(ApplicationVersion::current().toString()));

    QCommandLineParser parser;
    parser.setApplicationDescription("JudoAssistant Score");
    parser.addHelpOption();
    parser.addVersionOption();
    // parser.addPositionalArgument("tournament", "Tournament file to open");

    parser.process(*this);

    mArgs = parser.positionalArguments();
}

int ScoreApplication::exec() {
    ScoreOperatorWindow operatorWindow;
    // ScoreDisplayWindow displayWindow;

    // if (mArgs.size() > 0) {
    //     window.readTournament(mArgs.at(0));
    // }

    operatorWindow.show();
    // displayWindow.show();

    return QApplication::exec();
}

int main(int argc, char *argv[]) {
    ScoreApplication app(argc, argv);
    return app.exec();
}

